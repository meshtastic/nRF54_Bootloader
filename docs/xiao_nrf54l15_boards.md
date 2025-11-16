# Seeed Studio XIAO nRF54L15 Boards

This document describes the board configurations for the Seeed Studio XIAO nRF54L15 and XIAO nRF54L15 Sense.

## Overview

The Seeed Studio XIAO nRF54L15 is an ultra-low-power development board in the popular XIAO form factor, featuring Nordic's nRF54L15 SoC. It supports multiple wireless protocols including Bluetooth LE 6.0, Matter, Thread, and Zigbee.

## Board Variants

### XIAO nRF54L15
- **Board ID**: `xiao_nrf54l15`
- **Model**: Base version without sensors
- **USB PID**: 0x0068
- **UF2 Volume**: XIAO54L15

### XIAO nRF54L15 Sense
- **Board ID**: `xiao_nrf54l15_sense`
- **Model**: Enhanced version with onboard sensors (6-axis IMU, PDM microphone)
- **USB PID**: 0x0069
- **UF2 Volume**: XIAO-SENSE

## Hardware Specifications

### Common Features
- **SoC**: Nordic nRF54L15
- **Processor**: ARM Cortex-M33 @ 128 MHz
- **Flash**: 1.5 MB RRAM
- **RAM**: 256 KB
- **USB**: USB-C interface
- **Form Factor**: XIAO (20mm x 17.5mm)
- **GPIO Pins**: 14 pins (11 digital, 6 analog)
- **Interfaces**: I2C, SPI, UART
- **Wireless**: BLE 6.0, Thread, Matter, Zigbee, Amazon Sidewalk

### XIAO nRF54L15 Sense Additional Features
- **IMU**: 6-axis motion sensor (LSM6DS3TR-C)
- **Microphone**: PDM digital microphone
- **Battery**: Onboard battery charging circuit

## Pin Configuration

### LED
- **User LED**: P2.0 (active low)
- Single LED for status indication

### Buttons
- **BUTTON_1**: P1.4 (D0 on XIAO connector)
- **BUTTON_2**: P1.5 (D1 on XIAO connector)
- Physical reset and boot buttons on board

### USB
- **VID**: 0x2886 (Seeed Studio)
- **PID**:
  - 0x0068 (XIAO nRF54L15)
  - 0x0069 (XIAO nRF54L15 Sense)

## Building the Bootloader

### Using CMake

**For XIAO nRF54L15:**
```bash
cmake -DBOARD=xiao_nrf54l15 -B_build
cmake --build _build
```

**For XIAO nRF54L15 Sense:**
```bash
cmake -DBOARD=xiao_nrf54l15_sense -B_build
cmake --build _build
```

### Using Make

**For XIAO nRF54L15:**
```bash
make BOARD=xiao_nrf54l15
```

**For XIAO nRF54L15 Sense:**
```bash
make BOARD=xiao_nrf54l15_sense
```

## Flashing the Bootloader

### Method 1: Using nrfjprog (J-Link)

```bash
# Flash bootloader
cmake --build _build --target flash-jlink

# Or with make
make BOARD=xiao_nrf54l15 flash
```

### Method 2: Using UF2 Bootloader

If a UF2 bootloader is already installed:

```bash
cmake --build _build --target flash-uf2

# The bootloader will be available as bootloader_mbr.uf2 in the build directory
```

### Method 3: Direct DFU

The XIAO nRF54L15 includes a SAMD11 with CMSIS-DAP for programming over USB.

## Entering Bootloader Mode

### Double-tap Reset
1. Quickly press the reset button twice
2. The LED will indicate bootloader mode
3. The board will appear as a USB mass storage device

### Hold Boot Button
1. Press and hold the boot button
2. Press the reset button
3. Release the reset button, then release the boot button

## UF2 Bootloader

The UF2 bootloader configuration:
- **Family ID**: 0xADA54115 (nRF54L15)
- **Flash Size**: 1.5 MB
- **RAM Size**: 256 KB
- **Product Name**: "Seeed XIAO nRF54L15" or "Seeed XIAO nRF54L15 Sense"

## XIAO Connector Pinout

The standard XIAO connector provides access to:

| Pin | GPIO | Function | Notes |
|-----|------|----------|-------|
| D0 | P1.4 | GPIO/ADC | Also BUTTON_1 |
| D1 | P1.5 | GPIO/ADC | Also BUTTON_2 |
| D2 | P1.6 | GPIO/ADC | - |
| D3 | P1.7 | GPIO/ADC | - |
| D4 | P1.10 | GPIO/ADC | - |
| D5 | P1.1 | GPIO/ADC | - |
| D6 | P2.8 | GPIO | SPI SCK |
| D7 | P2.7 | GPIO | SPI MISO |
| D8 | P2.1 | GPIO | SPI MOSI |
| D9 | P2.4 | GPIO | I2C SCL |
| D10 | P2.2 | GPIO | I2C SDA |

## Wireless Protocols

The nRF54L15 supports multiple wireless protocols:

### Bluetooth LE 6.0
- Channel Sounding for precise distance measurement
- Periodic Advertising with Responses
- Enhanced security features

### Matter
- Application-layer protocol for smart home devices
- Runs over Thread or Wi-Fi networks

### Thread
- IPv6-based mesh networking for IoT
- Low-power, self-healing mesh topology

### Zigbee
- IEEE 802.15.4-based mesh networking
- Wide ecosystem support

### Amazon Sidewalk
- Long-range, low-bandwidth network
- Community-powered connectivity

## Power Management

### Power Modes
- **Active**: ~3.8 mA (CPU active, radio RX)
- **Sleep**: ~1.7 µA (LFXO running, RAM retention)
- **Deep Sleep**: ~0.4 µA (minimal retention)

### Battery Operation (Sense variant)
- Built-in battery charging circuit
- LED indicator for charging status
- Supports 3.7V Li-Po batteries

## Programming and Debugging

### CMSIS-DAP Debug Interface
The XIAO nRF54L15 includes an onboard SAMD11 with CMSIS-DAP, providing:
- Drag-and-drop programming via UF2
- Serial logging over USB
- SWD debugging support

### Supported Development Environments
- Arduino IDE (with Seeed board package)
- PlatformIO
- Zephyr RTOS
- nRF Connect SDK

## Known Issues and Notes

### Pin Assignments
The button pin assignments (P1.4 and P1.5) are based on the XIAO form factor and may need verification against the actual hardware. These pins correspond to D0 and D1 on the XIAO connector.

### LED Behavior
- The LED (P2.0) is active low
- In bootloader mode, the LED will blink to indicate status
- User applications can control the LED after boot

### USB PID
The USB PIDs (0x0068 and 0x0069) are assigned sequentially following the existing XIAO nRF52840 pattern. If Seeed has assigned different PIDs, update the `board.h` files accordingly.

## Resources

### Official Documentation
- [Seeed Studio XIAO nRF54L15 Wiki](https://wiki.seeedstudio.com/xiao_nrf54l15_sense_getting_started/)
- [Seeed Studio Product Page](https://www.seeedstudio.com/XIAO-nRF54L15-p-6493.html)
- [nRF54L15 Product Page](https://www.nordicsemi.com/Products/nRF54L15)

### Hardware Files
- [Schematic PDF](https://files.seeedstudio.com/wiki/XIAO_nRF54L15/Getting_Start/nRF54L15_Schematic.pdf)
- KiCAD libraries and design files available from Seeed

### Software Resources
- [Zephyr Board Support](https://docs.zephyrproject.org/latest/boards/seeed/xiao_nrf54l15/doc/index.html)
- [Arduino Board Package](https://github.com/Seeed-Studio/)
- [Example Projects](https://wiki.seeedstudio.com/xiao_nrf54l15_sense_getting_started/)

## Troubleshooting

### Bootloader Not Entering
- Ensure proper timing on double-tap reset (within 500ms)
- Try holding boot button while pressing reset
- Check USB cable supports data transfer (not charge-only)

### Device Not Recognized
- Install Seeed XIAO drivers if on Windows
- Check USB VID/PID in device manager
- Try different USB port or cable

### Firmware Update Fails
- Verify UF2 file is for the correct board variant
- Check available flash space
- Ensure bootloader version is compatible

## Support

For issues specific to these board files:
- Check the Adafruit nRF52 Bootloader repository
- Review the nRF54L15 support documentation

For general XIAO nRF54L15 questions:
- [Seeed Studio Forum](https://forum.seeedstudio.com/)
- [Seeed Studio Discord](https://discord.gg/seeed)

## License

Board support files are released under MIT License, consistent with the Adafruit nRF52 Bootloader project.
