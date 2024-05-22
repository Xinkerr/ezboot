#ifndef __FW_PARTITION_H__
#define __FW_PARTITION_H__

//bootloader address
#define BOOT_ADDRESS            ((uint32_t)(0x08000000))

// OTA manager data address
#define OTA_MGR_DATA_ADDRESS    ((uint32_t)(0x08008000)-0x1000)

// application address
#define APP_ADDRESS             ((uint32_t)(0x08008000))

#define APP_REGION_SIZE         ((uint32_t)96*1024)



#define OTA_IMAGE_EXTERN_FLASH  1
#define OTA_IMAGE_ADDRESS       ((uint32_t)(1024*1024))

#endif
