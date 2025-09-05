# PCB Layout Guide - Whoop Alternative

## PCB Spezifikationen

### Grundlegende Parameter
- **Board Size:** 35mm x 25mm (Kompakter Fitness-Tracker)
- **Layer Count:** 4 Layers
- **Thickness:** 1.6mm
- **Via Size:** 0.2mm drill, 0.4mm pad
- **Minimum Trace:** 0.1mm (4 mil)
- **Minimum Spacing:** 0.1mm (4 mil)

### Layer Stack-up (von oben nach unten)
```
Layer 1 (Top):      Komponenten + Signal (35µm Cu)
     FR4 Prepreg:   0.2mm
Layer 2 (GND):      Ground Plane (35µm Cu)
     FR4 Core:      1.2mm  
Layer 3 (PWR):      Power Planes (35µm Cu)
     FR4 Prepreg:   0.2mm
Layer 4 (Bottom):   Signal + Komponenten (35µm Cu)
```

## Komponentenplatzierung

### Top Layer (Layer 1)
```
Hauptkomponenten Anordnung:

    ┌─────────────── 35mm ──────────────┐
    │  [Battery]              [Antenna] │ 25mm
    │                                   │
    │  [nRF52840]    [Sensors Cluster]  │
    │                                   │
    │  [Power ICs]   [Flash Memory]     │
    └───────────────────────────────────┘

Detaillierte Platzierung:
┌─────────────────────────────────────────┐
│ BAT+  MAX17048  │  │  Antenna Keepout  │
│                 │  │     (5mm zone)    │
│ BQ24074  TPS62740  │                   │
├─────────────────────┼───────────────────┤
│                     │  MAX86141         │
│    nRF52840         │  ┌─LED1─LED2┐     │
│                     │  └─Photodiode─┘   │
│  32MHz   32kHz      │                   │
├─────────────────────┼───────────────────┤
│ USB-C               │  BMA400           │
│ Connector           │  MAX30205         │
│                     │  W25Q64           │
└─────────────────────┴───────────────────┘
```

### Bottom Layer (Layer 4)
```
Passive Komponenten und Routing:
- Entkopplungs-Kondensatoren unter ICs
- Crystal Load Capacitors
- Pull-up Widerstände für I²C
- Test Points für Debug
- Charge/Status LEDs
```

## Kritische Routing Guidelines

### Power Integrity

#### 3.0V Rail (Layer 3)
```
TPS62740 → Main Power Plane
├── 10µF Tantalum (Low ESR) ← Eingangs-Kondensator
├── 1µF Ceramic (X7R) ← Ausgangs-Kondensator  
├── 2.2µH Inductor ← Switching Element
│
├── nRF52840 VDD Pins (4x 100nF lokal)
├── MAX86141 VDD + VLED
├── BMA400 VDD + VDDIO
├── MAX30205 VDD
├── W25Q64 VCC
└── MAX17048 VDD
```

#### Ground Plane (Layer 2)
```
Unified Ground Strategy:
- Kontinuierliche Ground Plane
- Via Stitching (alle 5mm)
- Separate Analog/Digital Bereiche
- Thermal Vias unter Power ICs
```

### High-Speed Signale

#### 32MHz Crystal (nRF52840)
```
Routing Regeln:
- Trace Länge: <5mm total
- Matched Length ±0.1mm
- 50Ω controlled impedance
- Ground Guard Traces
- Crystal Guard Ring (GND)

XC1 ──┬── 8pF ── GND
      └── Crystal
XC2 ──┬── 8pF ── GND
      └── Crystal
```

#### SPI Bus (Flash Memory)
```
Routing Regeln:
- Trace Länge: <10mm
- 50Ω ±10% impedance
- Length Matching ±0.5mm
- Minimale Via Count

SCK  ───────────► W25Q64
MOSI ───────────► W25Q64  
MISO ◄─────────── W25Q64
CS   ───────────► W25Q64
```

### Sensor Interfaces

#### I²C Bus
```
Routing Strategie:
- SCL/SDA parallel routing
- 4.7kΩ Pull-ups nahe MCU
- Minimale Trace Länge
- Vermeidung von Switching Nodes

nRF52840 ──┬── 4.7kΩ ── 3.0V (SCL)
           │
           ├── MAX86141 (0x5E)
           ├── BMA400   (0x14)  
           ├── MAX30205 (0x48)
           └── MAX17048 (0x36)
```

#### PPG Optical Interface
```
Kritische Anforderungen:
- LED/Photodiode Separation (>2mm)
- Dark Current Minimierung
- Electromagnetic Isolation
- Thermal Management

Layout:
LED1 ──┬── 0.1µF ── GND (Entkopplung)
       └── MAX86141 LED1+

LED2 ──┬── 0.1µF ── GND (Entkopplung)  
       └── MAX86141 LED2+

Photodiode:
PD+ ────► MAX86141 PD1 (Kurze Traces)
PD- ────► MAX86141 PD2 (Matched Length)
```

## EMI/EMC Considerations

### BLE Antenna Platzierung
```
Antenna Keepout Zone:
┌─────────────────┐
│   Antenna PCB   │ ← 2.4GHz Ceramic
├─────────────────┤
│                 │
│   5mm Keepout   │ ← Keine Komponenten
│                 │
└─────────────────┘

Requirements:
- Ground Plane unter Antenne
- Keine Switching Circuits in der Nähe  
- Isolated Power für RF Section
```

### Switching Converter Isolation
```
TPS62740 Layout:
    VIN ──┬── INPUT_CAP
          │
    ┌─────▼──────┐
    │  TPS62740  │
    │            │ ← Thermal Via Array
    └─────┬──────┘
          │
     SW ──┴── 2.2µH ── VOUT
              │
              └── OUTPUT_CAP

Critical:
- Short SW trace (<5mm)
- High current paths (wide traces)
- Kelvin sensing für FB
```

## Thermal Management

### Power Dissipation Map
```
Heat Sources:
- TPS62740: ~50mW (Efficiency Loss)
- BQ24074: ~150mW (Charging)
- nRF52840: ~24mW (TX Mode)
- MAX86141: ~30mW (LED Drive)

Thermal Solutions:
┌─────────────┐
│  Component  │
│             │ ← Thermal Pad
├─────────────┤
│ Thermal Via │ ← 0.2mm, filled
│   Array     │   (9 vias minimum)
├─────────────┤
│ Ground/PWR  │ ← Heat spreading
│   Plane     │
└─────────────┘
```

### Via Stitching Pattern
```
Power/Ground Via Stitching:
○ ○ ○ ○ ○ ○ ○    ← Every 5mm
○ ○ ○ ○ ○ ○ ○
○ ○ ○ ○ ○ ○ ○

Critical Areas:
- Around switching converters
- Under main MCU
- Between analog/digital sections
```

## Manufacturing Constraints

### Minimum Design Rules
```
Feature               Minimum    Preferred
─────────────────────────────────────────
Trace Width          0.1mm      0.15mm
Trace Spacing        0.1mm      0.15mm
Via Drill            0.15mm     0.2mm
Via Pad              0.3mm      0.4mm
Annular Ring         0.05mm     0.075mm
Solder Mask          0.075mm    0.1mm
Silkscreen Width     0.1mm      0.15mm
```

### Assembly Considerations
```
Component Orientation:
- All polarized components same direction
- Critical components accessible for rework
- Test points on component side
- Fiducial markers (3x minimum)

Solder Paste:
- Type 4 paste for fine pitch
- Stencil thickness: 0.12mm
- Aperture ratio: 0.66 minimum
```

## Test and Debug Features

### Test Point Layout
```
Debug Interface:
TP1 ── SWDIO   (1mm pad)
TP2 ── SWDCLK  (1mm pad)
TP3 ── RESET   (1mm pad)
TP4 ── GND     (1mm pad)

Power Monitoring:
TP5 ── 3.0V    (Current Measurement)
TP6 ── 3.7V    (Battery Monitoring)
TP7 ── VSYS    (System Rail)
TP8 ── CHARGE_STAT (Charge Status)

Sensor Monitoring:
TP9  ── PPG_INT
TP10 ── IMU_INT1
TP11 ── IMU_INT2
TP12 ── I2C_SCL
TP13 ── I2C_SDA
```

### Status LED Placement
```
LED Indicator:
RGB_LED ── P0.13 (Status)
CHG_LED ── P0.14 (Charging)

Position: Sichtbar durch Gehäuse
Current: 2mA maximum (Battery saving)
Resistor: 1.5kΩ für 2mA bei 3.0V
```

## Assembly Sequence

### Bottom-Up Assembly
```
1. Solder Paste Application (Stencil)
2. Component Placement:
   - Passives (Bottom Layer)
   - ICs (Bottom Layer)
3. Reflow Soldering (Bottom)
4. Flip Board
5. Solder Paste Application (Top)
6. Component Placement:
   - Passives (Top Layer)
   - Main ICs (Top Layer)
   - Connectors
7. Reflow Soldering (Top)
8. Manual Soldering:
   - Through-hole components
   - Connectors
   - Test points
```

### Quality Control
```
Inspection Points:
1. Pre-reflow: Paste inspection (SPI)
2. Post-reflow: AOI (Automated Optical)
3. Electrical: In-circuit test (ICT)
4. Functional: Boundary scan
5. Final: End-of-line test

Critical Tests:
- Power sequencing
- I²C communication
- SPI flash access
- BLE functionality
- Sensor readings
- Charging circuit
```

## Design for Manufacturing (DFM)

### Panel Layout (Produktions-Optimierung)
```
Production Panel: 50mm x 80mm

┌─────────────────────────────────┐
│  Board1    │    Board2          │
│            │                    │
├────────────┼────────────────────┤
│            │                    │
│  Board3    │    Board4          │
└─────────────────────────────────┘

Features:
- V-scoring für break-away
- Tooling holes
- Fiducial markers (panel level)
- Test coupons
```

### Cost Optimization
```
Cost Factors:
1. Layer Count: 4L optimal für mixed-signal
2. Via Types: Through-hole only (no blind/buried)
3. Material: Standard FR4 (Tg=170°C)
4. Surface Finish: HASL (cost-effective)
5. Solder Mask: Green standard
6. Silkscreen: White standard

Advanced Options (higher cost):
- ENIG für gold contacts
- Controlled impedance
- Via-in-pad with filling
- HDI technology (µvias)
```

Diese Anleitung bietet alle notwendigen Informationen für die erfolgreiche PCB-Entwicklung des Whoop-Alternative Prototypen, von der Komponentenplatzierung bis zur Serienfertigung.
