/* NRF54L05 bootloader with the s145 SoftDevice.
 *
 * s145 has no MBR, so the bootloader owns RRAM 0x0 and the reset vector. Layout:
 *   0x00000000  bootloader (31 KB code + 1 KB CF2 config)
 *   0x00008000  application (own vector table, started by bootloader_app_start)
 *   0x0004f000  bootloader settings page (LittleFS ends one page below)
 *   0x0005a800  SoftDevice s145, up to the end of RRAM
 * The SoftDevice keeps RAM 0x20000000-0x20004800; the last 128 bytes of RAM hold the
 * double-reset marker and the BLE peer data handed over from the application. */
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x7C00
  BOOTLOADER_CONFIG (r) : ORIGIN = 0x00007C00, LENGTH = 0x400
  BOOTLOADER_SETTINGS (rw) : ORIGIN = 0x0004f000, LENGTH = 0x1000

  RAM (rwx) : ORIGIN = 0x20004800, LENGTH = 0x13780
  NOINIT (rwx) : ORIGIN = 0x20017f80, LENGTH = 0x78
  DBL_RESET (rwx) : ORIGIN = 0x20017ff8, LENGTH = 0x04
}

SECTIONS
{
  .fs_data_out ALIGN(4):
  {
    PROVIDE( __start_fs_data = .);
    KEEP(*(fs_data))
    PROVIDE( __stop_fs_data = .);
  } = 0

  .bootloaderConfig :
  {
    KEEP(*(.bootloaderConfig))
  } > BOOTLOADER_CONFIG

  .bootloaderSettings(NOLOAD) :
  {
  } > BOOTLOADER_SETTINGS

  .dbl_reset(NOLOAD) :
  {
  } > DBL_RESET

  .noinit(NOLOAD) :
  {
    *(.noinit*)
  } > NOINIT
}

INCLUDE "nrf_common.ld"
