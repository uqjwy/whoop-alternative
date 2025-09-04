# Whoop Alternative - Wearable Health Monitor

Ein Open-Source Fitness-Tracker mit Fokus auf Herzfrequenzvariabilität (HRV), Schlafanalyse und Krankheitsfrüherkennung.

## 🎯 Ziele & Features

- **Formfaktor:** Bizeps-Band (Oberarm), spritzwassergeschützt
- **Akkulaufzeit:** 7 Tage (200-250 mAh LiPo)
- **Kernfeatures:**
  - Live Herzfrequenz (HR)
  - Herzfrequenzvariabilität (HRV) nachts
  - Schlafanalyse (Dauer/Unruhe)
  - Belastungsmonitoring (HR-basiert)
  - Health-Monitor mit "Krank werden"-Trends
  - Mobile App mit BLE-Synchronisation

## 🔧 Hardware-Architektur

### Hauptkomponenten
- **MCU/BLE:** nRF52840-Modul (Raytac MDBT50Q)
- **PPG-AFE:** MAX86141 + OSRAM SFH7050 LED/PD
- **IMU:** BMA400 (Low-Power)
- **Temperatur:** MAX30205 (Hauttemperatur-Trends)
- **Power:** BQ24074 (Lader), TPS62740 (Buck 3.0V), MAX17048 (Fuel Gauge)
- **Speicher:** SPI-NOR Flash 8MB (W25Q64)
- **(Optional) EDA:** 2 Elektroden + INA333-Frontend

### Bus-Architektur
- **I²C0:** PPG, IMU, Temperatur
- **I²C1:** Fuel Gauge
- **SPI0:** Flash-Speicher
- **SAADC:** EDA-Signal, Akku-Teiler

## 🏗️ Projektstruktur

```
firmware/           # Zephyr RTOS Firmware
├── app/           # Hauptanwendung
├── drivers/       # Hardware-Treiber (PPG, IMU, Temp, Flash)
├── modules/       # Signalverarbeitung & Algorithmen
├── ble/          # Bluetooth Low Energy Services
├── storage/      # Flash-Speicher Management
├── power/        # Energiemanagement
└── tests/        # Unit & Hardware-Tests

app/               # Mobile App & Analyse-Tools
├── mobile/       # Flutter Mobile App
└── notebooks/    # Python-Analyse & Validierung

hardware/          # Hardware-Design
├── schematics/   # PCB-Schaltpläne
├── pcb/         # PCB-Layout
└── mech/        # Mechanik (Gehäuse, Armband)

docs/             # Dokumentation
├── testplans/   # Testpläne & Validierung
├── risks/       # Risikoanalyse
└── user-guide/ # Benutzerhandbuch
```

## 🚀 Quick Start

### Phase 1: Evaluation (2-4 Wochen)
1. **Hardware:** nRF52840-DK + Breakout-Boards bestellen
2. **Erste Tests:** I²C/SPI bring-up, PPG-Rohdaten
3. **BLE Demo:** Heart Rate Service an Handy
4. **Validierung:** HR-Genauigkeit am Bizeps

### Phase 2: Firmware MVP (3-6 Wochen)
1. **Algorithmen:** PPG-Filterung, HRV-Berechnung
2. **Logging:** 24h Datenaufzeichnung
3. **Health-Engine:** Baseline-Tracking & Anomalie-Erkennung

### Phase 3: Hardware v1 (4-6 Wochen)
1. **PCB Design:** 4-Layer Board mit Analog-Isolation
2. **Mechanik:** Gehäuse + Armband-Prototyp
3. **Validation:** 7-Tage Akkutest

### Phase 4: Mobile App (2-4 Wochen)
1. **Core Features:** Pairing, Live-HR, HRV-Trends
2. **Data Sync:** BLE-Übertragung & Chart-Visualisierung

## 📋 Entwicklungsumgebung

### Voraussetzungen
- **Firmware:** nRF Connect SDK (Zephyr RTOS)
- **Mobile:** Flutter/React Native
- **Hardware:** KiCad/Altium Designer
- **Analyse:** Python + Jupyter Notebooks

### Tools & Dependencies
```bash
# Firmware (nRF Connect SDK)
west init -m https://github.com/nrfconnect/sdk-nrf --mr main
west update

# Mobile App (Flutter)
flutter create mobile_app
cd mobile_app && flutter pub get

# Python Analyse
pip install numpy pandas scipy matplotlib seaborn jupyter
```

## ⚡ Akzeptanzkriterien

### Signal-Qualität
- **HR-Genauigkeit:** RMSE ≤ 5 bpm vs. Brustgurt
- **HRV nachts:** ≥ 80% gültige Nächte (Artefakte < 20%)
- **Schlafanalyse:** Dauer ± 20 min vs. Schlaftagebuch

### Power-Management
- **Zielvorgabe:** ≤ 1.2 mA Durchschnittsstrom (7 Tage Laufzeit)
- **BLE-Sync:** < 5 min für 8MB Datenübertragung

### Health-Monitoring
- **"Krank werden"-Algorithmus:** < 1 Fehlalarm/Monat
- **Baseline:** 30-Tage-Einlernzeit vor Aktivierung

## 🛡️ Risiken & Compliance

- **Regulatorik:** Kein Medizinprodukt - nur Wellness/Fitness
- **Datenschutz:** DSGVO-konforme lokale Verarbeitung
- **Haftung:** Disclaimer für alle Gesundheitsdaten
- **Mechanik:** IPX4 Spritzwasserschutz

## 💰 Budget (MVP-Prototyp)

- **Entwicklungsboards:** ~80-180 €
- **Elektronik + Akku:** ~60-100 €
- **Mechanik/Armband:** ~20-40 €
- **Gesamt:** ~160-320 €

## 📞 Support & Contribution

Dieses Projekt ist in aktiver Entwicklung. Bei Fragen oder Interesse an Mitarbeit bitte Issue erstellen.

## 📄 Lizenz

MIT License - siehe [LICENSE](LICENSE) für Details.

---

**⚠️ Wichtiger Hinweis:** Dieses Gerät ist kein Medizinprodukt und dient ausschließlich Fitness- und Wellness-Zwecken.
