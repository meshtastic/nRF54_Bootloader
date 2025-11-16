# nRF54L15 Support

This document describes the nRF54L15 bare metal support added to the Adafruit nRF52 Bootloader.

## Overview

Nordic Semiconductor announced bare metal support for the nRF54L series chips in August 2025. This implementation adds support for the nRF54L15 SoC with SoftDevice S145, following the same architecture patterns used for the nRF52 series.

## Specifications

### nRF54L15 Hardware
- **Flash (RRAM):** 1.5 MB (1,536 KB)
- **RAM:** 256 KB
- **Processor:** ARM Cortex-M33 with TrustZone
- **Bluetooth:** Bluetooth LE 5.4, Bluetooth 6.0 ready

### SoftDevice S145
- **Type:** Central and multirole Bluetooth LE protocol stack
- **Connections:** Up to 5 concurrent links in all roles (Broadcaster, Central, Observer, Peripheral)
- **Features:** Coded PHY and Extended Advertising support
- **Version:** 1.0.0 (initial support)

## Memory Layout

### Flash Memory Map (1.5 MB total)
```
0x000000 - 0x001000: MBR (4 KB)
0x001000 - 0x02D000: SoftDevice S145 (~180 KB estimated)
0x02D000 - 0x16A000: Application Space (~1.24 MB)
0x16A000 - 0x17D800: Bootloader (Debug, 78 KB)
0x174000 - 0x17D800: Bootloader (Release, 38 KB)
0x17D800 - 0x17E000: Bootloader Config (2 KB)
0x17E000 - 0x17F000: MBR Parameters Page (4 KB)
0x17F000 - 0x180000: Bootloader Settings (4 KB)
```

### RAM Memory Map (256 KB total)
```
0x20000000 - 0x20004000: SoftDevice RAM (~16 KB estimated)
0x20003F7C - 0x20003F80: Double Reset Detection (4 bytes)
0x20003F80 - 0x20004000: NOINIT (128 bytes, bond information)
0x20004000 - 0x20040000: Bootloader RAM (240 KB)
```

## Implementation Details

### Files Added

1. **Linker Scripts**
   - `linker/nrf54l15.ld` - Release build linker script
   - `linker/nrf54l15_debug.ld` - Debug build linker script

2. **Board Configuration** (Example: PCA10156 - nRF54L15 DK)
   - `src/boards/pca10156/board.cmake` - CMake board configuration
   - `src/boards/pca10156/board.mk` - Makefile board configuration
   - `src/boards/pca10156/board.h` - Board-specific defines
   - `src/boards/pca10156/pinconfig.c` - UF2 bootloader configuration

### Files Modified

1. **Build System**
   - `CMakeLists.txt` - Added nrf54l15 MCU variant support
   - `Makefile` - Added nrf54l15 build configuration

2. **Source Code**
   - `src/dfu_init.c` - Added NRF54L15_XXAA device revision (54115)
   - `src/usb/uf2/uf2cfg.h` - Added UF2 family ID (0xADA54115) and flash size

### Configuration Values

| Parameter | Value | Description |
|-----------|-------|-------------|
| MCU_VARIANT | nrf54l15 | MCU variant identifier |
| DFU_DEV_REV | 54115 | Device revision for DFU safety checks |
| DFU_APP_DATA_RESERVED | 40 KB | Reserved space for DFU |
| SD_NAME | s145 | SoftDevice name |
| SD_VERSION | 1.0.0 | SoftDevice version (default) |
| UF2_FAMILY_ID | 0xADA54115 | UF2 family identifier |
| BOOTLOADER_START | 0x174000 | Bootloader start address (release) |
| BOOTLOADER_START_DEBUG | 0x16A000 | Bootloader start address (debug) |

## Building for nRF54L15

### Prerequisites

1. **Update nrfx submodule** to a version that supports nRF54L15:
   ```bash
   git submodule update --init --recursive
   # Or update to a specific version with nRF54L15 support
   cd lib/nrfx
   git checkout <version-with-nrf54l15-support>
   cd ../..
   ```

2. **Install SoftDevice S145**:
   - Download SoftDevice S145 for nRF54L from Nordic Semiconductor
   - Extract to `lib/softdevice/s145_nrf54l_1.0.0/`
   - The directory should contain the SoftDevice hex file and API headers

### Build Commands

**Using CMake:**
```bash
# Build for PCA10156 (nRF54L15-DK)
cmake -DBOARD=pca10156 -B_build
cmake --build _build

# Flash with nrfjprog
cmake --build _build --target flash-jlink
```

**Using Make:**
```bash
# Build for PCA10156
make BOARD=pca10156

# Flash with nrfjprog
make BOARD=pca10156 flash
```

## Creating Custom Boards

To create a custom nRF54L15 board configuration:

1. Create a new directory in `src/boards/<your_board_name>/`
2. Create the following files based on `pca10156` example:
   - `board.cmake` - Set `MCU_VARIANT` to `nrf54l15`
   - `board.mk` - Set `MCU_SUB_VARIANT` to `nrf54l15`
   - `board.h` - Define LEDs, buttons, USB IDs, etc.
   - `pinconfig.c` - Configure UF2 bootloader settings

3. Update `board.h` with your board-specific:
   - LED pins
   - Button pins
   - USB VID/PID
   - Board name and URLs

4. Build with your board: `cmake -DBOARD=<your_board_name> -B_build`

## Migration from nRF52 Series

The nRF54L15 support follows the same patterns as nRF52 series, making migration straightforward:

### Similarities
- Same SoftDevice architecture and API style
- Compatible DFU protocol
- Same UF2 bootloader mechanism
- Similar build system structure

### Key Differences
- **Processor:** Cortex-M33 instead of Cortex-M4
- **Flash Type:** RRAM instead of traditional Flash
- **Memory Size:** 1.5 MB flash / 256 KB RAM (vs 1 MB / 256 KB on nRF52840)
- **SoftDevice:** S145 instead of S132/S140
- **Memory Layout:** Different bootloader addresses due to larger flash

## Testing Status

This implementation is initial support based on Nordic's bare metal announcement and nRF54L15 specifications. Full testing requires:

- [ ] nrfx library with nRF54L15 support
- [ ] SoftDevice S145 official release
- [ ] Physical nRF54L15 hardware (PCA10156 DK)
- [ ] Build verification
- [ ] Flash and execution testing
- [ ] DFU functionality verification
- [ ] USB functionality verification

## Additional Resources

- [Nordic nRF54L15 Product Page](https://www.nordicsemi.com/Products/nRF54L15)
- [Nordic Bare Metal SDK Announcement](https://www.nordicsemi.com/Products/Development-software/nRF-Connect-SDK/Bare-Metal-option-for-nRF54L-Series)
- [nRF54L15 Development Kit](https://www.nordicsemi.com/Products/Development-hardware/nRF54L15-DK)

## Notes

- The UF2 family ID `0xADA54115` is derived from the nRF52 pattern and may need to be registered officially
- SoftDevice S145 version 1.0.0 is assumed; update `SD_VERSION` in board configuration if using a different version
- Exact SoftDevice memory requirements will be confirmed once S145 is officially released
- The bootloader assumes USB support similar to nRF52840; boards without USB may need modifications

## Future Work

- [ ] Update once official SoftDevice S145 is released with confirmed memory requirements
- [ ] Add support for other nRF54L variants (nRF54L10, nRF54L05)
- [ ] Add support for SoftDevice S115 (peripheral-only, memory-optimized)
- [ ] Verify and potentially adjust memory layout based on actual S145 size
- [ ] Test and validate on actual hardware
- [ ] Add CI/CD build verification for nRF54L15 targets
