cmake_minimum_required(VERSION 3.17)
include(CMakePrintHelpers)
find_package(Python COMPONENTS Interpreter)

if (NOT DEFINED BOARD)
  message(FATAL_ERROR "BOARD is not defined")
endif ()

# include board specific
include(${CMAKE_CURRENT_LIST_DIR}/src/boards/${BOARD}/board.cmake)

# nRF54L only: Cortex-M33
set(CMAKE_SYSTEM_PROCESSOR cortex-m33 CACHE INTERNAL "System Processor")

# MCU_VARIANT is set by board.cmake (nrf54l15, nrf54l10, or nrf54l05)
if (NOT DEFINED MCU_VARIANT)
  set(MCU_VARIANT nrf54l15)
endif ()
set(JLINK_DEVICE ${MCU_VARIANT}_xxaa)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain/arm_gcc.cmake)

set(NRFX_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/nrfx)
set(SDK11_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/sdk11/components)
set(SDK_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/sdk/components)
set(SOFTDEVICE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/softdevice)
set(TCRYPT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/tinycrypt/lib)

option(DEFAULT_TO_OTA_DFU "Default to OTA DFU instead of Serial DFU" OFF)
option(SIGNED_FW "Enable signed firmware verification" OFF)
option(DUALBANK_FW "Enable dual bank DFU support" OFF)
set(SIGNED_FW_QX "" CACHE STRING "Public key Qx (comma-separated bytes)")
set(SIGNED_FW_QY "" CACHE STRING "Public key Qy (comma-separated bytes)")

#-------------------
# Bootloader
#-------------------
if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "Build type" FORCE)
endif ()

# SoftDevice S145 for nRF54L. 10.0.1 ships a separate hex per SoC, so SD_CHIP below is the
# SoC name rather than a family name.
if(NOT DEFINED SD_VERSION)
  set(SD_VERSION 10.0.1)
endif ()

project(nRF54L_Bootloader C ASM)
set(CMAKE_EXECUTABLE_SUFFIX .elf)
add_executable(bootloader
  # src
  src/dfu_ble_svc.c
  src/dfu_init.c
  src/sd_isr.c
  src/sd_isr.S
  src/flash_nrf5x.c
  src/main.c
  src/boards/boards.c
  src/boards/${BOARD}/pinconfig.c
  # nrfx (MDK in bsp/stable/mdk since nrfx 4.x)
  ${NRFX_DIR}/bsp/stable/mdk/system_nrf54l.c
  ${NRFX_DIR}/drivers/src/nrfx_rramc.c
  # sdk 11
  ${SDK11_DIR}/libraries/bootloader_dfu/bootloader.c
  ${SDK11_DIR}/libraries/bootloader_dfu/bootloader_settings.c
  ${SDK11_DIR}/libraries/bootloader_dfu/bootloader_util.c
  ${SDK11_DIR}/libraries/bootloader_dfu/dfu_transport_serial.c
  ${SDK11_DIR}/libraries/bootloader_dfu/dfu_transport_ble.c
  ${SDK11_DIR}/ble/ble_services/ble_dfu/ble_dfu.c
  ${SDK11_DIR}/ble/ble_services/ble_dis/ble_dis.c
  ${SDK11_DIR}/drivers_nrf/pstorage/pstorage_raw.c
  # latest sdk
  src/app_timer_nrf54l.c
  ${SDK_DIR}/libraries/scheduler/app_scheduler.c
  ${SDK_DIR}/libraries/util/app_error.c
  ${SDK_DIR}/libraries/util/app_util_platform.c
  ${SDK_DIR}/libraries/crc16/crc16.c
  ${SDK_DIR}/libraries/hci/hci_mem_pool.c
  ${SDK_DIR}/libraries/hci/hci_slip.c
  ${SDK_DIR}/libraries/hci/hci_transport.c
  ${SDK_DIR}/libraries/util/nrf_assert.c
  # UART transport (nRF54L has no USB)
  ${SDK_DIR}/libraries/uart/app_uart.c
  ${SDK_DIR}/drivers_nrf/uart/nrf_drv_uart.c
  ${SDK_DIR}/drivers_nrf/common/nrf_drv_common.c
  # startup (canonical nrfx 4.x startup)
  ${NRFX_DIR}/bsp/stable/mdk/gcc_startup_nrf54l15_application.S
  )

if (DUALBANK_FW)
  target_sources(bootloader PUBLIC
    ${SDK11_DIR}/libraries/bootloader_dfu/dfu_dual_bank.c
  )
else ()
  target_sources(bootloader PUBLIC
    ${SDK11_DIR}/libraries/bootloader_dfu/dfu_single_bank.c
  )
endif ()

if (SIGNED_FW)
  target_sources(bootloader PUBLIC
    ${TCRYPT_DIR}/source/sha256.c
    ${TCRYPT_DIR}/source/ecc.c
    ${TCRYPT_DIR}/source/ecc_dsa.c
    ${TCRYPT_DIR}/source/utils.c
  )

  target_include_directories(bootloader PUBLIC
    ${TCRYPT_DIR}/include
  )
endif ()

target_include_directories(bootloader PUBLIC
  src
  src/boards
  src/boards/${BOARD}
  src/cmsis/include
  # nrfx 4.2.0 — MDK and soc live under bsp/stable/
  ${NRFX_DIR}
  ${NRFX_DIR}/bsp/stable/mdk
  ${NRFX_DIR}/hal
  ${NRFX_DIR}/haly
  ${NRFX_DIR}/drivers/include
  ${NRFX_DIR}/drivers/src
  ${NRFX_DIR}/helpers
  ${NRFX_DIR}/lib
  ${NRFX_DIR}/bsp/stable
  ${NRFX_DIR}/bsp/stable/soc
  ${NRFX_DIR}/bsp/stable/soc/irqs
  # sdk 11 for ble dfu
  ${SDK11_DIR}/libraries/bootloader_dfu
  ${SDK11_DIR}/libraries/bootloader_dfu/hci_transport
  ${SDK11_DIR}/libraries/util
  ${SDK11_DIR}/drivers_nrf/pstorage
  ${SDK11_DIR}/ble/common
  ${SDK11_DIR}/ble/ble_services/ble_dfu
  ${SDK11_DIR}/ble/ble_services/ble_dis
  # later sdk with updated drivers
  ${SDK_DIR}/libraries/timer
  ${SDK_DIR}/libraries/scheduler
  ${SDK_DIR}/libraries/crc16
  ${SDK_DIR}/libraries/util
  ${SDK_DIR}/libraries/hci/config
  ${SDK_DIR}/libraries/hci
  ${SDK_DIR}/libraries/uart
  ${SDK_DIR}/drivers_nrf/delay
  ${SDK_DIR}/drivers_nrf/common
  ${SDK_DIR}/drivers_nrf/uart
  )

set(LD_FILE ${CMAKE_CURRENT_LIST_DIR}/linker/${MCU_VARIANT}.ld)

target_link_options(bootloader PUBLIC
  "LINKER:--script=${LD_FILE}"
  -L${NRFX_DIR}/bsp/stable/mdk
  -nostartfiles
  --specs=nosys.specs --specs=nano.specs
  )
target_compile_options(bootloader PUBLIC
  -fno-builtin
  -fshort-enums
  -fstack-usage
  -fno-strict-aliasing
  -Wall
  -Wextra
  -Werror
  -Wfatal-errors
  -Werror-implicit-function-declaration
  -Wfloat-equal
  -Wundef
  -Wshadow
  -Wwrite-strings
  -Wsign-compare
  -Wmissing-format-attribute
  -Wno-endif-labels
  -Wunreachable-code
  # Suppress warning caused by SDK
  -Wno-unused-parameter -Wno-expansion-to-defined -Wno-array-bounds
  )
target_compile_definitions(bootloader PUBLIC
  SOFTDEVICE_PRESENT
  CONFIG_GPIO_AS_PINRESET
  NRF_APPLICATION
  __START=main
  __STARTUP_CLEAR_BSS
  __STACK_SIZE=8192
  __HEAP_SIZE=0
  )

if (SIGNED_FW)
  if (SIGNED_FW_QX STREQUAL "" OR SIGNED_FW_QY STREQUAL "")
    message(FATAL_ERROR "SIGNED_FW requires SIGNED_FW_QX and SIGNED_FW_QY to be set")
  endif ()
  string(REPLACE " " "" SIGNED_FW_QX_STR "${SIGNED_FW_QX}")
  string(REPLACE " " "" SIGNED_FW_QY_STR "${SIGNED_FW_QY}")
  target_compile_definitions(bootloader PUBLIC
    SIGNED_FW
    "SIGNED_FW_QX=${SIGNED_FW_QX_STR}"
    "SIGNED_FW_QY=${SIGNED_FW_QY_STR}"
  )
endif ()

if (DEFAULT_TO_OTA_DFU)
  target_compile_definitions(bootloader PUBLIC DEFAULT_TO_OTA_DFU)
endif ()

if (TRACE_ETM STREQUAL "1")
  target_compile_definitions(bootloader PUBLIC ENABLE_TRACE)
endif ()

# nRF54L MCU variant selection
if (MCU_VARIANT STREQUAL "nrf54l15")
  set(DFU_DEV_REV 54115)
  set(DFU_APP_DATA_RESERVED 8*4096)
  set(SD_CHIP nrf54l15)
  target_compile_definitions(bootloader PUBLIC NRF54L15_XXAA)
elseif (MCU_VARIANT STREQUAL "nrf54l10")
  set(DFU_DEV_REV 54110)
  set(DFU_APP_DATA_RESERVED 8*4096)
  set(SD_CHIP nrf54l10)
  target_compile_definitions(bootloader PUBLIC NRF54L10_XXAA NRF54L15_XXAA)
elseif (MCU_VARIANT STREQUAL "nrf54l05")
  set(DFU_DEV_REV 54105)
  set(DFU_APP_DATA_RESERVED 8*4096)
  set(SD_CHIP nrf54l05)
  target_compile_definitions(bootloader PUBLIC NRF54L05_XXAA NRF54L15_XXAA)
else ()
  message(FATAL_ERROR "Unknown MCU_VARIANT: ${MCU_VARIANT}")
endif ()

if (NOT DEFINED SD_NAME)
  set(SD_NAME s145)
endif ()

string(TOUPPER ${SD_NAME} SD_NAME_UPPER)
target_compile_definitions(bootloader PUBLIC ${SD_NAME_UPPER})

set(SD_FILENAME ${SD_NAME}_${SD_CHIP}_${SD_VERSION})
set(SD_HEX ${SOFTDEVICE_DIR}/${SD_FILENAME}/${SD_FILENAME}_softdevice.hex)

target_compile_definitions(bootloader PUBLIC ${SD_NAME_UPPER})
target_include_directories(bootloader PUBLIC
  ${SOFTDEVICE_DIR}/${SD_FILENAME}/${SD_FILENAME}_API/include
  )

# debug build
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_sources(bootloader PUBLIC lib/SEGGER_RTT/RTT/SEGGER_RTT.c)
  target_include_directories(bootloader PUBLIC lib/SEGGER_RTT/RTT)
  target_compile_definitions(bootloader PUBLIC
    CFG_DEBUG
    SEGGER_RTT_MODE_DEFAULT=SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL
  )
endif ()
target_compile_definitions(bootloader PUBLIC DFU_APP_DATA_RESERVED=${DFU_APP_DATA_RESERVED})

#----------------------------------
# ANT_LICENSE_KEY handling
#----------------------------------
if (DEFINED ANT_LICENSE_KEY)
  target_compile_definitions(bootloader PUBLIC ANT_LICENSE_KEY=${ANT_LICENSE_KEY})
endif()

#----------------------------------
# Get bootloader version from git
#----------------------------------
execute_process(COMMAND git describe --dirty --always --tags OUTPUT_VARIABLE GIT_VERSION)
string(STRIP ${GIT_VERSION} GIT_VERSION)

# Parse version from git tag (e.g. "0.9.2-111-gabcdef-dirty" → 0.9.2)
# If no tag exists, git describe --always returns just a commit hash — default to 0.0.0
string(REPLACE "-" ";" RELEASE_VERSION ${GIT_VERSION})
list(GET RELEASE_VERSION 0 RELEASE_VERSION)
string(REPLACE "." ";" RELEASE_VERSION ${RELEASE_VERSION})
list(LENGTH RELEASE_VERSION _ver_len)
if(_ver_len GREATER_EQUAL 3)
  list(GET RELEASE_VERSION 0 RELEASE_VERSION_MAJOR)
  list(GET RELEASE_VERSION 1 RELEASE_VERSION_MINOR)
  list(GET RELEASE_VERSION 2 RELEASE_VERSION_PATCH)
else()
  set(RELEASE_VERSION_MAJOR 0)
  set(RELEASE_VERSION_MINOR 0)
  set(RELEASE_VERSION_PATCH 0)
endif()
math(EXPR MK_BOOTLOADER_VERSION "(${RELEASE_VERSION_MAJOR} << 16) + (${RELEASE_VERSION_MINOR} << 8) + ${RELEASE_VERSION_PATCH}")

cmake_print_variables(GIT_VERSION MK_BOOTLOADER_VERSION)

target_compile_definitions(bootloader PUBLIC
  BLEDIS_FW_VERSION="${GIT_VERSION} ${SD_NAME} ${SD_VERSION}"
  MK_BOOTLOADER_VERSION=${MK_BOOTLOADER_VERSION}
  )

# Add bin/hex output
add_custom_command(TARGET bootloader POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:bootloader> $<TARGET_FILE_DIR:bootloader>/bootloader.bin
  COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:bootloader> $<TARGET_FILE_DIR:bootloader>/bootloader.hex
  VERBATIM)

#----------------------------------
# Flashing target
#----------------------------------
if (NOT DEFINED NRFJPROG)
  set(NRFJPROG nrfjprog)
endif()

add_custom_target(flash-bootloader
  DEPENDS bootloader
  COMMAND ${NRFJPROG} --program $<TARGET_FILE:bootloader> --verify --sectoranduicrerase -f nrf54l --reset
  )

add_custom_target(flash-sd
  COMMAND ${NRFJPROG} --program ${SD_HEX} --verify --sectorerase -f nrf54l
  )

add_custom_target(flash-erase
  COMMAND ${NRFJPROG} -f nrf54l --eraseall
  )

add_custom_target(flash-all
  DEPENDS bootloader flash-erase
  COMMAND ${NRFJPROG} --program ${SD_HEX} --verify --sectorerase -f nrf54l
  COMMAND ${NRFJPROG} --program $<TARGET_FILE:bootloader> --verify --sectoranduicrerase -f nrf54l --reset
  )
