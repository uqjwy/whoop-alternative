# Whoop Alternative - Prototyp Schaltplan

## Hauptkomponenten Übersicht

### Mikrocontroller: nRF52840
**Package:** QFN-48 (7mm x 7mm)
**Funktion:** Hauptprozessor mit integriertem BLE 5.0

#### Wichtige Pins:
- **Power:** VDD (Pin 13, 24, 36, 47) = 3.0V
- **Ground:** VSS (Pin 12, 25, 35, 48)
- **Crystal:** XC1/XC2 (Pin 1, 2) für 32MHz
- **Low-freq Crystal:** XL1/XL2 (Pin 3, 4) für 32.768kHz
- **Reset:** RESET (Pin 18)
- **SWD Debug:** SWDIO (Pin 21), SWDCLK (Pin 20)

#### I²C0 (Primäre Sensoren):
- **SCL:** P0.27 (Pin 31)
- **SDA:** P0.26 (Pin 30)

#### SPI0 (Flash-Speicher):
- **SCK:** P0.19 (Pin 22)
- **MOSI:** P0.20 (Pin 23)
- **MISO:** P0.21 (Pin 24)
- **CS:** P0.22 (Pin 25)

#### GPIO (Interrupts & Status):
- **PPG_INT:** P0.02 (Pin 5)
- **IMU_INT1:** P0.03 (Pin 6)
- **IMU_INT2:** P0.04 (Pin 7)
- **TEMP_INT:** P0.05 (Pin 8)
- **CHARGE_STAT:** P0.06 (Pin 9)

---

## Sensor Subsystem

### 1. PPG Sensor: MAX86141
**Package:** OLGA-14 (3.3mm x 2.9mm)
**I²C Address:** 0x5E
**Funktion:** Herzfrequenz & SpO2 Messung

#### Pin-Verbindungen:
```
MAX86141                 nRF52840
─────────────────────────────────
VDD (Pin 1)       ─────→ 3.0V
VLED (Pin 2)      ─────→ 3.0V (über 10µH Ferrit)
GND (Pin 3, 14)   ─────→ GND
SCL (Pin 4)       ─────→ P0.27 (SCL)
SDA (Pin 5)       ─────→ P0.26 (SDA)
INT (Pin 6)       ─────→ P0.02 (PPG_INT)
LED1+ (Pin 7)     ─────→ Externe Rot-LED (660nm)
LED1- (Pin 8)     ─────→ GND
LED2+ (Pin 9)     ─────→ Externe IR-LED (880nm)
LED2- (Pin 10)    ─────→ GND
PD1 (Pin 11)      ─────→ Externe Photodiode (+)
PD2 (Pin 12)      ─────→ Externe Photodiode (-)
VREF (Pin 13)     ─────→ 1.8V Referenz (intern)
```

#### Externe Komponenten:
- **Rot-LED:** OSRAM SFH 4735 (660nm, 30mW)
- **IR-LED:** OSRAM SFH 4736 (880nm, 30mW)
- **Photodiode:** OSRAM SFH 2712FA (560nm-1000nm)

### 2. IMU Sensor: BMA400
**Package:** LGA-12 (2.0mm x 2.0mm)
**I²C Address:** 0x14
**Funktion:** 3-Achsen Beschleunigungsmesser

#### Pin-Verbindungen:
```
BMA400                   nRF52840
─────────────────────────────────
VDD (Pin 1)       ─────→ 3.0V
VDDIO (Pin 2)     ─────→ 3.0V
GND (Pin 3)       ─────→ GND
SCK/SCL (Pin 4)   ─────→ P0.27 (SCL)
SDI/SDA (Pin 5)   ─────→ P0.26 (SDA)
SDO/ADDR (Pin 6)  ─────→ GND (I²C Addr = 0x14)
CS (Pin 7)        ─────→ 3.0V (I²C Mode)
INT1 (Pin 8)      ─────→ P0.03 (IMU_INT1)
INT2 (Pin 9)      ─────→ P0.04 (IMU_INT2)
NC (Pin 10-12)    ─────→ Nicht verbunden
```

### 3. Temperatur Sensor: MAX30205
**Package:** µDFN-6 (1.5mm x 1.5mm)
**I²C Address:** 0x48
**Funktion:** Hochpräzise Hauttemperatur

#### Pin-Verbindungen:
```
MAX30205                 nRF52840
─────────────────────────────────
VDD (Pin 1)       ─────→ 3.0V
SCL (Pin 2)       ─────→ P0.27 (SCL)
SDA (Pin 3)       ─────→ P0.26 (SDA)
ADDR (Pin 4)      ─────→ GND (Addr = 0x48)
INT (Pin 5)       ─────→ P0.05 (TEMP_INT)
GND (Pin 6)       ─────→ GND
```

---

## Power Management System

### 1. Battery Charger: BQ24074
**Package:** VQFN-20 (4mm x 4mm)
**Funktion:** Li-Po Laderegler mit Power Path

#### Pin-Verbindungen:
```
BQ24074                  System
─────────────────────────────────
VCC (Pin 1)       ─────→ USB-C VBUS (5V)
BAT (Pin 2)       ─────→ Li-Po+ (3.7V nominell)
OUT (Pin 3)       ─────→ VSYS (3.7V)
CE (Pin 4)        ─────→ 3.0V (Charging Enable)
EN1 (Pin 5)       ─────→ GND
EN2 (Pin 6)       ─────→ 3.0V
ILIM (Pin 7)      ─────→ 1.5kΩ → GND (500mA)
ISET (Pin 8)      ─────→ 2.0kΩ → GND (300mA)
TS (Pin 9)        ─────→ 10kΩ NTC → GND
STAT (Pin 10)     ─────→ P0.06 (CHARGE_STAT)
VSS (Pin 11-15)   ─────→ GND
PGND (Pin 16-20)  ─────→ GND
```

### 2. Buck Converter: TPS62740
**Package:** SON-8 (2mm x 2mm)
**Funktion:** 3.7V → 3.0V Wandler

#### Pin-Verbindungen:
```
TPS62740                 System
─────────────────────────────────
VIN (Pin 1)       ─────→ VSYS (3.7V)
EN (Pin 2)        ─────→ VSYS (Always On)
FB (Pin 3)        ─────→ Feedback Network
VOUT (Pin 4)      ─────→ 3.0V Rail
GND (Pin 5)       ─────→ GND
SW (Pin 6)        ─────→ 2.2µH Inductor
PGND (Pin 7)      ─────→ GND
NC (Pin 8)        ─────→ Nicht verbunden
```

#### Feedback Network für 3.0V:
- **R1:** 470kΩ (VOUT → FB)
- **R2:** 330kΩ (FB → GND)

### 3. Fuel Gauge: MAX17048
**Package:** µDFN-8 (2mm x 2mm)
**I²C Address:** 0x36
**Funktion:** Batterie-Überwachung

#### Pin-Verbindungen:
```
MAX17048                 System
─────────────────────────────────
CELL (Pin 1)      ─────→ Li-Po+ (3.7V)
VDD (Pin 2)       ─────→ 3.0V
SCL (Pin 3)       ─────→ P0.27 (SCL)
SDA (Pin 4)       ─────→ P0.26 (SDA)
ALRT (Pin 5)      ─────→ P0.07 (BAT_ALERT)
QSTRT (Pin 6)     ─────→ GND (über 1nF)
GND (Pin 7)       ─────→ GND
CTG (Pin 8)       ─────→ GND
```

### 4. Flash Speicher: W25Q64JVSSIQ
**Package:** SOIC-8 (5.3mm x 5.3mm)
**Funktion:** 8MB SPI NOR Flash

#### Pin-Verbindungen:
```
W25Q64                   nRF52840
─────────────────────────────────
CS (Pin 1)        ─────→ P0.22 (CS)
DO (Pin 2)        ─────→ P0.21 (MISO)
WP (Pin 3)        ─────→ 3.0V
GND (Pin 4)       ─────→ GND
DI (Pin 5)        ─────→ P0.20 (MOSI)
CLK (Pin 6)       ─────→ P0.19 (SCK)
HOLD (Pin 7)      ─────→ 3.0V
VCC (Pin 8)       ─────→ 3.0V
```

---

## Passives Komponenten Schema

### Power Supply Kapazitäten:
```
3.0V Rail:
├── 10µF Tantalum (Low ESR)
├── 1µF Keramik (X7R)
└── 100nF Keramik (X7R, bei jedem IC)

3.7V Rail:
├── 22µF Tantalum
└── 1µF Keramik

USB Input:
├── 47µF Elektrolyt
└── 100nF Keramik
```

### Crystal Schaltung (nRF52840):
```
32MHz Crystal:
├── Crystal: 32MHz ±20ppm
├── C1: 8pF → GND (XC1)
└── C2: 8pF → GND (XC2)

32.768kHz Crystal:
├── Crystal: 32.768kHz ±20ppm
├── C3: 12pF → GND (XL1)
└── C4: 12pF → GND (XL2)
```

### Pull-up Widerstände:
```
I²C Bus:
├── R_SCL: 4.7kΩ → 3.0V
└── R_SDA: 4.7kΩ → 3.0V

Reset:
└── R_RESET: 10kΩ → 3.0V

SPI:
└── R_CS: 10kΩ → 3.0V
```

---

## PCB Layout Hinweise

### 4-Layer Stack-up:
1. **Layer 1:** Komponenten + Signale
2. **Layer 2:** Ground Plane
3. **Layer 3:** Power Plane (3.0V + 3.7V)
4. **Layer 4:** Signale + Komponenten

### Kritische Design-Aspekte:

#### 1. Analog-Digital Trennung:
- Separate AGND/DGND Bereiche
- Ferritperlen zwischen analog/digital Bereichen
- Sternpunkt-Erdung am nRF52840

#### 2. 2.4GHz Antenna Keepout:
- 5mm Keepout um Antenna-Bereich
- Keine Leiterbahnen unter Antenne
- Ground Plane unter Antenne

#### 3. PPG Sensor Optik:
- Lichtschutz zwischen LEDs und Photodiode
- Schwarzer Lötstoplack um optische Komponenten
- Mechanische Trennung für Ambient Light Rejection

#### 4. Power Integrity:
- Minimaler ESR bei Entkopplungs-Kondensatoren
- Via-Stitching zwischen Power Planes
- Thermal Vias bei Power ICs

---

## Mechanisches Design

### Abmessungen:
- **PCB:** 35mm x 25mm x 1.6mm (4-Layer)
- **Gehäuse:** 40mm x 30mm x 12mm
- **Gewicht:** <25g (mit 200mAh Li-Po)

### Ladekontakte:
```
Pogo Pins (4x):
├── Pin 1: GND
├── Pin 2: VBUS (5V)
├── Pin 3: USB D+
└── Pin 4: USB D-

Magnet-Ausrichtung:
├── 2x Neodym Magnete (3mm Ø)
└── Magnetische Ladedock
```

### Wasserdichtigkeit (IPX4):
- O-Ring Dichtung um PCB
- Wasserdichte Pogo-Pin Verbindung
- Medizinisches Silikon-Gehäuse

---

## Power Budget (Prototyp):

| Komponente | Aktiv | Sleep | Duty Cycle | Durchschnitt |
|------------|-------|-------|------------|--------------|
| nRF52840   | 8.0mA | 1.5µA | 10%        | 0.80mA       |
| MAX86141   | 15mA  | 0.1µA | 2%         | 0.30mA       |
| BMA400     | 0.18mA| 0.1µA | 100%       | 0.18mA       |
| MAX30205   | 0.6mA | 0.1µA | 1%         | 0.01mA       |
| W25Q64     | 15mA  | 1µA   | 0.1%       | 0.02mA       |
| MAX17048   | 40µA  | 3µA   | 100%       | 0.04mA       |
| TPS62740   | -     | -     | -          | 0.02mA       |
| **Total**  | -     | -     | -          | **1.37mA**   |

**Batterie-Laufzeit:** 200mAh / 1.37mA = **146 Stunden ≈ 6.1 Tage**

---

## Testpoints & Debug:

### SWD Interface:
- **SWDIO:** P0.21 (Testpoint)
- **SWDCLK:** P0.20 (Testpoint)
- **RESET:** Testpoint
- **GND:** Testpoint

### Power Monitoring:
- **3.0V:** Testpoint
- **3.7V:** Testpoint
- **VSYS:** Testpoint
- **I_BATT:** Shunt-Widerstand für Strommessung

### Sensor Status:
- **LED_STATUS:** P0.13 (RGB LED)
- **CHARGE_LED:** P0.14 (Status LED)

---

## Stückliste (BOM) Hauptkomponenten:

| Komponente | Teil-Nr | Hersteller | Stückpreis |
|------------|---------|------------|------------|
| nRF52840   | nRF52840-QIAA | Nordic | $6.50 |
| MAX86141   | MAX86141ENI+ | Maxim | $12.80 |
| BMA400     | BMA400 | Bosch | $3.20 |
| MAX30205   | MAX30205MTA+ | Maxim | $2.10 |
| BQ24074    | BQ24074RGTR | TI | $1.85 |
| TPS62740   | TPS62740DSST | TI | $1.60 |
| MAX17048   | MAX17048G+ | Maxim | $1.90 |
| W25Q64     | W25Q64JVSSIQ | Winbond | $0.85 |
| **Total ICs** | | | **$30.80** |

**Gesamt-BOM (inkl. Passives):** ~$45-50 (Prototyp)
**Produktions-BOM (1k+ Stück):** ~$28-32

---

*Dieser Schaltplan dient als Grundlage für die Prototyp-Entwicklung des Whoop-Alternative Fitness-Trackers. Alle Spannungs- und Stromwerte sind für kontinuierliche 7-Tage-Überwachung optimiert.*
