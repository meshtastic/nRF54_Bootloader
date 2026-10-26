# nRF54L Bootloader

BLE DFU bootloader for Nordic nRF54L series (nRF54L15, nRF54L10, nRF54L05).

Based on the Nordic nRF5 SDK bootloader architecture, ported to nRF54L with SoftDevice S145.

## Supported Boards

| Board | Target | MCU |
|-------|--------|-----|
| nRF54L15-DK | `nrf54l15dk` | nRF54L15 |
| nRF54L10-DK | `nrf54l10dk` | nRF54L10 |
| nRF54L05-DK | `nrf54l05dk` | nRF54L05 |
| XIAO nRF54L15 | `xiao_nrf54l15` | nRF54L15 |
| XIAO nRF54L15 Sense | `xiao_nrf54l15_sense` | nRF54L15 |

## Features

- DFU over Serial (UART) and OTA (BLE)
- Dual bank firmware updates (disabled by default)
- Signed firmware updates (disabled by default)
- Watchdog feed-through during DFU

Not available: UF2 and USB CDC (nRF54L has no USB peripheral), and
SoftDevice or bootloader self-update (s145 has no MBR, the MBR commands
report `NRF_ERROR_NOT_SUPPORTED`).

## Memory layout

The bootloader owns RRAM `0x0` and the reset vector; the application has
its own vector table at `0x8000` and forwards the SoftDevice interrupts.

| Region | nRF54L05 | nRF54L10 | nRF54L15 |
|--------|----------|----------|----------|
| Bootloader | `0x0 – 0x7C00` | `0x0 – 0x7C00` | `0x0 – 0x7C00` |
| CF2 config | `0x7C00` | `0x7C00` | `0x7C00` |
| Application | `0x8000 – 0x47000` | `0x8000 – 0xC7000` | `0x8000 – 0x147000` |
| Bootloader settings page | `0x4F000` | `0xCF000` | `0x14F000` |
| SoftDevice s145 10.0.1 | `0x5A800` | `0xDA800` | `0x15A800` |

RAM: `0x20000000 – 0x20004800` SoftDevice, bootloader and application from
`0x20004800`; the last 128 bytes (`0x2003FF80`) hold the BLE peer data and
the double-reset marker.

## How to use

The bootloader checks the board buttons on reset:

- **DFU button held**: enter serial DFU mode
- **DFU and OTA buttons held**: enter OTA DFU mode (for Nordic nRF Connect/Toolbox)
- otherwise: boot the application if present, else enter DFU
- **Double reset** within 500 ms: enter DFU mode

On the nRF54L15-DK, `DFU` is **Button1** and `OTA` is **Button2**. The XIAO
boards only have the user button (`DFU`); use `GPREGRET` for OTA.

The application can also trigger DFU via the `GPREGRET` register:

```c
void reset_to_dfu(void) {
    NRF_POWER->GPREGRET[0] = 0x4e; // 0xA8 for OTA
    NVIC_SystemReset();
}
```

Serial DFU runs on the board console UART (UARTE20, the SAMD11 bridge on
the XIAO, VCOM0 on the DK) at 115200 baud and waits 3 s for the first
packet when entered through `GPREGRET`.

## Building

### Prerequisites

- ARM GCC toolchain (12.x or later)
- CMake 3.17+ and Ninja
- Nordic [nRF Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools) or [pyocd](https://pyocd.io) for flashing
- [adafruit-nrfutil](https://github.com/adafruit/Adafruit_nRF52_nrfutil) for serial DFU

### Clone

```
git clone --recurse-submodules https://github.com/meshtastic/nRF54_Bootloader.git
```

### Build

```bash
cmake -G Ninja -B _build -DBOARD=nrf54l15dk -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build _build
```

Output: `_build/bootloader.hex`

### Flash

With nrfjprog (J-Link):

```bash
cmake --build _build --target flash-all   # erase, SoftDevice, bootloader
cmake --build _build --target flash-bootloader
cmake --build _build --target flash-sd
```

With pyocd (CMSIS-DAP, e.g. the XIAO's SAMD11 bridge):

```bash
pyocd load -t nrf54l --erase chip _build/bootloader.hex
pyocd load -t nrf54l --erase sector <s145_softdevice.hex>
```

### DFU an application over serial

```bash
adafruit-nrfutil dfu genpkg --dev-type 0x0054 --sd-req 0x310D --application app.hex app_dfu.zip
adafruit-nrfutil dfu serial --package app_dfu.zip -p /dev/ttyACM0 -b 115200 --singlebank
```

`--sd-req` is the SoftDevice FWID: `0x310D` for s145 10.0.1, on every
variant. It was `0x3024` for s145 9.0.0, which this bootloader no longer
ships; devices still running that SoftDevice need it reflashed.

## Build options

| Option | Default | Description |
|--------|---------|-------------|
| `DUALBANK_FW` | `0` | Split flash into two banks for safe updates |
| `SIGNED_FW` | `0` | Require signed firmware packages |
| `DEFAULT_TO_OTA_DFU` | `0` | Default to OTA DFU instead of serial |

### Signed firmware

Generate a signing key:

```bash
nrfutil keys --gen-key stored_key.pem
nrfutil keys --show-vk code stored_key.pem
```

Build with signing enabled:

```bash
cmake -G Ninja -B _build -DBOARD=nrf54l15dk -DSIGNED_FW=ON -DSIGNED_FW_QX='...' -DSIGNED_FW_QY='...'
```

Create a signed DFU package:

```bash
adafruit-nrfutil dfu genpkg --dev-type 0x0054 --sd-req 0x310D --application app.hex --key-file stored_key.pem app_dfu.zip
```

## Adding a new board

1. Create `src/boards/<board_name>/` with:
   - `board.h` — pin definitions (LEDs, buttons, UART)
   - `board.cmake` — set `MCU_VARIANT` (nrf54l15, nrf54l10, or nrf54l05)
   - `pinconfig.c` — board metadata

2. Build with `cmake -DBOARD=<board_name> ...`

See existing boards in `src/boards/` for reference.

## License

MIT License. See [LICENSE](LICENSE) for details.
