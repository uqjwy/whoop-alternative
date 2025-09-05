# Whoop Alternative - Schaltplan Blockdiagramm

```
                    WHOOP ALTERNATIVE PROTOTYP - HAUPTSCHALTPLAN
                    =============================================

USB-C           ┌──────────────┐    ┌────────────┐    ┌──────────────┐
Connector   ────┤   BQ24074    ├────┤ LiPo 200mAh├────┤   TPS62740   ├──── 3.0V Rail
(5V)            │  Charger     │    │   3.7V      │    │Buck Converter│
                └──────┬───────┘    └────────┬───┘    └──────────────┘
                       │                     │                │
                    CHARGE_STAT              │             ┌──┴── MAX17048
                       │                     │             │   Fuel Gauge
                       │                  ┌──┴──┐          │     │
                       │                  │3.7V │          │     │ I²C
                       │                  │Rail │          │     │
                       │                  └─────┘          │     │
                       │                                   │     │
              ┌────────┴─────────────────────────────────────────┴─────┐
              │                                                        │
              │                     nRF52840                           │
              │                 Main Controller                        │
              │                                                        │
              │  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐    │
              │  │  P0.27  │  │  P0.26  │  │  P0.19  │  │  P0.20  │    │
              │  │   SCL   │  │   SDA   │  │   SCK   │  │  MOSI   │    │
              │  └─────┬───┘  └─────┬───┘  └─────┬───┘  └─────┬───┘    │
              │        │            │            │            │        │
              │  ┌─────┴───┐  ┌─────┴───┐  ┌─────┴────────────┴───┐    │
              │  │  P0.21  │  │  P0.22  │  │      GPIO Pins       │    │
              │  │  MISO   │  │   CS    │  │  P0.02 - P0.07      │    │
              │  └─────┬───┘  └─────┬───┘  └──────────┬──────────┘    │
              └────────┼────────────┼─────────────────┼───────────────┘
                       │            │                 │
                  ┌────┴────┐  ┌────┴─────────────────┴─────┐
                  │         │  │                            │
              ┌───┴───┐ ┌───┴──┴───┐                   ┌────┴────┐
              │ SPI   │ │   I²C0   │                   │  GPIO   │
              │ Bus   │ │   Bus    │                   │ Signals │
              └───┬───┘ └─────┬────┘                   └────┬────┘
                  │           │                             │
              ┌───┴───┐   ┌───┴─────────────────────────────┴─────┐
              │W25Q64 │   │           Sensor Cluster              │
              │8MB    │   │                                       │
              │Flash  │   │ ┌─────────┐ ┌─────────┐ ┌─────────┐   │
              └───────┘   │ │MAX86141 │ │ BMA400  │ │MAX30205 │   │
                          │ │PPG AFE  │ │3-Axis   │ │Temp     │   │
                          │ │@0x5E    │ │IMU@0x14 │ │@0x48    │   │
                          │ └─────────┘ └─────────┘ └─────────┘   │
                          └───────────────────────────────────────┘

                         ┌─── PPG Optical System ───┐
                         │                          │
                    ┌────┴───┐  ┌─────────┐  ┌─────┴────┐
                    │Red LED │  │IR LED   │  │Photodiode│
                    │660nm   │  │880nm    │  │Si PIN    │
                    │30mW    │  │30mW     │  │400-1000nm│
                    └────────┘  └─────────┘  └──────────┘

POWER RAILS:
============
USB 5V ────────► BQ24074 ────► VSYS (3.7V Battery) ────► TPS62740 ────► 3.0V
                     │                                                    │
                     └── CHARGE_STAT                                      ├─► nRF52840
                                                                          ├─► MAX86141
                                                                          ├─► BMA400
                                                                          ├─► MAX30205
                                                                          ├─► W25Q64
                                                                          └─► MAX17048

I²C BUS TOPOLOGY:
================
        SCL (P0.27) ────┬──── 4.7kΩ ──── 3.0V
                         │
                         ├──── MAX86141 (0x5E)
                         ├──── BMA400   (0x14)
                         ├──── MAX30205 (0x48)
                         └──── MAX17048 (0x36)

        SDA (P0.26) ────┬──── 4.7kΩ ──── 3.0V
                         │
                         └──── (All I²C devices)

SPI BUS:
========
        SCK  (P0.19) ────► W25Q64 Flash
        MOSI (P0.20) ────► W25Q64 Flash
        MISO (P0.21) ────► W25Q64 Flash
        CS   (P0.22) ────► W25Q64 Flash

INTERRUPT SYSTEM:
================
        P0.02 (PPG_INT)   ◄──── MAX86141 INT Pin
        P0.03 (IMU_INT1)  ◄──── BMA400 INT1 Pin
        P0.04 (IMU_INT2)  ◄──── BMA400 INT2 Pin
        P0.05 (TEMP_INT)  ◄──── MAX30205 INT Pin
        P0.06 (CHARGE_STAT) ◄── BQ24074 STAT Pin
        P0.07 (BAT_ALERT) ◄──── MAX17048 ALRT Pin

DEBUG & PROGRAMMING:
===================
        P0.20 (SWDCLK) ──── SWD Debug Port
        P0.21 (SWDIO)  ──── SWD Debug Port
        RESET Pin      ──── External Reset

BLE ANTENNA:
===========
        Antenna ──── nRF52840 (2.4GHz Ceramic Antenna)
                     │
                     └── 5mm Keepout Zone

CRYSTAL OSCILLATORS:
===================
        32MHz ──── nRF52840 Main Clock
                   (8pF load caps)
        
        32.768kHz ─ nRF52840 RTC Clock
                   (12pF load caps)
```

## Stromversorgung im Detail

```
POWER MANAGEMENT TREE:
=====================

USB-C (5V, 500mA max)
  │
  ├─► BQ24074 Battery Charger
  │   ├─► Charge Current: 300mA (2kΩ ISET)
  │   ├─► Input Limit: 500mA (1.5kΩ ILIM)
  │   ├─► Thermal Protection: 10kΩ NTC
  │   └─► Status Output ──► nRF52840 P0.06
  │
  └─► VSYS Rail (3.3V - 4.2V)
      │
      ├─► LiPo Battery (200mAh, 3.7V nominal)
      │   │
      │   └─► MAX17048 Fuel Gauge
      │       ├─► I²C Address: 0x36
      │       ├─► Alert Output ──► nRF52840 P0.07
      │       └─► 1% State-of-Charge Accuracy
      │
      └─► TPS62740 Buck Converter
          ├─► Input: 2.5V - 5.5V
          ├─► Output: 3.0V ±2%
          ├─► Efficiency: >90%
          ├─► Quiescent: 20µA
          │
          └─► 3.0V System Rail
              ├─► nRF52840 (0.8mA avg)
              ├─► MAX86141 (0.3mA avg)
              ├─► BMA400 (0.18mA avg)
              ├─► MAX30205 (0.01mA avg)
              ├─► W25Q64 (0.02mA avg)
              └─► MAX17048 (0.04mA avg)
              
              Total: 1.37mA average
              Battery Life: 200mAh / 1.37mA = 146 hours
```

## Sensor Interface Details

```
PPG SENSOR INTERFACE (MAX86141):
===============================

External LEDs:
┌─────────────┐     ┌─────────────┐
│   Red LED   │     │   IR LED    │
│  OSRAM SFH  │     │ OSRAM SFH   │
│   4735      │     │   4736      │
│   660nm     │     │   880nm     │
│   30mW      │     │   30mW      │
└──────┬──────┘     └──────┬──────┘
       │                   │
       ├─► LED1+ ──────────┼─► LED2+
       │                   │
    MAX86141 AFE            │
       │                   │
       ├─► LED1- ──────────┼─► LED2-
       │                   │
       │                   │
    ┌──┴─────────────────────┴──┐
    │      Photodiode           │
    │    OSRAM SFH 2712FA       │
    │     560nm-1000nm          │
    │    Reverse Bias 5V        │
    └───────────┬───────────────┘
                │
                ├─► PD1+ (Anode)
                └─► PD2- (Cathode)

Signal Processing Chain:
LED Driver → Tissue → Photodiode → TIA → ADC → Digital Filter

IMU SENSOR INTERFACE (BMA400):
=============================

3-Axis Accelerometer:
┌─────────────┐
│   X-Axis    │ ──► 12-bit ADC
│   Y-Axis    │ ──► 12-bit ADC  
│   Z-Axis    │ ──► 12-bit ADC
└─────────────┘
       │
    ┌──┴───┐
    │FIFO  │ ──► INT1 (Motion Detection)
    │Buffer│ ──► INT2 (Step Counter)
    └──────┘

Features:
- Range: ±2g, ±4g, ±8g, ±16g
- Resolution: 12-bit
- ODR: 0.78Hz - 800Hz
- Power: 14µA @ 25Hz
- FIFO: 1024 bytes

TEMPERATURE SENSOR (MAX30205):
=============================

Precision Temperature Monitoring:
┌─────────────┐
│ Digital     │ ──► 16-bit Resolution
│ Temperature │ ──► ±0.1°C Accuracy
│ Sensor      │ ──► INT Pin (Threshold)
└─────────────┘

Features:
- Range: +28°C to +39°C (Human Body)
- Resolution: 0.00390625°C (16-bit)
- Conversion Time: 8ms
- Power: 600µA active, 0.1µA standby
```

## Lade- und Verbindungssystem

```
CHARGING DOCK INTERFACE:
=======================

Device Side (4 Pogo Pins):
┌─────────────────────────┐
│  ○     ○     ○     ○   │ ← Pogo Pins
│ GND   5V    D+    D-   │
└─────────────────────────┘

Dock Side (Magnetic Alignment):
┌─────────────────────────┐
│  ●     ●     ●     ●   │ ← Spring Pins
│ GND   5V    D+    D-   │
└─────────────────────────┘
    │     │     │     │
    │     │     │     └── USB D- (Data)
    │     │     └────────── USB D+ (Data)
    │     └──────────────── VBUS (5V Charging)
    └────────────────────── Ground Reference

Magnetic Alignment:
┌───┐                 ┌───┐
│ N │ ←── 3mm Gap ──→ │ S │ Neodym Magnets
└───┘                 └───┘
Device                 Dock

Connection Sequence:
1. Magnetic alignment (±2mm tolerance)
2. Pogo pin contact verification
3. USB enumeration (if data connection)
4. Charging protocol start (5V/300mA)
5. Status LED indication
```

## Pin Assignment Übersicht

| nRF52840 Pin | Signal Name | Funktion | Verbindung |
|--------------|-------------|----------|------------|
| P0.02 | PPG_INT | Input | MAX86141 INT |
| P0.03 | IMU_INT1 | Input | BMA400 INT1 |
| P0.04 | IMU_INT2 | Input | BMA400 INT2 |
| P0.05 | TEMP_INT | Input | MAX30205 INT |
| P0.06 | CHARGE_STAT | Input | BQ24074 STAT |
| P0.07 | BAT_ALERT | Input | MAX17048 ALRT |
| P0.13 | STATUS_LED | Output | RGB Status LED |
| P0.14 | CHARGE_LED | Output | Charge Indicator |
| P0.19 | SPI_SCK | Output | W25Q64 CLK |
| P0.20 | SPI_MOSI | Output | W25Q64 DI |
| P0.21 | SPI_MISO | Input | W25Q64 DO |
| P0.22 | SPI_CS | Output | W25Q64 CS |
| P0.26 | I2C_SDA | I/O | All I²C Devices |
| P0.27 | I2C_SCL | Output | All I²C Devices |

Dieser detaillierte Schaltplan zeigt alle Hauptkomponenten, deren Verbindungen und das Power Management System für den Whoop-Alternative Prototypen. Die Dokumentation umfasst alle notwendigen Informationen für die Hardware-Entwicklung und PCB-Design.
