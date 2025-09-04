# Mobile App - Flutter

Cross-platform mobile application for the Whoop Alternative wearable device.

## Features

### Core Functionality
- **Device Pairing:** Bluetooth Low Energy connection management
- **Live Monitoring:** Real-time heart rate display
- **Health Metrics:** HRV, sleep analysis, strain monitoring
- **Data Sync:** Automatic data synchronization from device
- **Health Alerts:** "Getting sick" predictions and notifications
- **Charts & Trends:** Historical data visualization

### Screens & Navigation
```
├── Home Screen
│   ├── Current Heart Rate
│   ├── Today's Metrics
│   ├── Battery Status
│   └── Device Connection
├── Sleep Screen
│   ├── Last Night's Sleep
│   ├── Sleep Trends (7/30 days)
│   ├── HRV Analysis
│   └── Sleep Score
├── Strain Screen
│   ├── Daily Strain
│   ├── Heart Rate Zones
│   ├── Recovery Status
│   └── Training Load
├── Health Monitor
│   ├── Baseline Trends
│   ├── Anomaly Alerts
│   ├── Illness Predictions
│   └── Health Score
├── Data Screen
│   ├── Raw Data Export
│   ├── Sync Status
│   ├── Storage Usage
│   └── Data History
└── Settings
    ├── Device Settings
    ├── Alert Preferences
    ├── User Profile
    ├── Privacy Settings
    └── App Info
```

## Technology Stack

- **Framework:** Flutter 3.x
- **State Management:** Provider / Riverpod
- **BLE Communication:** flutter_blue_plus
- **Charts:** fl_chart
- **Local Storage:** hive / sqflite
- **Notifications:** flutter_local_notifications
- **Export:** csv, share_plus

## Project Structure

```
lib/
├── main.dart                   # App entry point
├── app/                       # App configuration
│   ├── app.dart              # App widget
│   ├── routes.dart           # Navigation routes
│   └── theme.dart            # App theming
├── core/                     # Core utilities
│   ├── constants/            # App constants
│   ├── errors/              # Error handling
│   ├── utils/               # Utility functions
│   └── extensions/          # Dart extensions
├── features/                 # Feature modules
│   ├── device/              # Device connection & BLE
│   │   ├── data/            # BLE services, data models
│   │   ├── domain/          # Business logic
│   │   └── presentation/    # UI components
│   ├── monitoring/          # Live monitoring
│   ├── sleep/               # Sleep analysis
│   ├── strain/              # Strain tracking
│   ├── health/              # Health trends
│   ├── data/                # Data export/import
│   └── settings/            # App settings
├── shared/                   # Shared components
│   ├── widgets/             # Reusable widgets
│   ├── models/              # Data models
│   └── services/            # Shared services
└── assets/                   # Static assets
    ├── images/
    ├── icons/
    └── fonts/
```

## BLE Services Integration

### Standard Services
- **Heart Rate Service (0x180D)**
  - Heart Rate Measurement Characteristic
  - RR-Interval Data
  - Body Sensor Location

- **Battery Service (0x180F)**
  - Battery Level Characteristic
  - Battery Status

### Custom Services
- **Vitals Service (Custom UUID)**
  - HRV Characteristic (RMSSD, SDNN)
  - Temperature Characteristic
  - Respiratory Rate Characteristic

- **Health Monitor Service (Custom UUID)**
  - Baseline Status Characteristic
  - Anomaly Flags Characteristic
  - Illness Prediction Characteristic

- **Data Sync Service (Custom UUID)**
  - Log Download Characteristic (512B chunks)
  - Sync Status Characteristic
  - Device Info Characteristic

## Key Features Implementation

### Device Connection
```dart
class DeviceConnectionService {
  // Scan for devices
  Future<List<BluetoothDevice>> scanForDevices();
  
  // Connect to device
  Future<bool> connectToDevice(BluetoothDevice device);
  
  // Subscribe to services
  Future<void> subscribeToServices();
  
  // Handle disconnection
  void handleDisconnection();
}
```

### Data Synchronization
```dart
class DataSyncService {
  // Sync health metrics
  Future<void> syncHealthMetrics();
  
  // Download logs
  Future<void> downloadLogs(DateTime from, DateTime to);
  
  // Parse received data
  HealthMetrics parseHealthData(List<int> data);
}
```

### Chart Visualization
```dart
class HealthChartsWidget extends StatelessWidget {
  // Heart rate chart
  Widget buildHeartRateChart(List<HeartRateData> data);
  
  // HRV trend chart
  Widget buildHRVChart(List<HRVData> data);
  
  // Sleep analysis chart
  Widget buildSleepChart(SleepData data);
}
```

## Development Setup

### Prerequisites
```bash
# Install Flutter
flutter doctor

# Install dependencies
flutter pub get

# Run app
flutter run
```

### BLE Testing
```bash
# Enable BLE permissions (Android)
# Add to android/app/src/main/AndroidManifest.xml:
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />

# iOS BLE setup in ios/Runner/Info.plist:
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app uses Bluetooth to connect to your health monitor</string>
```

## Data Models

### Health Metrics
```dart
class HealthMetrics {
  final DateTime timestamp;
  final int heartRate;
  final double hrv;
  final double temperature;
  final int respiratoryRate;
  final int batteryLevel;
  final bool isCharging;
}

class SleepData {
  final DateTime bedtime;
  final DateTime wakeTime;
  final Duration totalSleep;
  final List<SleepStage> stages;
  final double sleepScore;
  final double averageHRV;
}

class StrainData {
  final DateTime date;
  final double strainScore;
  final Map<HeartRateZone, Duration> timeInZones;
  final int calories;
  final Duration activeTime;
}
```

## Notifications

### Health Alerts
- Low HRV notifications
- "Getting sick" predictions
- Battery low warnings
- Device disconnection alerts

### Implementation
```dart
class NotificationService {
  // Schedule health alert
  Future<void> scheduleHealthAlert(HealthAlert alert);
  
  // Show illness prediction
  Future<void> showIllnessPrediction(IllnessPrediction prediction);
  
  // Battery notifications
  Future<void> showBatteryAlert(int batteryLevel);
}
```

## Export & Sharing

### Data Export Formats
- **CSV:** Raw sensor data export
- **PDF:** Health reports and summaries
- **JSON:** Structured data for analysis

### Privacy & Security
- Local data encryption
- Secure BLE pairing
- GDPR compliance features
- Data anonymization options

## Testing Strategy

### Unit Tests
- BLE service mocking
- Data parsing validation
- Chart data processing
- Health calculation algorithms

### Integration Tests
- End-to-end BLE communication
- Data sync workflows
- Notification handling
- Export functionality

### Widget Tests
- UI component testing
- Chart rendering validation
- Navigation testing
- State management verification
