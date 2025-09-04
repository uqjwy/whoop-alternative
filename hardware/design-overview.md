# Hardware Design - Whoop Alternative

Complete hardware design files and documentation for the Whoop Alternative wearable health monitor.

## Project Overview

The hardware is designed as a compact, power-efficient wearable device optimized for continuous health monitoring with 7-day battery life.

### Key Specifications
- **Form Factor:** Bicep band (upper arm placement)
- **Battery Life:** 7 days continuous monitoring
- **Water Resistance:** IPX4 (splash resistant)
- **Sensors:** PPG, IMU, Temperature
- **Connectivity:** Bluetooth Low Energy 5.0
- **Charging:** Pogo pin charging dock

## Component Selection & Power Budget

### Main Components

| Component | Part Number | Function | Avg Current |
|-----------|-------------|----------|-------------|
| **MCU** | nRF52840 | Main processor + BLE | 0.80 mA |
| **PPG Sensor** | MAX86141 | Heart rate monitoring | 0.30 mA |
| **IMU** | BMA400 | Motion detection | 0.18 mA |
| **Temperature** | MAX30205 | Skin temperature | 0.003 mA |
| **Flash** | W25Q64JVSSIQ | Data storage | 0.015 mA |
| **Fuel Gauge** | MAX17048 | Battery monitoring | 0.04 mA |
| **Peripherals** | Various | Support circuits | 0.5 mA |
| **Total** | | | **1.37 mA** |

**Battery Life:** 250mAh / 1.37mA = **182 hours = 7.6 days**

## PCB Design (4-Layer)

### Stack-up
```
Layer 1: Component/Signal   (35µm copper)
Layer 2: Ground Plane      (35µm copper)  
Layer 3: Power Plane       (35µm copper)
Layer 4: Signal/Components (35µm copper)
```

### Critical Design Considerations
- Separate analog/digital ground planes
- 2.4GHz antenna keepout zones
- Thermal management for components
- Power supply decoupling and filtering

## Mechanical Design

### Enclosure
- **Material:** Medical-grade silicone
- **Dimensions:** ~40mm x 25mm x 12mm
- **Weight:** <25g including battery
- **Sealing:** IPX4 water resistance

### Charging System
- **Interface:** 4-pin pogo connector
- **Dock:** Magnetic alignment
- **Input:** USB-C charging

## Cost Analysis (Prototype)

| Component Category | Cost/Unit |
|-------------------|-----------|
| Electronic components | $61 |
| PCB fabrication | $15 |
| Mechanical parts | $18 |
| Assembly & test | $35 |
| **Total (Qty 100)** | **$129** |

*Volume production (1000+): ~$65/unit*

## Manufacturing & Testing

### Validation Requirements
1. **Electrical:** Power, sensors, BLE communication
2. **Performance:** PPG signal quality, battery life
3. **Environmental:** Temperature, humidity, water resistance
4. **Regulatory:** FCC Part 15, CE marking, RoHS

### Tools Required
- **KiCad 6.0+** for PCB design
- **FreeCAD** for mechanical design
- **Test equipment** for validation

---

**Medical Disclaimer:** This hardware is designed for wellness/fitness monitoring only and is not intended as a medical device.
