/* Linker script to configure memory regions for nRF54L15 - DEBUG version */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  /** Flash start address for the bootloader in DEBUG mode.
   *  Debug build starts earlier to allow more space for debugging code.
   */
  /* nRF54L15: 1.5MB flash, debug bootloader starts earlier */
  FLASH (rx) : ORIGIN = 0x16A000, LENGTH = 0x17E000-0x16A000 - 2K /* ~78 KB for debug */

  BOOTLOADER_CONFIG (r): ORIGIN = 0x17E000 - 2K, LENGTH = 2K

  /** Location of mbr params page in flash. */
  MBR_PARAMS_PAGE (rw) : ORIGIN = 0x17E000, LENGTH = 0x1000

  /** Location of bootloader setting in flash. */
  BOOTLOADER_SETTINGS (rw) : ORIGIN = 0x17F000, LENGTH = 0x1000



  /** RAM Region for bootloader. nRF54L15 has 256KB RAM */
  /* Avoid conflict with NOINIT for OTA bond sharing */
  /* SoftDevice S145 uses 5.9 KB + 1.8 KB call stack = 7.7 KB, allocated 12 KB for configuration headroom */
  RAM (rwx) :  ORIGIN = 0x20003000, LENGTH = 0x20040000-0x20003000

  /* Location for double reset detection, no init */
  DBL_RESET (rwx) :  ORIGIN = 0x20002F7C, LENGTH = 0x04

  /** Location of non initialized RAM. Non initialized RAM is used for exchanging bond information
   *  from application to bootloader when using buttonless DFU OTA. */
  NOINIT (rwx) :  ORIGIN = 0x20002F80, LENGTH = 0x80



  /** Location in UICR where bootloader start address is stored. */
  UICR_BOOTLOADER (r) : ORIGIN = 0x10001014, LENGTH = 0x04

  /** Location in UICR where mbr params page address is stored. */
  UICR_MBR_PARAM_PAGE(r) : ORIGIN = 0x10001018, LENGTH = 0x04
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

  /* Place the bootloader settings page in flash. */
  .bootloaderSettings(NOLOAD) :
  {

  } > BOOTLOADER_SETTINGS

  /* Write the bootloader address in UICR. */
  .uicrBootStartAddress :
  {
    KEEP(*(.uicrBootStartAddress))
  } > UICR_BOOTLOADER

  /* Place the mbr params page in flash. */
    .mbrParamsPage(NOLOAD) :
  {

  } > MBR_PARAMS_PAGE

  /* Write the bootloader address in UICR. */
  .uicrMbrParamsPageAddress :
  {
    KEEP(*(.uicrMbrParamsPageAddress))
  } > UICR_MBR_PARAM_PAGE

  .dbl_reset(NOLOAD) :
  {

  } > DBL_RESET

  /* No init RAM section in bootloader. Used for bond information exchange. */
  .noinit(NOLOAD) :
  {

  } > NOINIT

  /* other placements follow here... */
}

INCLUDE "nrf_common.ld"
