# Sensor-Agnostic Firmware Architecture

## Übersicht

Diese Firmware-Architektur ermöglicht den einfachen Wechsel zwischen verschiedenen Sensoren (z.B. MAX30101 → MAX86141) ohne größere Code-Änderungen. Die Abstraktionsschicht entkoppelt die Hardware von der Signalverarbeitung.

## Architektur-Prinzipien

### 1. Sensor-Abstraktionsschicht
```c
// Einheitliches Interface für alle PPG-Sensoren
typedef struct ppg_sensor_ops {
    bool (*init)(const ppg_config_t* config);
    bool (*start)(void);
    int (*read_fifo)(ppg_sample_t* samples, int max);
    bool (*stop)(void);
    // ...
} ppg_sensor_ops_t;
```

### 2. Einheitliches Datenformat
```c
// Normalisierte Rohdaten für alle Sensoren
typedef struct {
    uint32_t timestamp;     // Systemzeit (ms)
    int32_t channels[4];    // [Red, IR, Green, UV] normalisiert
    uint8_t led_slots;      // Aktive LED-Slots
    int16_t temperature;    // Temperatur (0.01°C)
    uint8_t quality;        // Signalqualität (0-100%)
} ppg_sample_t;
```

### 3. Konfiguration statt Code-Änderungen
```ini
[sensor_system]
ppg_sensor=MAX30101    # Einfach zu MAX86141 ändern
imu_sensor=BMA400

[ppg_config]
sample_rate=100
led_current_red=25
led_current_ir=25
```

## Verzeichnisstruktur

```
firmware/
├── drivers/
│   ├── interfaces/          # Abstraktionsschicht
│   │   ├── sensor_interfaces.h
│   │   └── sensor_config.h
│   ├── ppg/                 # PPG-Sensor-Treiber
│   │   ├── max30101_driver.c
│   │   └── max86141_driver.c  (geplant)
│   ├── imu/                 # IMU-Sensor-Treiber
│   │   ├── bma400_driver.c
│   │   └── bmi270_driver.c    (geplant)
│   └── sensor_manager.c     # Zentrale Sensorverwaltung
├── modules/
│   ├── ppg_pipeline/        # Signalverarbeitung
│   ├── hrv/                 # HRV-Analyse
│   ├── health_monitor/      # Gesundheitsüberwachung
│   └── data_logger/         # Datenprotokollierung
├── config/
│   └── sensor_config.txt    # Konfigurationsdatei
└── tests/
    └── sensor_abstraction_test.c
```

## Sensor-Wechsel Workflow

### Von MAX30101 zu MAX86141

1. **Neue Treiber implementieren:**
```c
// max86141_driver.c
const ppg_sensor_ops_t max86141_ops = {
    .init = max86141_init,
    .start = max86141_start,
    .read_fifo = max86141_read_fifo,
    // ...
};
```

2. **Konfiguration anpassen:**
```ini
# config/sensor_config.txt
ppg_sensor=MAX86141
led_current_red=50      # Höhere Ströme für AFE
led_current_ir=50
led_current_green=25
```

3. **Kalibrierung anpassen:**
```ini
[signal_processing]
spo2_cal_red=1.2        # Sensor-spezifische Konstanten
spo2_cal_ir=0.9
```

**Das war's!** Keine Änderungen an der Pipeline oder App-Logik nötig.

## Unterstützte Sensoren

### PPG-Sensoren
- ✅ **MAX30101** - Integrierte Red+IR LEDs
- 🔄 **MAX86141** - AFE für externe LEDs (geplant)
- 🔄 **MAXM86146** - Miniatur-AFE (geplant)

### IMU-Sensoren  
- ✅ **BMA400** - Low-Power Accelerometer
- 🔄 **BMI270** - Accel + Gyro (geplant)
- 🔄 **ICM-42688** - High-Performance IMU (geplant)

## Datenprotokollierung & Replay

### Raw Data Logging
```c
// Protokolliert alle Rohdaten für Offline-Analyse
sensor_logger_log_synchronized(&logger, &sample);
```

### Replay-System
```c
// Testet Algorithmen mit aufgezeichneten Daten
sensor_replay_read_synchronized(&replay, &sample);
```

**Vorteil:** Beim Sensor-Wechsel können Algorithmen mit den gleichen Daten getestet werden.

## Quick Start

### 1. Build System
```bash
# Alle Tests bauen
make all

# Abstraktionsschicht testen
make abstraction_test
make run_abstraction
```

### 2. Konfiguration ändern
```bash
# Sensor-Typ wechseln
nano config/sensor_config.txt

# Konfiguration validieren
make check_config
```

### 3. Neue Sensoren hinzufügen
```bash
# Unterstützte Sensoren anzeigen
make show_sensors

# Neuen Treiber implementieren
cp drivers/ppg/max30101_driver.c drivers/ppg/new_sensor_driver.c
# Implementierung anpassen...

# In sensor_manager.c registrieren
```

## Implementierungsaufwand

### Mit Abstraktionsschicht ✅
- Neuer Treiber: ~1-2 Tage
- Parameter-Tuning: ~1 Tag
- **Gesamt: ~3 Tage**

### Ohne Abstraktionsschicht ❌
- Code-Anpassungen überall: ~1 Woche
- Pipeline-Umschreibung: ~3 Tage
- Testing & Debug: ~2 Tage
- **Gesamt: ~2 Wochen**

## Nächste Schritte

1. ✅ **Abstraktionsschicht implementiert**
2. ✅ **MAX30101 Treiber fertig**
3. 🔄 **MAX86141 Treiber implementieren**
4. 🔄 **BMI270 IMU-Treiber**
5. 🔄 **Parameter-Tuning für AFE**

## Testing

### Sensor-Abstraktionstest
```bash
# Vollständiger Architektur-Test
./build/abstraction_test

# Ergebnis:
# ✅ Sensor Initialization: PASS
# ✅ Data Logging: PASS  
# ✅ Data Acquisition: PASS
# ✅ Config Switching: PASS
# ✅ Health Monitoring: PASS
```

### Konfigurationstest
```bash
# Verschiedene Profile testen
./build/config_test development
./build/config_test production
./build/config_test low_power
```

## Fazit

Diese Architektur macht den Sensor-Wechsel von MAX30101 zu MAX86141 **trivial**:

1. **Nur neuer Treiber** - keine Pipeline-Änderungen
2. **Konfigurationsdatei** - kein Code-Recompile 
3. **Gleiche Daten-Pipeline** - bewährte Algorithmen
4. **Replay-System** - Validierung mit echten Daten

**Ready for MAX86141!** 🚀
