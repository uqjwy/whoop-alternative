import 'package:permission_handler/permission_handler.dart';
import 'package:flutter/foundation.dart';

class PermissionService {
  static Future<void> requestAllPermissions() async {
    // Skip permissions on web platform
    if (kIsWeb) {
      return;
    }
    
    await _requestBluetoothPermissions();
    await _requestLocationPermissions();
    await _requestNotificationPermissions();
  }

  static Future<void> _requestBluetoothPermissions() async {
    if (kIsWeb) return;
    
    await Permission.bluetoothScan.request();
    await Permission.bluetoothConnect.request();
    await Permission.bluetoothAdvertise.request();
  }

  static Future<void> _requestLocationPermissions() async {
    if (kIsWeb) return;
    
    await Permission.location.request();
    await Permission.locationWhenInUse.request();
  }

  static Future<void> _requestNotificationPermissions() async {
    if (kIsWeb) return;
    
    await Permission.notification.request();
  }

  static Future<bool> hasBluetoothPermissions() async {
    if (kIsWeb) return true; // Assume granted on web
    
    return await Permission.bluetoothScan.isGranted &&
           await Permission.bluetoothConnect.isGranted;
  }

  static Future<bool> hasLocationPermissions() async {
    if (kIsWeb) return true; // Assume granted on web
    
    return await Permission.location.isGranted ||
           await Permission.locationWhenInUse.isGranted;
  }

  static Future<bool> hasNotificationPermissions() async {
    if (kIsWeb) return true; // Assume granted on web
    
    final status = await Permission.notification.request();
    return status.isGranted;
  }
}
