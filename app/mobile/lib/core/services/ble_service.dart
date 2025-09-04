import 'dart:async';
import 'dart:typed_data';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:logging/logging.dart';
import '../models/device_models.dart';
import '../models/health_models.dart';

class BleService {
  static final _logger = Logger('BleService');
  static final BleService _instance = BleService._internal();
  factory BleService() => _instance;
  BleService._internal();

  // BLE Service UUIDs
  static const String HEART_RATE_SERVICE = "0000180D-0000-1000-8000-00805F9B34FB";
  static const String BATTERY_SERVICE = "0000180F-0000-1000-8000-00805F9B34FB";
  static const String VITALS_SERVICE = "12345678-1234-1234-1234-123456789ABC";
  static const String HEALTH_MONITOR_SERVICE = "12345678-1234-1234-1234-123456789ABD";
  static const String DATA_SYNC_SERVICE = "12345678-1234-1234-1234-123456789ABE";

  // Characteristic UUIDs
  static const String HEART_RATE_MEASUREMENT = "00002A37-0000-1000-8000-00805F9B34FB";
  static const String BATTERY_LEVEL = "00002A19-0000-1000-8000-00805F9B34FB";
  static const String HRV_CHARACTERISTIC = "12345678-1234-1234-1234-123456789AC1";
  static const String TEMPERATURE_CHARACTERISTIC = "12345678-1234-1234-1234-123456789AC2";
  static const String RESPIRATORY_CHARACTERISTIC = "12345678-1234-1234-1234-123456789AC3";
  static const String HEALTH_FLAGS_CHARACTERISTIC = "12345678-1234-1234-1234-123456789AD1";
  static const String DATA_DOWNLOAD_CHARACTERISTIC = "12345678-1234-1234-1234-123456789AE1";

  BluetoothDevice? _connectedDevice;
  final Map<String, BluetoothCharacteristic> _characteristics = {};
  
  // Stream controllers
  final _connectionStateController = StreamController<DeviceConnectionState>.broadcast();
  final _heartRateController = StreamController<HeartRateData>.broadcast();
  final _vitalsController = StreamController<VitalsData>.broadcast();
  final _healthAlertsController = StreamController<HealthAlert>.broadcast();
  final _batteryController = StreamController<int>.broadcast();

  // Getters for streams
  Stream<DeviceConnectionState> get connectionState => _connectionStateController.stream;
  Stream<HeartRateData> get heartRateStream => _heartRateController.stream;
  Stream<VitalsData> get vitalsStream => _vitalsController.stream;
  Stream<HealthAlert> get healthAlertsStream => _healthAlertsController.stream;
  Stream<int> get batteryStream => _batteryController.stream;

  bool get isConnected => _connectedDevice != null;
  BluetoothDevice? get connectedDevice => _connectedDevice;

  Future<void> startScan({Duration timeout = const Duration(seconds: 10)}) async {
    _logger.info('Starting BLE scan...');
    
    try {
      // Check if Bluetooth is available
      if (await FlutterBluePlus.isAvailable == false) {
        throw Exception('Bluetooth not available');
      }

      // Check if Bluetooth is turned on
      if (await FlutterBluePlus.adapterState.first != BluetoothAdapterState.on) {
        throw Exception('Bluetooth is not turned on');
      }

      await FlutterBluePlus.startScan(
        timeout: timeout,
        withServices: [Guid(HEART_RATE_SERVICE)],
      );
    } catch (e) {
      _logger.severe('Error starting scan: $e');
      rethrow;
    }
  }

  Future<void> stopScan() async {
    await FlutterBluePlus.stopScan();
    _logger.info('BLE scan stopped');
  }

  Stream<List<ScanResult>> get scanResults => FlutterBluePlus.scanResults;

  Future<bool> connectToDevice(BluetoothDevice device) async {
    _logger.info('Connecting to device: ${device.name}');
    
    try {
      _connectionStateController.add(DeviceConnectionState.connecting);
      
      await device.connect(timeout: const Duration(seconds: 15));
      _connectedDevice = device;
      
      // Listen for disconnection
      device.connectionState.listen((state) {
        if (state == BluetoothConnectionState.disconnected) {
          _handleDisconnection();
        }
      });

      // Discover services
      await _discoverServices();
      
      // Subscribe to characteristics
      await _subscribeToCharacteristics();
      
      _connectionStateController.add(DeviceConnectionState.connected);
      _logger.info('Successfully connected to device');
      
      return true;
    } catch (e) {
      _logger.severe('Error connecting to device: $e');
      _connectionStateController.add(DeviceConnectionState.disconnected);
      return false;
    }
  }

  Future<void> disconnect() async {
    if (_connectedDevice != null) {
      _logger.info('Disconnecting from device');
      await _connectedDevice!.disconnect();
      _handleDisconnection();
    }
  }

  void _handleDisconnection() {
    _logger.info('Device disconnected');
    _connectedDevice = null;
    _characteristics.clear();
    _connectionStateController.add(DeviceConnectionState.disconnected);
  }

  Future<void> _discoverServices() async {
    if (_connectedDevice == null) return;

    _logger.info('Discovering services...');
    
    List<BluetoothService> services = await _connectedDevice!.discoverServices();
    
    for (BluetoothService service in services) {
      _logger.info('Found service: ${service.uuid}');
      
      for (BluetoothCharacteristic characteristic in service.characteristics) {
        String uuid = characteristic.uuid.toString().toUpperCase();
        _characteristics[uuid] = characteristic;
        _logger.info('  Characteristic: $uuid');
      }
    }
  }

  Future<void> _subscribeToCharacteristics() async {
    _logger.info('Subscribing to characteristics...');

    // Heart Rate Measurement
    await _subscribeToCharacteristic(
      HEART_RATE_MEASUREMENT,
      _handleHeartRateData,
    );

    // Battery Level
    await _subscribeToCharacteristic(
      BATTERY_LEVEL,
      _handleBatteryData,
    );

    // HRV Data
    await _subscribeToCharacteristic(
      HRV_CHARACTERISTIC,
      _handleHRVData,
    );

    // Temperature Data
    await _subscribeToCharacteristic(
      TEMPERATURE_CHARACTERISTIC,
      _handleTemperatureData,
    );

    // Respiratory Rate
    await _subscribeToCharacteristic(
      RESPIRATORY_CHARACTERISTIC,
      _handleRespiratoryData,
    );

    // Health Flags
    await _subscribeToCharacteristic(
      HEALTH_FLAGS_CHARACTERISTIC,
      _handleHealthFlags,
    );
  }

  Future<void> _subscribeToCharacteristic(
    String uuid,
    Function(List<int>) handler,
  ) async {
    BluetoothCharacteristic? characteristic = _characteristics[uuid];
    
    if (characteristic != null) {
      try {
        await characteristic.setNotifyValue(true);
        characteristic.onValueReceived.listen(handler);
        _logger.info('Subscribed to $uuid');
      } catch (e) {
        _logger.warning('Failed to subscribe to $uuid: $e');
      }
    } else {
      _logger.warning('Characteristic $uuid not found');
    }
  }

  void _handleHeartRateData(List<int> data) {
    if (data.length >= 2) {
      int heartRate = data[1];
      List<int>? rrIntervals;
      
      // Parse RR intervals if available (flags indicate RR data present)
      if (data[0] & 0x10 != 0 && data.length > 2) {
        rrIntervals = [];
        for (int i = 2; i < data.length - 1; i += 2) {
          int rr = (data[i + 1] << 8) | data[i];
          rrIntervals.add(rr);
        }
      }
      
      HeartRateData hrData = HeartRateData(
        heartRate: heartRate,
        timestamp: DateTime.now(),
        rrIntervals: rrIntervals,
        quality: 95, // TODO: Parse from data or calculate
      );
      
      _heartRateController.add(hrData);
    }
  }

  void _handleBatteryData(List<int> data) {
    if (data.isNotEmpty) {
      int batteryLevel = data[0];
      _batteryController.add(batteryLevel);
    }
  }

  void _handleHRVData(List<int> data) {
    if (data.length >= 8) {
      // Parse HRV data (example format)
      double rmssd = _parseFloat32(data, 0);
      double sdnn = _parseFloat32(data, 4);
      
      VitalsData vitals = VitalsData(
        hrv: rmssd,
        sdnn: sdnn,
        timestamp: DateTime.now(),
      );
      
      _vitalsController.add(vitals);
    }
  }

  void _handleTemperatureData(List<int> data) {
    if (data.length >= 4) {
      double temperature = _parseFloat32(data, 0);
      
      // Update vitals with temperature
      // This is a simplified implementation
      VitalsData vitals = VitalsData(
        temperature: temperature,
        timestamp: DateTime.now(),
      );
      
      _vitalsController.add(vitals);
    }
  }

  void _handleRespiratoryData(List<int> data) {
    if (data.length >= 4) {
      double respiratoryRate = _parseFloat32(data, 0);
      
      VitalsData vitals = VitalsData(
        respiratoryRate: respiratoryRate,
        timestamp: DateTime.now(),
      );
      
      _vitalsController.add(vitals);
    }
  }

  void _handleHealthFlags(List<int> data) {
    if (data.isNotEmpty) {
      int flags = data[0];
      
      // Parse health alert flags
      if (flags & 0x01 != 0) {
        HealthAlert alert = HealthAlert(
          type: HealthAlertType.illnessPrediction,
          message: 'Possible illness detected - monitor your symptoms',
          severity: AlertSeverity.medium,
          timestamp: DateTime.now(),
        );
        _healthAlertsController.add(alert);
      }
      
      if (flags & 0x02 != 0) {
        HealthAlert alert = HealthAlert(
          type: HealthAlertType.lowHrv,
          message: 'HRV is significantly below your baseline',
          severity: AlertSeverity.high,
          timestamp: DateTime.now(),
        );
        _healthAlertsController.add(alert);
      }
    }
  }

  double _parseFloat32(List<int> data, int offset) {
    if (data.length < offset + 4) return 0.0;
    
    Uint8List bytes = Uint8List.fromList(data.sublist(offset, offset + 4));
    return ByteData.sublistView(bytes).getFloat32(0, Endian.little);
  }

  Future<void> syncData() async {
    if (_connectedDevice == null) return;

    _logger.info('Starting data sync...');
    
    BluetoothCharacteristic? dataChar = _characteristics[DATA_DOWNLOAD_CHARACTERISTIC];
    
    if (dataChar != null) {
      try {
        // Request data sync
        await dataChar.write([0x01]); // Sync command
        _logger.info('Data sync initiated');
      } catch (e) {
        _logger.severe('Error during data sync: $e');
      }
    }
  }

  Future<void> sendCommand(DeviceCommand command) async {
    // Implementation for sending commands to device
    _logger.info('Sending command: ${command.type}');
    // TODO: Implement command sending
  }

  void dispose() {
    _connectionStateController.close();
    _heartRateController.close();
    _vitalsController.close();
    _healthAlertsController.close();
    _batteryController.close();
  }
}
