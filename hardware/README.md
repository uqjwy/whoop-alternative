# Hardware Design - Whoop Alternative

Hardware design files for the Whoop Alternative wearable device.

## Overview

### Target Specifications
- **Form Factor:** Bicep band (upper arm placement)
- **Battery Life:** 7 days continuous monitoring
- **Water Resistance:** IPX4 (splash-proof)
- **Size:** ~40mm x 30mm x 12mm main module
- **Weight:** <25g including band

### Key Components
- **MCU:** nRF52840 (ARM Cortex-M4F, BLE 5.0)
- **PPG Sensor:** MAX86141 (heart rate, SpO2)
- **IMU:** BMA400 (ultra-low power accelerometer)
- **Temperature:** MAX30205 (precision temperature sensor)
- **Flash Memory:** W25Q64 (8MB SPI NOR flash)
- **Power Management:** BQ24074 + TPS62740 + MAX17048
- **Battery:** 200-250mAh LiPo

## Directory Structure

```
hardware/
├── schematics/              # Circuit schematics
│   ├── main_board.sch      # Main PCB schematic
│   ├── power_mgmt.sch      # Power management circuit
│   ├── sensors.sch         # Sensor interfaces
│   └── ble_antenna.sch     # BLE antenna design
├── pcb/                    # PCB layout files
│   ├── main_board.pcb      # 4-layer PCB layout
│   ├── stackup.txt         # PCB stackup specification
│   ├── gerbers/           # Manufacturing files
│   └── assembly/          # Assembly drawings & BOM
├── mechanical/             # Mechanical design
│   ├── enclosure/         # Device enclosure
│   │   ├── case_top.step  # Upper case part
│   │   ├── case_bottom.step # Lower case part
│   │   └── gaskets.step   # Sealing gaskets
│   ├── band/              # Wrist/arm band
│   │   ├── band_main.step # Main band structure
│   │   ├── clasp.step     # Clasp mechanism
│   │   └── materials.txt  # Band material specifications
│   └── optics/            # PPG optical design
│       ├── light_pipe.step # LED light guide
│       ├── photodiode_housing.step
│       └── optical_isolation.step
├── simulation/             # Circuit simulations
│   ├── power_analysis/    # Power consumption analysis
│   ├── emi_emc/          # EMI/EMC simulation
│   └── thermal/          # Thermal analysis
└── testing/               # Test procedures
    ├── bringup/          # Board bring-up tests
    ├── validation/       # Design validation tests
    └── production/       # Production test procedures
```

## Circuit Design

### Power Architecture
```
USB-C Connector
     ↓
BQ24074 (Battery Charger/Power Path)
     ↓
3.7V LiPo Battery (200-250mAh)
     ↓
TPS62740 (3.0V Buck Converter)
     ↓
3.0V Rail → nRF52840, Sensors, Flash
     ↓
MAX17048 (Fuel Gauge) → I²C → nRF52840
```

### Sensor Interfaces
```
nRF52840 MCU
├── I²C0 (Primary Sensors)
│   ├── MAX86141 (PPG) - 0x5E
│   ├── BMA400 (IMU) - 0x14
│   └── MAX30205 (Temp) - 0x48
├── I²C1 (Power Management)
│   └── MAX17048 (Fuel Gauge) - 0x36
├── SPI0 (Storage)
│   └── W25Q64 (8MB Flash)
├── SAADC (Analog Inputs)
│   ├── Channel 0: Battery voltage divider
│   ├── Channel 1: EDA electrode 1 (optional)
│   └── Channel 2: EDA electrode 2 (optional)
└── GPIO
    ├── INT pins from sensors
    ├── Status LEDs
    └── Charging indicator
```

### PCB Specifications
- **Layers:** 4-layer stackup
  - Layer 1: Signal/Component
  - Layer 2: Ground plane
  - Layer 3: Power plane (3.0V)
  - Layer 4: Signal/Component
- **Dimensions:** 30mm x 25mm
- **Thickness:** 1.6mm
- **Via:** 0.2mm drill, 0.45mm annular ring
- **Trace Width:** 0.1mm minimum
- **Spacing:** 0.1mm minimum

### Power Budget Analysis
| Component | Active Current | Sleep Current | Duty Cycle | Avg Current |
|-----------|---------------|---------------|------------|-------------|
| nRF52840  | 8.0 mA        | 1.5 µA       | 10%        | 0.80 mA     |
| MAX86141  | 15.0 mA       | 0.1 µA       | 2%         | 0.30 mA     |
| BMA400    | 0.18 mA       | 0.1 µA       | 100%       | 0.18 mA     |
| MAX30205  | 0.6 mA        | 0.1 µA       | 1%         | 0.01 mA     |
| W25Q64    | 15.0 mA       | 1.0 µA       | 0.1%       | 0.02 mA     |
| TPS62740  | Quiescent: 60 µA                        | 0.06 mA     |
| **Total** |               |               |            | **1.37 mA** |

**Battery Life Calculation:**
- Battery Capacity: 220mAh (typical)
- Average Current: 1.37mA
- **Estimated Life: 6.7 days** (meets 7-day target with margin)

## Mechanical Design

### Enclosure Requirements
- **Material:** Medical-grade silicone outer, PC/ABS inner shell
- **Sealing:** IPX4 water resistance
- **Dimensions:** 40mm x 30mm x 12mm (max)
- **Weight:** <20g (without band)
- **Charging:** Pogo pin connector (magnetic alignment)

### Optical Design
The PPG sensor requires careful optical design for reliable readings:

```
Skin Surface
     ↑
Light Guide (clear silicone)
     ↑
Photodiode (ambient light blocking)
     ↑
PCB Opening
     ↑
MAX86141 Package
     ↑
LED Current Control Circuit
```

- **LED Wavelengths:** 660nm (Red), 880nm (IR)
- **Optical Isolation:** Dark barrier between LED and photodiode
- **Contact Pressure:** 15-25g force for good skin contact
- **Ambient Light:** <1% leakage into photodiode

### Band Design
- **Material:** Hypoallergenic silicone with texture for grip
- **Length:** Adjustable 22-35cm circumference
- **Width:** 25mm for stable sensor contact
- **Thickness:** 3mm for flexibility and comfort
- **Color Options:** Black, White, Gray

## Manufacturing Considerations

### PCB Fabrication
- **Recommended Suppliers:** JLCPCB, PCBWay, OSH Park
- **Specifications:** 4-layer, 1.6mm, HASL finish
- **Design Rules:** IPC-2221 Class 2
- **Testing:** In-circuit test points for critical signals

### Component Selection
- **Procurement:** All components available from major distributors
- **Lifecycle:** Minimum 10-year availability
- **Packaging:** Prefer 0402 and larger for hand assembly
- **Alternates:** Second sources identified for critical components

### Assembly Process
1. **SMT Assembly:** Automated pick-and-place for main components
2. **Reflow Soldering:** Lead-free SAC305 solder
3. **Hand Assembly:** Connectors, mechanical components
4. **Programming:** Flash bootloader and test firmware
5. **Testing:** Functional test of all subsystems
6. **Enclosure:** Seal assembly in waterproof housing

## Design Validation

### Electrical Testing
- **Power Consumption:** Verify power budget under all operating modes
- **Signal Integrity:** PPG signal quality, IMU accuracy
- **BLE Performance:** Range, power consumption, throughput
- **EMI/EMC:** FCC Part 15, CE compliance testing

### Mechanical Testing
- **Drop Test:** 1.5m drop onto concrete (device in band)
- **Water Resistance:** IPX4 spray test
- **Thermal Cycling:** -10°C to +60°C operation
- **Wear Testing:** 1000-hour accelerated wear simulation

### Biological Safety
- **Skin Compatibility:** ISO 10993-5 cytotoxicity testing
- **Allergenic Testing:** Patch testing for common allergens
- **Long-term Wear:** 30-day user comfort study

## Risk Mitigation

### Technical Risks
- **PPG Performance:** Multiple validation setups, algorithm tuning
- **Battery Life:** Conservative power budget, measurement validation
- **BLE Connectivity:** Antenna optimization, range testing
- **Mechanical Failure:** Stress analysis, material selection

### Regulatory Risks
- **FCC/CE Certification:** Early pre-compliance testing
- **Biocompatibility:** Use proven materials and suppliers
- **Privacy:** Hardware-level encryption, secure boot

### Supply Chain Risks
- **Component Availability:** Multi-sourced critical components
- **Cost Variations:** Locked pricing for initial production
- **Quality Control:** Incoming inspection procedures

## Cost Analysis (100 units)

| Category | Unit Cost | Notes |
|----------|-----------|-------|
| PCB | $15 | 4-layer, small qty |
| Components | $25 | nRF52840, sensors, passives |
| Battery | $5 | 220mAh LiPo with protection |
| Enclosure | $8 | Injection molded silicone |
| Band | $3 | Silicone with clasp |
| Assembly | $10 | SMT + final assembly |
| **Total** | **$66** | Per unit at 100 qty |

*Note: Costs will decrease significantly at higher volumes (1000+ units)*
