# Signal Flow Documentation - Whoop Alternative

Vollständige Dokumentation des Signalflusses vom Hardware-Sensor bis zur mobilen Visualisierung.

## Übersicht der Signalverarbeitung

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐    ┌──────────────┐
│   Hardware  │───►│   Firmware   │───►│ BLE Transfer│───►│  Mobile App  │
│   Sensoren  │    │  Processing  │    │   Protocol  │    │Visualization │
└─────────────┘    └──────────────┘    └─────────────┘    └──────────────┘
```

## 1. Hardware Signal Acquisition

### 1.1 PPG-Sensor (MAX86141)

**Erfassungsort:** Oberarm  
**Hardware:** MAX86141 PPG AFE mit externen LEDs und Photodiode  
**I²C-Adresse:** 0x5E  

#### Signalkette:
```
LED (660nm/880nm) → Gewebe → Photodiode → TIA → ADC → Digital Filter → FIFO
```

#### Erfasste Rohsignale:
- **Red PPG (660nm):** SpO2-Messung, Pulswellenanalyse
- **IR PPG (880nm):** Herzfrequenz, HRV-Extraktion
- **Sample Rate:** 25-400 Hz (konfigurierbar)
- **Auflösung:** 18-bit ADC
- **FIFO-Puffer:** 128 Samples

#### Hardware-Konfiguration:
```c
// firmware/drivers/ppg/max86141.h
#define PPG_SAMPLE_RATE_100HZ   0x03
#define PPG_LED_CURRENT_6_2MA   0x1F
#define PPG_ADC_RANGE_4096nA    0x00
#define PPG_FIFO_DEPTH          128
```

### 1.2 IMU-Sensor (BMA400)

**Erfassungsort:** Oberarm (3-Achsen-Beschleunigung)  
**Hardware:** BMA400 Ultra-Low Power Accelerometer  
**I²C-Adresse:** 0x14  

#### Erfasste Signale:
- **X-Achse:** Lateral-Bewegung
- **Y-Achse:** Vertikal-Bewegung  
- **Z-Achse:** Vor-/Rückwärts-Bewegung
- **Sample Rate:** 12.5-800 Hz
- **Auflösung:** 12-bit
- **Messbereich:** ±2g, ±4g, ±8g, ±16g

#### Hardware-Features:
```c
// firmware/drivers/imu/bma400.h
#define BMA400_RANGE_2G         0x00
#define BMA400_ODR_25HZ         0x06
#define BMA400_STEP_COUNTER     0x01
#define BMA400_ACTIVITY_DETECT  0x02
```

### 1.3 Temperatur-Sensor (MAX30205)

**Erfassungsort:** Hauttemperatur am Oberarm  
**Hardware:** MAX30205 Precision Temperature Sensor  
**I²C-Adresse:** 0x48  

#### Erfasste Signale:
- **Körpertemperatur:** 28°C - 39°C
- **Auflösung:** 16-bit (0.00390625°C)
- **Genauigkeit:** ±0.1°C
- **Sample Rate:** 1 Hz (kontinuierlich)

## 2. Firmware Signal Processing

### 2.1 Sensor Manager (firmware/drivers/sensor_manager.c)

#### Zentrale Koordination:
```c
typedef struct {
    sensor_interface_t ppg;
    sensor_interface_t imu;
    sensor_interface_t temp;
    
    uint32_t sampling_rates[3];
    bool sensors_active[3];
    
    sensor_data_callback_t data_callback;
    sensor_error_callback_t error_callback;
} sensor_manager_t;
```

#### Datensammlung-Pipeline:
1. **Interrupt-driven Collection:** Sensoren triggern bei neuen Daten
2. **Time-synchronized Sampling:** Zeitstempel für alle Sensordaten
3. **Quality Check:** Signal-Quality-Assessment für jedes Sample
4. **Buffering:** Zwischenspeicherung in Ring-Buffern

### 2.2 PPG Processing Pipeline (firmware/modules/ppg_pipeline/)

#### Signalverarbeitungs-Schritte:

##### Schritt 1: Preprocessing
```c
// firmware/modules/ppg_pipeline/ppg_preprocessing.c
ppg_raw_sample_t → dc_removal() → bandpass_filter() → ppg_clean_sample_t
```
- **DC-Komponente entfernen:** High-Pass Filter (0.5 Hz)
- **Bandpass-Filter:** 0.5-10 Hz (Herzfrequenz-Band)
- **Artifact-Detection:** Bewegungsartefakte identifizieren

##### Schritt 2: Peak Detection
```c
// firmware/modules/ppg_pipeline/peak_detection.c
ppg_clean_sample_t → adaptive_threshold() → peak_validate() → rr_interval_t
```
- **Adaptive Schwellwerte:** Dynamische Peak-Erkennung
- **RR-Intervall-Extraktion:** Zeit zwischen Herzschlägen
- **Quality Metrics:** Signal-to-Noise Ratio, Peak-Konfidenz

##### Schritt 3: Heart Rate Calculation
```c
// firmware/modules/ppg_pipeline/hr_calculation.c
rr_interval_t → moving_average() → outlier_removal() → heart_rate_t
```
- **Gleitender Mittelwert:** 8-Sample Fenster
- **Outlier-Filterung:** Physiologisch unrealistische Werte entfernen
- **BPM-Berechnung:** 60000ms / RR-Intervall

### 2.3 HRV Analysis (firmware/modules/hrv/)

#### HRV-Metriken-Berechnung:
```c
// firmware/modules/hrv/hrv_analysis.c
typedef struct {
    float rmssd;        // Root Mean Square of Successive Differences
    float sdnn;         // Standard Deviation of NN intervals
    float pnn50;        // Percentage of adjacent NN intervals > 50ms
    float stress_index; // Derived stress metric
} hrv_metrics_t;
```

#### Zeitbereich-Analyse:
1. **RMSSD:** Kurzzeit-HRV (parasympathisches Nervensystem)
2. **SDNN:** Gesamt-HRV (24h Variabilität)
3. **pNN50:** Herzfrequenz-Flexibilität

#### Frequenzbereich-Analyse:
```c
// firmware/modules/hrv/frequency_analysis.c
- LF Power (0.04-0.15 Hz): Sympathisches + Parasympathisches
- HF Power (0.15-0.4 Hz):  Parasympathisches (Atmung)
- LF/HF Ratio:             Sympatho-vagale Balance
```

### 2.4 IMU Processing (firmware/modules/imu_algorithms/)

#### Activity Detection:
```c
// firmware/modules/imu_algorithms/activity_detection.c
typedef enum {
    ACTIVITY_SLEEPING,
    ACTIVITY_RESTING,
    ACTIVITY_LIGHT,
    ACTIVITY_MODERATE,
    ACTIVITY_VIGOROUS
} activity_level_t;
```

#### Signal Processing:
1. **Vector Magnitude:** √(x² + y² + z²)
2. **Moving Average:** Glättung über 5-Sekunden-Fenster
3. **Activity Classification:** Machine Learning-basierte Klassifikation
4. **Step Counting:** Peak-Detection in vertikaler Achse

### 2.5 Sleep Analysis (firmware/modules/sleep_light/)

#### Actigraphy-basierte Schlafanalyse:
```c
// firmware/modules/sleep_light/sleep_detection.c
typedef struct {
    uint32_t sleep_onset;      // Einschlafzeit
    uint32_t wake_time;        // Aufwachzeit
    uint32_t total_sleep;      // Gesamtschlafzeit
    uint32_t light_sleep;      // Leichtschlafphasen
    uint32_t deep_sleep;       // Tiefschlafphasen
    uint8_t sleep_efficiency;  // Schlafeffizienz %
} sleep_metrics_t;
```

#### Algorithmus:
1. **Movement Threshold:** Bewegungsschwelle für Schlaf/Wach
2. **Epoch Analysis:** 30-Sekunden-Epochen analysieren
3. **Sleep Stage Classification:** Light/Deep/REM-Phasen
4. **Circadian Rhythm:** 24h-Rhythmus-Tracking

### 2.6 Health Trends (firmware/modules/health_trends/)

#### Baseline-Tracking:
```c
// firmware/modules/health_trends/baseline_tracking.c
typedef struct {
    float rhr_baseline;        // Resting Heart Rate Baseline
    float hrv_baseline;        // HRV Baseline
    float temp_baseline;       // Temperature Baseline
    
    float rhr_deviation;       // Aktuelle Abweichung
    float hrv_deviation;       // Aktuelle Abweichung  
    float temp_deviation;      // Aktuelle Abweichung
    
    uint8_t health_score;      // 0-100 Gesundheitsscore
    bool illness_risk;         // Krankheitsrisiko-Flag
} health_baseline_t;
```

#### Anomalie-Erkennung:
1. **Rolling Baseline:** 14-Tage gleitender Durchschnitt
2. **Statistical Deviation:** Z-Score-basierte Anomalie-Erkennung
3. **Multi-Parameter Fusion:** Kombination aller Vital-Parameter
4. **Predictive Modeling:** Früherkennung von Krankheiten

## 3. BLE Data Transfer

### 3.1 BLE Services Architecture

#### Standard BLE Services:
```c
// firmware/ble/services/
┌─────────────────────────────────────────┐
│ Heart Rate Service (0x180D)             │
├─ Heart Rate Measurement (0x2A37)        │
├─ RR-Interval Data                       │
└─ Body Sensor Location (0x2A38)          │
│                                         │
│ Battery Service (0x180F)                │
├─ Battery Level (0x2A19)                 │
└─ Battery Status                         │
│                                         │
│ Device Information (0x180A)             │
├─ Manufacturer Name (0x2A29)             │
├─ Model Number (0x2A24)                  │
└─ Firmware Revision (0x2A26)             │
└─────────────────────────────────────────┘
```

#### Custom Whoop Service:
```c
// firmware/ble/services/whoop_service.c
#define WHOOP_SERVICE_UUID          "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define WHOOP_HRV_CHAR_UUID         "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define WHOOP_SLEEP_CHAR_UUID       "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define WHOOP_STRAIN_CHAR_UUID      "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"
#define WHOOP_HEALTH_CHAR_UUID      "6E400005-B5A3-F393-E0A9-E50E24DCCA9E"
```

### 3.2 Data Transmission Protocol

#### Packet Structure:
```c
typedef struct {
    uint8_t packet_type;       // DATA_TYPE_HRV, DATA_TYPE_SLEEP, etc.
    uint8_t sequence_number;   // Für Packet-Ordering
    uint16_t timestamp;        // Zeitstempel
    uint16_t data_length;      // Nutzdaten-Länge
    uint8_t data[];           // Nutzdaten
    uint8_t checksum;         // CRC8-Prüfsumme
} ble_packet_t;
```

#### Datentypen:
```c
#define DATA_TYPE_REALTIME_HR    0x01
#define DATA_TYPE_HRV_METRICS    0x02
#define DATA_TYPE_SLEEP_DATA     0x03
#define DATA_TYPE_ACTIVITY_DATA  0x04
#define DATA_TYPE_HEALTH_TRENDS  0x05
#define DATA_TYPE_RAW_PPG        0x06  // Debug-Modus
```

#### Übertragungsraten:
- **Real-time HR:** 1 Hz (jede Sekunde)
- **HRV Metrics:** 0.017 Hz (alle 60 Sekunden)
- **Sleep Data:** 0.0003 Hz (alle 5 Minuten)
- **Health Trends:** 0.00012 Hz (alle 2 Stunden)

## 4. Mobile App Data Processing

### 4.1 BLE Data Reception (lib/features/device/)

#### BLE Service Manager:
```dart
// lib/features/device/data/ble_service_manager.dart
class BLEServiceManager {
  Stream<HeartRateData> get heartRateStream;
  Stream<HRVData> get hrvStream;
  Stream<SleepData> get sleepStream;
  Stream<HealthData> get healthStream;
  
  Future<void> connectToDevice(String deviceId);
  Future<void> startDataCollection();
  Future<void> syncHistoricalData();
}
```

#### Data Models:
```dart
// lib/shared/models/sensor_data.dart
class HeartRateData {
  final int bpm;
  final List<int> rrIntervals;
  final DateTime timestamp;
  final double signalQuality;
}

class HRVData {
  final double rmssd;
  final double sdnn;
  final double pnn50;
  final double stressIndex;
  final DateTime timestamp;
}

class SleepData {
  final DateTime bedtime;
  final DateTime wakeTime;
  final Duration totalSleep;
  final Duration deepSleep;
  final Duration lightSleep;
  final double sleepEfficiency;
}
```

### 4.2 Data Storage (lib/features/data/)

#### Local Database Schema:
```dart
// lib/features/data/local_database.dart
// SQLite Tables:
CREATE TABLE heart_rate_data (
  id INTEGER PRIMARY KEY,
  timestamp INTEGER NOT NULL,
  bpm INTEGER NOT NULL,
  rr_intervals TEXT,
  signal_quality REAL
);

CREATE TABLE hrv_data (
  id INTEGER PRIMARY KEY,
  timestamp INTEGER NOT NULL,
  rmssd REAL NOT NULL,
  sdnn REAL NOT NULL,
  pnn50 REAL NOT NULL,
  stress_index REAL
);

CREATE TABLE sleep_sessions (
  id INTEGER PRIMARY KEY,
  date TEXT NOT NULL,
  bedtime INTEGER,
  wake_time INTEGER,
  total_sleep INTEGER,
  deep_sleep INTEGER,
  light_sleep INTEGER,
  sleep_efficiency REAL
);
```

### 4.3 Data Visualization (lib/features/*/presentation/)

#### Real-time Dashboard (lib/features/monitoring/):
```dart
// lib/features/monitoring/presentation/live_monitoring_screen.dart
class LiveMonitoringScreen extends StatelessWidget {
  Widget build(BuildContext context) {
    return Scaffold(
      body: Column(
        children: [
          HeartRateGauge(),        // Echzeit-Herzfrequenz
          HRVTrendChart(),         // HRV-Trend (letzte 6h)
          ActivityIndicator(),     // Aktivitätslevel
          HealthStatusCard(),      // Gesundheitsstatus
        ],
      ),
    );
  }
}
```

#### Sleep Analysis (lib/features/sleep/):
```dart
// lib/features/sleep/presentation/sleep_screen.dart
class SleepScreen extends StatelessWidget {
  Widget build(BuildContext context) {
    return Scaffold(
      body: Column(
        children: [
          SleepHypnogram(),        // Schlafphasen-Diagramm
          SleepMetricsCards(),     // Schlafmetriken
          SleepTrendChart(),       // 7/30-Tage Trend
          HRVDuringSleep(),        // HRV während Schlaf
        ],
      ),
    );
  }
}
```

#### Strain & Recovery (lib/features/strain/):
```dart
// lib/features/strain/presentation/strain_screen.dart
class StrainScreen extends StatelessWidget {
  Widget build(BuildContext context) {
    return Scaffold(
      body: Column(
        children: [
          StrainGauge(),           // Tagesbelastung 0-21
          HeartRateZones(),        // HR-Zonen-Verteilung
          RecoveryScore(),         // Recovery-Score
          TrainingLoadChart(),     // Trainingsbelastung
        ],
      ),
    );
  }
}
```

#### Health Monitoring (lib/features/health/):
```dart
// lib/features/health/presentation/health_screen.dart
class HealthScreen extends StatelessWidget {
  Widget build(BuildContext context) {
    return Scaffold(
      body: Column(
        children: [
          HealthScoreCard(),       // Gesundheitsscore 0-100
          BaselineTrends(),        // Baseline-Abweichungen
          IllnessPrediction(),     // Krankheitsvorhersage
          VitalTrendsChart(),      // Vitalparameter-Trends
        ],
      ),
    );
  }
}
```

## 5. Complete Signal Flow Summary

### 5.1 Datenfluss von Sensor zu App

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           COMPLETE SIGNAL FLOW                             │
└─────────────────────────────────────────────────────────────────────────────┘

Hardware Sensors:
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│  MAX86141   │    │   BMA400    │    │  MAX30205   │
│ PPG Sensor  │    │ IMU Sensor  │    │ Temp Sensor │
│             │    │             │    │             │
│ • Red LED   │    │ • X-Axis    │    │ • Body Temp │
│ • IR LED    │    │ • Y-Axis    │    │ • 16-bit    │
│ • Photodiode│    │ • Z-Axis    │    │ • ±0.1°C    │
│ • 18-bit ADC│    │ • 12-bit    │    │ • 1 Hz      │
│ • 25-400 Hz │    │ • 12.5-800Hz│    │             │
└──────┬──────┘    └──────┬──────┘    └──────┬──────┘
       │                  │                  │
       └──────────────────┼──────────────────┘
                          │
                    ┌─────▼─────┐
                    │ I²C Bus   │
                    │ nRF52840  │
                    └─────┬─────┘
                          │
Firmware Processing:      │
┌─────────────────────────▼─────────────────────────┐
│                Sensor Manager                     │
│ • Interrupt-driven collection                     │
│ • Time-synchronized sampling                      │
│ • Quality assessment                              │
│ • Ring buffer storage                             │
└─────────────────────┬───────────────────────┬─────┘
                      │                       │
           ┌──────────▼──────────┐    ┌───────▼───────┐
           │   PPG Pipeline      │    │ IMU/Temp Proc │
           │ • DC removal        │    │ • Activity det │
           │ • Bandpass filter   │    │ • Step count   │
           │ • Peak detection    │    │ • Sleep detect │
           │ • HR calculation    │    │ • Temp monitor │
           │ • HRV analysis      │    │               │
           └──────────┬──────────┘    └───────┬───────┘
                      │                       │
                      └──────────┬────────────┘
                                 │
                    ┌────────────▼────────────┐
                    │    Health Analytics     │
                    │ • Baseline tracking     │
                    │ • Anomaly detection     │
                    │ • Illness prediction    │
                    │ • Stress analysis       │
                    └────────────┬────────────┘
                                 │
BLE Transmission:                │
                    ┌────────────▼────────────┐
                    │     BLE Services        │
                    │ • Heart Rate Service    │
                    │ • Custom Whoop Service  │
                    │ • Battery Service       │
                    │ • Data streaming        │
                    └────────────┬────────────┘
                                 │
                       ┌─────────▼─────────┐
                       │  Bluetooth LE     │
                       │   2.4GHz Radio    │
                       └─────────┬─────────┘
                                 │
Mobile App Reception:            │
                    ┌────────────▼────────────┐
                    │  BLE Service Manager    │
                    │ • Device connection     │
                    │ • Data stream handling  │
                    │ • Packet validation     │
                    │ • Error recovery        │
                    └────────────┬────────────┘
                                 │
                    ┌────────────▼────────────┐
                    │   Local Database        │
                    │ • SQLite storage        │
                    │ • Historical data       │
                    │ • Sync management       │
                    │ • Export capabilities   │
                    └────────────┬────────────┘
                                 │
User Interface:                  │
                    ┌────────────▼────────────┐
                    │   Data Visualization    │
                    │ • Real-time displays    │
                    │ • Charts & graphs       │
                    │ • Trend analysis        │
                    │ • Health insights       │
                    │ • Alert notifications   │
                    └─────────────────────────┘
```

### 5.2 Datenrate und -volumen

#### Echzeit-Datenströme:
- **PPG Raw:** 100 Hz × 3 Bytes = 300 B/s
- **Heart Rate:** 1 Hz × 4 Bytes = 4 B/s
- **HRV Metrics:** 1/60 Hz × 16 Bytes = 0.27 B/s
- **IMU Data:** 25 Hz × 6 Bytes = 150 B/s
- **Temperature:** 1 Hz × 2 Bytes = 2 B/s

**Gesamt BLE-Datenrate:** ~456 Bytes/Sekunde

#### Historische Daten (pro Tag):
- **Heart Rate:** 86,400 Samples × 4 Bytes = 345.6 KB
- **HRV Data:** 1,440 Samples × 16 Bytes = 23.04 KB
- **Sleep Data:** 1 Session × 64 Bytes = 64 Bytes
- **Activity Data:** 17,280 Samples × 8 Bytes = 138.24 KB

**Gesamt pro Tag:** ~507 KB

### 5.3 Latenz-Budget

```
Signal Acquisition to Display:
┌─────────────────────────────────────────────────────────────┐
│ Component                │ Latency    │ Cumulative Total    │
├─────────────────────────────────────────────────────────────┤
│ Sensor Sampling          │ 10-40ms    │ 10-40ms            │
│ Firmware Processing      │ 20-50ms    │ 30-90ms            │
│ BLE Transmission         │ 7.5-30ms   │ 37.5-120ms         │
│ App Processing           │ 10-20ms    │ 47.5-140ms         │
│ UI Rendering             │ 16.67ms    │ 64.2-156.7ms       │
├─────────────────────────────────────────────────────────────┤
│ TOTAL END-TO-END LATENCY │            │ ~65-157ms          │
└─────────────────────────────────────────────────────────────┘
```

### 5.4 Energieverbrauch

#### Sensor Power Consumption:
- **MAX86141:** 0.3 mA (kontinuierlich)
- **BMA400:** 0.18 mA @ 25Hz
- **MAX30205:** 0.01 mA (kontinuierlich)
- **BLE Radio:** 0.5 mA (durchschnittlich)
- **nRF52840:** 0.8 mA (processing)

**Total System:** ~1.8 mA → 111 Stunden Batterielaufzeit

## 6. Debugging und Monitoring

### 6.1 Signal Quality Monitoring

#### PPG Signal Quality:
```c
// firmware/modules/ppg_pipeline/signal_quality.c
typedef struct {
    float snr;              // Signal-to-Noise Ratio
    float perfusion_index;  // PI (Signal strength)
    float motion_artifact;  // Movement detection
    bool valid_signal;      // Overall quality flag
} ppg_quality_t;
```

#### Echtzeit-Debugging:
- **UART Debug Output:** Rohsignale und verarbeitete Daten
- **BLE Debug Service:** Raw-Daten-Streaming zur App
- **LED-Indikatoren:** Visueller Status auf Hardware
- **Mobile Debug Screen:** Live-Signal-Visualisierung

### 6.2 Data Validation

#### Plausibility Checks:
```c
// Physiologische Grenzen
#define MIN_HEART_RATE_BPM    30
#define MAX_HEART_RATE_BPM    220
#define MIN_HRV_RMSSD_MS      5.0
#define MAX_HRV_RMSSD_MS      200.0
#define MIN_BODY_TEMP_C       30.0
#define MAX_BODY_TEMP_C       42.0
```

#### Error Handling:
- **Sensor Timeout Detection**
- **I²C Communication Errors**
- **BLE Connection Drops**
- **Data Corruption Checks**
- **Automatic Recovery Mechanisms**

---

## Zusammenfassung

Dieses Dokument beschreibt den kompletten Signalfluss vom Hardware-Sensor bis zur mobilen Visualisierung im Whoop-Alternative System. Der Datenfluss umfasst:

1. **Hardware-Akquisition:** Drei Hauptsensoren am Oberarm erfassen Vitalparameter
2. **Firmware-Verarbeitung:** Echzeit-Signalverarbeitung und Feature-Extraktion
3. **BLE-Übertragung:** Effiziente drahtlose Datenübertragung
4. **Mobile-Visualisierung:** Benutzerfreundliche Darstellung und Analyse

Das System ist für niedrigen Stromverbrauch, hohe Datenqualität und geringe Latenz optimiert, um eine professionelle Gesundheitsüberwachung zu ermöglichen.
