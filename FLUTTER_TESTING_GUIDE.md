# Flutter Setup und App Testing Guide

## 📱 Flutter Installation auf macOS

### 1. Flutter SDK installieren
```bash
# Option A: Mit Homebrew (empfohlen)
brew install flutter

# Option B: Manuell herunterladen
# Gehe zu: https://docs.flutter.dev/get-started/install/macos
# Lade Flutter SDK herunter und entpacke es
```

### 2. Flutter PATH hinzufügen
```bash
# Füge zu deiner ~/.zshrc hinzu:
echo 'export PATH="$PATH:/path/to/flutter/bin"' >> ~/.zshrc
source ~/.zshrc
```

### 3. Flutter Setup überprüfen
```bash
flutter doctor
```

### 4. iOS Simulator vorbereiten (für iOS Testing)
```bash
# Xcode installieren (falls noch nicht vorhanden)
xcode-select --install

# iOS Simulator öffnen
open -a Simulator
```

## 🧪 App Testing Optionen

### Option 1: iOS Simulator (empfohlen für Quick Testing)
```bash
cd app/mobile
flutter run
```

### Option 2: Chrome/Web Browser
```bash
cd app/mobile
flutter run -d chrome
```

### Option 3: Physisches iOS Gerät
```bash
# iPhone via USB verbinden
flutter run
```

### Option 4: Mock-Modus (ohne echte Hardware)
Die App enthält bereits Mock-Daten und Simulationen, sodass du alle Features ohne echte Wearable-Hardware testen kannst.

## 🔧 Erweiterte Test-Features

### BLE Testing ohne Hardware
- Die App enthält einen BLE-Simulator
- Simuliert alle Sensordaten (Herzfrequenz, HRV, Temperatur)
- Zeigt realistische Gesundheitsdaten an

### Was du testen kannst:
1. **Device Connection Simulation**
2. **Real-time Heart Rate Charts**
3. **HRV Analysis Dashboard**
4. **Health Alerts System**
5. **Sleep Tracking UI**
6. **Activity Monitoring**
7. **Recovery Scoring**

## 🐛 Debugging und Development

### Flutter Development Tools
```bash
# Hot Reload während der Entwicklung
# Drücke 'r' im Terminal für Hot Reload
# Drücke 'R' für Hot Restart

# Flutter Inspector
flutter inspector
```

### Logs und Debugging
```bash
# App Logs anzeigen
flutter logs

# Performance Monitoring
flutter run --profile
```

## 📊 Test-Daten

Die App enthält bereits:
- Simulierte PPG-Daten
- Mock Schlaf-Scores
- Beispiel Aktivitätsdaten
- Health Alert Simulationen
