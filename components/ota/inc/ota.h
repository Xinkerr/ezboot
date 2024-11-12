#ifndef __OTA_H__
#define __OTA_H__
#include <stdint.h>
#include <ezboot_config.h>

#define OTA_HEADER_SIZE             128

#define OTA_SUCCESS                  0
#define OTA_FLASH_ERR               -1
#define OTA_IMAGE_CHECK_ERR         -2
#define OTA_IMAGE_SIZE_ERR          -3
#define OTA_IMAGE_ENCRYPT_OVERFLOW  -4

typedef struct
{
    uint8_t head[4];
    uint32_t header_len;
    uint32_t data_version;
    uint32_t fw_version;
    uint32_t image_size;
    uint32_t crc;
    #if CONFIG_OTA_IMAGE_AES128_ENCRYPT
        uint32_t encrypt_len;
        uint8_t key_salt[16];
        uint8_t iv_salt[16];
    #endif
}ota_image_info_t;


/**
 * @brief 执行固件的OTA更新流程。
 * 
 * 该函数首先初始化FAL（Flash Abstraction Layer），然后从固件图像中获取头信息，
 * 校验固件图像的合法性，包括头信息校验、图像大小检查和数据完整性校验。
 * 若校验全部通过，则将新固件写入应用区域。
 * 
 * @return int 返回OTA更新过程中的错误码。成功时返回0，否则返回特定的错误码。
 */
int ota_firmware_update(void);

/**
 * @brief 擦除OTA图像分区
 * 
 * 这个函数用于擦除设备上的OTA图像分区的所有内容。
 * 
 * @return 返回擦除操作的结果状态码。成功>=0，失败返回错误码。
 */
int ota_image_erase(void);

#if CONFIG_TEST
int ota_flash_test(void);
#endif

#endif
