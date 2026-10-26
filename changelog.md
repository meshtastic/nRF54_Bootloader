# Changelog

## 0.5.0 — 2026-10-26

Requires s145 10.0.1. Boards on 9.0.0 / v0.3.0 can't update by DFU: reflash
the SoftDevice, this bootloader and an application built for 10.0.1 over SWD
once. Without a chip erase, InternalFS and the bootloader settings are kept.

- s145 10.0.1 on nRF54L05/L10/L15, one SoftDevice hex per SoC; the L05 and
  L10 no longer get the L15 image
- SoftDevice base 0x5A800 / 0xDA800 / 0x15A800; DFU FWID 0x310D
  (`--sd-req 0x310D`, was 0x3024)
- Single-bank DFU checks the init packet before marking the application
  invalid; a refused package leaves the application in place
- `nrf_nvic.h` and `ble_l2cap.h` shims moved out of the vendored SoftDevice
  tree into `src/`

## 0.3.0 — 2026-09-14

- Documentation describes the s145 layout, the button handling per board and
  the CMake build; the Make build, `board.mk` files and the UF2 tooling are
  gone
- Display, NeoPixel and APA102 status LED code removed, no nRF54L board has
  them

## 0.2.0 — 2026-09-13

Ported to the s145 SoftDevice layout on nRF54L, verified on the XIAO nRF54L15.

- Bootloader owns RRAM 0x0 and the vector table (no MBR); application at
  0x8000, settings page just below the SoftDevice, CF2 config at 0x7C00
- SoftDevice interrupt and SVC forwarding to the handler table at the s145 base
- RRAM writes through nrfx_rramc while the SoftDevice is off, `sd_flash_write`
  while it is on; page erase emulated with an all-ones write
- BLE OTA: SoftDevice enabled with a valid LF clock config and a CRACEN seed
- Serial DFU on the board console UARTE20, verified with adafruit-nrfutil
- Bootloader NOINIT peer data and the double-reset marker at the top of RAM
- LF clock stopped at hand-over so the application can pick its own source
- MBR commands report NOT_SUPPORTED: SoftDevice and bootloader self-update are
  not available

## 0.1.0 — 2026-05-13

Initial release. Adafruit-style DFU bootloader ported to the Nordic
nRF54L series (nRF54L05, nRF54L10, nRF54L15) with SoftDevice S145
v9.0.0.

**Supported boards:** nRF54L15-DK, nRF54L10-DK, nRF54L05-DK,
XIAO nRF54L15, XIAO nRF54L15 Sense.

**Transports:** DFU over serial (UART) and OTA (BLE). No UF2 / USB-CDC
— nRF54L has no USB peripheral.

**Companion packages:**

- PlatformIO platform: [caveman99/platform-nordicnrf54](https://github.com/caveman99/platform-nordicnrf54)
- Arduino core framework: [caveman99/nRF54_Arduino](https://github.com/caveman99/nRF54_Arduino)
