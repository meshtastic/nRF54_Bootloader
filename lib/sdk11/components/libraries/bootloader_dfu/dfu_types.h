/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/**@file
 *
 * @defgroup nrf_dfu_types Types and definitions.
 * @{
 *
 * @ingroup nrf_dfu
 *
 * @brief Device Firmware Update module type and definitions.
 */

/*
 * RRAM layout (nRF54L15, see linker/nrf54l15.ld):
 *
 *  -------------------------
 * |  Bootloader Settings    |
 * |          4 KB           |
 * |-------------------------| 0x17F000 BOOTLOADER_SETTINGS_ADDRESS
 * |     MBR Params Page     |
 * |          4 KB           |
 * |-------------------------| 0x17E000
 * |       Bootloader        |
 * |         ~31 KB          |
 * |-------------------------| 0x150000 BOOTLOADER_REGION_START
 * |    App Data Reserved    |
 * |                         |
 * |-------------------------|
 * |       Application       |
 * |    Single/ Dual Bank    |
 * |                         |
 * |-------------------------| CODE_REGION_1_START
 * |        SoftDevice       |
 * |      S145 (optional)    |
 * |-------------------------| 0x01000
 * |           MBR           |
 * |          4 KB           |
 *  -------------------------  0x00000
 */

#ifndef DFU_TYPES_H__
#define DFU_TYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_sdm.h"
#include "nrf_mbr.h"
#include "nrf.h"
#include "app_util.h"

#ifndef MBR_SIZE
#define MBR_SIZE  (0x1000)
#endif


/* nRF54L has no MBR SVC for VTOR — write the register directly */
static inline uint32_t sd_softdevice_vector_table_base_set(uint32_t addr)
{
    *(volatile uint32_t *)0xE000ED08UL = addr;
    return 0; /* NRF_SUCCESS */
}


// Application address is either after MBR or SD (if existed)

#define CODE_PAGE_SIZE                      0x1000                      /**< Size of a flash codepage. Used for size of the reserved flash space in the bootloader region. Will be runtime checked against NRF_UICR->CODEPAGESIZE to ensure the region is correct. */

/* Check smaller variants first — L10/L05 also define NRF54L15_XXAA
 * for header compatibility, so L15 must be the fallback. */
// nRF54L layout (no MBR): bootloader at 0x0, application at 0x8000, settings page just below
// the SoftDevice which sits at the top of RRAM. The region between the application start and
// the settings page is what DFU may write.
#define BOOTLOADER_REGION_START             0x00000000
#define CODE_REGION_1_START                 0x00008000
#if defined(NRF54L05_XXAA)
  #define SOFTDEVICE_REGION_START           0x0005A800
  #define BOOTLOADER_SETTINGS_ADDRESS       0x0004F000
#elif defined(NRF54L10_XXAA)
  #define SOFTDEVICE_REGION_START           0x000DA800
  #define BOOTLOADER_SETTINGS_ADDRESS       0x000CF000
#elif defined(NRF54L15_XXAA)
  #define SOFTDEVICE_REGION_START           0x0015A800
  #define BOOTLOADER_SETTINGS_ADDRESS       0x0014F000
#else
  #error "No nRF54L target defined"
#endif
#define DFU_APP_REGION_END                  BOOTLOADER_SETTINGS_ADDRESS
#define SOFTDEVICE_INFO_STRUCT_ADDRESS      (SOFTDEVICE_REGION_START + SOFTDEVICE_INFO_STRUCT_OFFSET)

#ifndef SD_MAGIC_NUMBER
#define SD_MAGIC_NUMBER 0x51B1E5DB
#endif

static inline bool is_sd_existed(void)
{
  return *((uint32_t*)(SOFTDEVICE_INFO_STRUCT_ADDRESS+4)) == SD_MAGIC_NUMBER;
}


#define DFU_REGION_TOTAL_SIZE           (DFU_APP_REGION_END - CODE_REGION_1_START)                 /**< Total size of the region between SD and Bootloader. */

#ifndef DFU_APP_DATA_RESERVED
  #error "DFU_APP_DATA_RESERVED is not defined"
#endif

#define DFU_IMAGE_MAX_SIZE_FULL         (DFU_REGION_TOTAL_SIZE - DFU_APP_DATA_RESERVED)                 /**< Maximum size of an application, excluding save data from the application. */
#define DFU_IMAGE_MAX_SIZE_BANKED       (((DFU_IMAGE_MAX_SIZE_FULL) - \
                                        (DFU_IMAGE_MAX_SIZE_FULL % (2 * CODE_PAGE_SIZE)))/2)            /**< Maximum size of an application, excluding save data from the application. */

#define DFU_BL_IMAGE_MAX_SIZE           (CODE_REGION_1_START - BOOTLOADER_REGION_START)  /**< Maximum size of a bootloader, excluding save data from the current bootloader. */
#define DFU_BANK_0_REGION_START         CODE_REGION_1_START                                             /**< Bank 0 region start. */
#define DFU_BANK_1_REGION_START         (DFU_BANK_0_REGION_START + DFU_IMAGE_MAX_SIZE_BANKED)           /**< Bank 1 region start. */

#define EMPTY_FLASH_MASK                0xFFFFFFFF                                                      /**< Bit mask that defines an empty address in flash. */

#define INVALID_PACKET                  0x00                                                            /**< Invalid packet identifies. */
#define INIT_PACKET                     0x01                                                            /**< Packet identifies for initialization packet. */
#define STOP_INIT_PACKET                0x02                                                            /**< Packet identifies for stop initialization packet. Used when complete init packet has been received so that the init packet can be used for pre validaiton. */
#define START_PACKET                    0x03                                                            /**< Packet identifies for the Data Start Packet. */
#define DATA_PACKET                     0x04                                                            /**< Packet identifies for a Data Packet. */
#define STOP_DATA_PACKET                0x05                                                            /**< Packet identifies for the Data Stop Packet. */

#define DFU_UPDATE_SD                   0x01                                                            /**< Bit field indicating update of SoftDevice is ongoing. */
#define DFU_UPDATE_BL                   0x02                                                            /**< Bit field indicating update of bootloader is ongoing. */
#define DFU_UPDATE_APP                  0x04                                                            /**< Bit field indicating update of application is ongoing. */

#define DFU_INIT_RX                     0x00                                                            /**< Op Code identifies for receiving init packet. */
#define DFU_INIT_COMPLETE               0x01                                                            /**< Op Code identifies for transmission complete of init packet. */

// Safe guard to ensure during compile time that the DFU_APP_DATA_RESERVED is a multiple of page size.
STATIC_ASSERT((((DFU_APP_DATA_RESERVED) & (CODE_PAGE_SIZE - 1)) == 0x00));

/**@brief Structure holding a start packet containing update mode and image sizes.
 */
typedef struct
{
    uint8_t  dfu_update_mode;                                                                           /**< Packet type, used to identify the content of the received packet referenced by data packet. */
    uint32_t sd_image_size;                                                                             /**< Size of the SoftDevice image to be transferred. Zero if no SoftDevice image will be transfered. */
    uint32_t bl_image_size;                                                                             /**< Size of the Bootloader image to be transferred. Zero if no Bootloader image will be transfered. */
    uint32_t app_image_size;                                                                            /**< Size of the application image to be transmitted. Zero if no Bootloader image will be transfered. */
} dfu_start_packet_t;

/**@brief Structure holding a bootloader init/data packet received.
 */
typedef struct
{
    uint32_t   packet_length;                                                                           /**< Packet length of the data packet. Each data is word size, meaning length of 4 is 4 words, not bytes. */
    uint32_t * p_data_packet;                                                                           /**< Data Packet received. Each data is a word size entry. */
} dfu_data_packet_t;

/**@brief Structure for holding dfu update packet. Packet type indicate the type of packet.
 */
typedef struct
{
    uint32_t   packet_type;                                                                             /**< Packet type, used to identify the content of the received packet referenced by data packet. */
    union
    {
        dfu_data_packet_t    data_packet;                                                               /**< Used when packet type is INIT_PACKET or DATA_PACKET. Packet contains data received for init or data. */
        dfu_start_packet_t * start_packet;                                                              /**< Used when packet type is START_DATA_PACKET. Will contain information on software to be updtaed, i.e. SoftDevice, Bootloader and/or Application along with image sizes. */
    } params;
} dfu_update_packet_t;

/**@brief DFU status error codes.
*/
typedef enum
{
    DFU_UPDATE_APP_COMPLETE,                                                                            /**< Status update of application complete.*/
    DFU_UPDATE_SD_COMPLETE,                                                                             /**< Status update of SoftDevice update complete. Note that this solely indicates that a new SoftDevice has been received and stored in bank 0 and 1. */
    DFU_UPDATE_SD_SWAPPED,                                                                              /**< Status update of SoftDevice update complete. Note that this solely indicates that a new SoftDevice has been received and stored in bank 0 and 1. */
    DFU_UPDATE_BOOT_COMPLETE,                                                                           /**< Status update complete.*/
    DFU_BANK_0_ERASED,                                                                                  /**< Status bank 0 erased.*/
    DFU_TIMEOUT,                                                                                        /**< Status timeout.*/
    DFU_RESET,                                                                                           /**< Status Reset to indicate current update procedure has been aborted and system should reset. */
    DFU_UF2_BOOTLOADER_COMPLETE
} dfu_update_status_code_t;

/**@brief Structure holding DFU complete event.
*/
typedef struct
{
    dfu_update_status_code_t status_code;                                                               /**< Device Firmware Update status. */
    uint16_t                 app_crc;                                                                   /**< CRC of the recieved application. */
    uint32_t                 sd_size;                                                                   /**< Size of the recieved SoftDevice. */
    uint32_t                 bl_size;                                                                   /**< Size of the recieved BootLoader. */
    uint32_t                 app_size;                                                                  /**< Size of the recieved Application. */
    uint32_t                 sd_image_start;                                                            /**< Location in flash where the received SoftDevice image is stored. */
    bool                     restart_into_bootloader;                                                   /**< If the chip must be reset and must reenter bootloader mode. */
} dfu_update_status_t;

/**@brief Update complete handler type. */
typedef void (*dfu_complete_handler_t)(dfu_update_status_t dfu_update_status);

#endif // DFU_TYPES_H__

/**@} */
