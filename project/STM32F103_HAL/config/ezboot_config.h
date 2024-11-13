#ifndef __BOOT_CONFIG_H__
#define __BOOT_CONFIG_H__

//================================================================COMMON================================================================================
//OTA image
#define CONFIG_OTA_IMAGE_AES128_ENCRYPT     1
#define CONFIG_OTA_WRITE_BLOCK_MAX          1024
#define CONFIG_OTA_IMAGE_AES_KEY            {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10}
#define CONFIG_OTA_IMAGE_AES_IV             {0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}

//NORFLASH config
// #define CONFIG_NORFLASH_CMD_SECTOR_ERASE    0x20
// #define CONFIG_NORFLASH_ERASE_SIZE          4096
// #define CONFIG_NORFLASH_PAGE_SIZE           256

//---------------------------Partition-------------------------------
//bootloader
#define BOOT_ADDRESS                        ((uint32_t)(0x08000000))

//OTA image on chip
// // application
// #define APP_ADDRESS                           ((uint32_t)(0x08003800))
// #define APP_REGION_SIZE                       ((uint32_t)(58*1024))
// // OTA manager
// #define OTA_MGR_EXTERN_FLASH                  0
// #define OTA_MGR_REGION_SIZE                   ((uint32_t)0x800)
// #define OTA_MGR_DATA_ADDRESS                  (APP_ADDRESS-OTA_MGR_REGION_SIZE)
// //OTA image
// #define OTA_IMAGE_EXTERN_FLASH                0
// #define OTA_IMAGE_ADDRESS                     ((uint32_t)(APP_ADDRESS+APP_REGION_SIZE))
// #define OTA_IMAGE_REGION_SIZE                 (APP_REGION_SIZE)


//OTA image on extern flash
// application
 #define APP_ADDRESS                            ((uint32_t)(0x08003800))
 #define APP_REGION_SIZE                        ((uint32_t)114*1024)
// OTA manager
#define OTA_MGR_EXTERN_FLASH                    1
#define OTA_MGR_DATA_ADDRESS                    ((uint32_t)(0*1024))
#define OTA_MGR_REGION_SIZE                     ((uint32_t)0x800)
// OTA image
 #define OTA_IMAGE_EXTERN_FLASH                 1
 #define OTA_IMAGE_ADDRESS                      ((uint32_t)(4*1024))
 #define OTA_IMAGE_REGION_SIZE                  ((uint32_t)114*1024)


//=================================================================BOOT=================================================================================
//LOG UART
#define CONFIG_LOG_UART_BAUDRATE            115200
#define CONFIG_LOG_LEVEL                    3 //LEVEL INF
#define CONFIG_TEST							0

#endif
