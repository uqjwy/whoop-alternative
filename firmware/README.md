# Firmware - nRF52840 Zephyr RTOS

Firmware für den Whoop-Alternative Wearable mit nRF52840 MCU und Zephyr RTOS.

## Hardware-Zielplattform

- **MCU:** nRF52840 (ARM Cortex-M4F, 1MB Flash, 256KB RAM)
- **BLE:** Bluetooth 5.0 Low Energy
- **Sensoren:** MAX86141 (PPG), BMA400 (IMU), MAX30205 (Temp)
- **Speicher:** W25Q64 (8MB SPI Flash)
- **Power:** BQ24074 + TPS62740 + MAX17048

## Projektstruktur

```
app/                    # Hauptanwendung
├── src/
│   ├── main.c         # Main entry point
│   ├── app_config.h   # Konfiguration
│   └── app_states.c   # State machine
├── prj.conf           # Zephyr Konfiguration
├── CMakeLists.txt     # Build system
└── Kconfig           # Konfigurationsoptionen

drivers/               # Hardware-Treiber
├── ppg/              # MAX86141 PPG Sensor
├── imu/              # BMA400 IMU
├── temp/             # MAX30205 Temperatur
├── flash/            # W25Q64 SPI Flash
└── fuel/             # MAX17048 Fuel Gauge

modules/              # Signalverarbeitung
├── ppg_pipeline/     # PPG-Filterung & Peak-Detection
├── hrv/             # HRV-Berechnung (RMSSD, etc.)
├── resp/            # Atemfrequenz-Extraktion
├── sleep_light/     # Schlafanalyse (Actigraphy)
├── strain/          # Belastungsberechnung
└── health_trends/   # Baseline-Tracking & Anomalie-Erkennung

ble/                 # Bluetooth Services
├── services/        # GATT Services (HRS, Battery, Custom)
└── gap/            # GAP Advertising & Connection

storage/             # Datenspeicherung
├── ringbuffer/      # Zirkulärer Puffer
└── flash_io/       # Flash-Dateisystem

power/               # Energiemanagement
├── duty_cycling/    # Sensor-Sampling-Kontrolle
└── sleep_mgmt/     # Deep-Sleep Management

tests/              # Tests
├── unit/           # Unit Tests
└── hil/           # Hardware-in-the-Loop Tests
```

## Build-System (Zephyr/West)

```bash
# Projekt initialisieren
west init -m https://github.com/nrfconnect/sdk-nrf --mr main
west update

# Firmware bauen
cd firmware/app
west build -b nrf52840dk_nrf52840

# Firmware flashen
west flash

# Debug über J-Link
west debug
```

## Sensor-Konfiguration

### PPG (MAX86141)
- **Sampling:** 50 Hz (Ruhe), 25 Hz (Aktivität)
- **LED-Strom:** Adaptiv 5-50 mA
- **Filter:** Bandpass 0.7-4 Hz

### IMU (BMA400)
- **Modus:** Low-Power mit Interrupt
- **ODR:** 25 Hz
- **Range:** ±8g

### Temperatur (MAX30205)
- **Sampling:** 1/min (nachts), 1/10min (tags)
- **Auflösung:** 0.00390625°C

## Power-Budget

| Komponente | Aktiv | Sleep | Duty Cycle |
|------------|-------|-------|------------|
| nRF52840   | 8 mA  | 1.5 µA| 10%        |
| MAX86141   | 15 mA | 0.1 µA| 2%         |
| BMA400     | 0.18 mA| 0.1 µA| 100%      |
| Flash      | 15 mA | 1 µA  | 0.1%       |
| **Total**  | **~1.2 mA** (Durchschnitt für 7 Tage) |

## BLE-Services

### Standard Services
- **Heart Rate Service (0x180D):** Live HR + RR-Intervalle
- **Battery Service (0x180F):** Akkustand

### Custom Services
- **Vitals Service:** HRV, Temperatur, Atemfrequenz
- **Health Monitor Service:** Baseline-Status, Anomalie-Flags
- **Data Sync Service:** Log-Download (512B Chunks)

## Entwicklungsworkflow

1. **Hardware bring-up:** I²C/SPI-Tests mit Breakout-Boards
2. **Treiber-Entwicklung:** Sensor-Integration einzeln testen
3. **Algorithmus-Integration:** PPG-Pipeline → HR → HRV
4. **Power-Optimierung:** Duty-Cycling & Sleep-Management
5. **BLE-Integration:** Services & Mobile-App-Konnektivität
6. **Validation:** 24h-Tests & Signalqualität vs. Referenz

## Debug & Logging

- **RTT:** Segger Real-Time Transfer für Live-Debugging
- **UART:** Fallback für Debug-Ausgaben
- **Flash-Logs:** Persistente Ereignis-Logs
- **Telemetrie:** Power-Consumption-Monitoring
