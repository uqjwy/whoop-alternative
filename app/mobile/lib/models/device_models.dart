enum DeviceConnectionState {
  disconnected,
  connecting,
  connected,
  error,
}

enum DeviceCommandType {
  startMeasurement,
  stopMeasurement,
  calibrate,
  sync,
  reboot,
  factoryReset,
}

class DeviceCommand {
  final DeviceCommandType type;
  final Map<String, dynamic>? parameters;
  final DateTime timestamp;

  DeviceCommand({
    required this.type,
    this.parameters,
    DateTime? timestamp,
  }) : timestamp = timestamp ?? DateTime.now();
}

class DeviceInfo {
  final String name;
  final String address;
  final String? firmwareVersion;
  final String? hardwareVersion;
  final String? serialNumber;
  final int? batteryLevel;
  final DeviceConnectionState connectionState;

  DeviceInfo({
    required this.name,
    required this.address,
    this.firmwareVersion,
    this.hardwareVersion,
    this.serialNumber,
    this.batteryLevel,
    this.connectionState = DeviceConnectionState.disconnected,
  });

  DeviceInfo copyWith({
    String? name,
    String? address,
    String? firmwareVersion,
    String? hardwareVersion,
    String? serialNumber,
    int? batteryLevel,
    DeviceConnectionState? connectionState,
  }) {
    return DeviceInfo(
      name: name ?? this.name,
      address: address ?? this.address,
      firmwareVersion: firmwareVersion ?? this.firmwareVersion,
      hardwareVersion: hardwareVersion ?? this.hardwareVersion,
      serialNumber: serialNumber ?? this.serialNumber,
      batteryLevel: batteryLevel ?? this.batteryLevel,
      connectionState: connectionState ?? this.connectionState,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'name': name,
      'address': address,
      'firmwareVersion': firmwareVersion,
      'hardwareVersion': hardwareVersion,
      'serialNumber': serialNumber,
      'batteryLevel': batteryLevel,
      'connectionState': connectionState.toString(),
    };
  }

  factory DeviceInfo.fromJson(Map<String, dynamic> json) {
    return DeviceInfo(
      name: json['name'],
      address: json['address'],
      firmwareVersion: json['firmwareVersion'],
      hardwareVersion: json['hardwareVersion'],
      serialNumber: json['serialNumber'],
      batteryLevel: json['batteryLevel'],
      connectionState: DeviceConnectionState.values.firstWhere(
        (e) => e.toString() == json['connectionState'],
        orElse: () => DeviceConnectionState.disconnected,
      ),
    );
  }
}
