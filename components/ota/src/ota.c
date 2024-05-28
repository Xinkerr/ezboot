#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ota.h>
#include <norflash.h>
#include <ezb_flash.h>
#include <image_decrypt.h>
#include <mlog.h>
#include <ezboot_config.h>

//AES128加密开启时，该值必须是128bits整数倍
#define WRITE_BLOCK         4096

static uint8_t cache_buf[WRITE_BLOCK];
#if CONFIG_OTA_IMAGE_AES128_ENCRYPT
static uint8_t write_buf[WRITE_BLOCK];
#endif

/**
 * 对长度进行对齐操作。
 * 
 * 该函数将给定的长度按照指定的对齐大小进行对齐，确保返回的长度满足对齐要求。
 * 如果给定的长度已经满足对齐要求，则直接返回该长度；如果不满足，则调整长度使其满足对齐要求。
 * 
 * @param length 需要进行对齐操作的原始长度。
 * @param align_to 对齐的目标大小，即要求的对齐边界。
 * @return 对齐后的长度。该长度要么与原始长度相同（如果原始长度已经满足对齐要求），要么是满足对齐要求的下一个较大长度。
 */
uint16_t align_length(uint16_t length, uint16_t align_to) {
    // 检查原始长度是否已经满足对齐要求
    if (length % align_to == 0) {
        return length;
    } else {
        // 计算并返回满足对齐要求的下一个长度
        return length + (align_to - (length % align_to));
    }
}

static uint32_t crc32(uint32_t initial_value, const void *data, size_t length, bool eof) 
{
    const uint8_t* pdata = (const uint8_t*)data;
    uint32_t crc = initial_value;
    for (size_t i = 0; i < length; i++) 
    {
        crc ^= pdata[i];
        for (int j = 0; j < 8; j++) 
        {
            if (crc & 1) 
            {
                crc = (crc >> 1) ^ 0xEDB88320;
            } 
            else 
            {
                crc >>= 1;
            }
        }
    }
    if(eof)
        return ~crc;
    else
        return crc;
}

static int image_header_get(ota_image_info_t* info)
{
    int ret;
    
    ret = norflash_read(OTA_IMAGE_ADDRESS, cache_buf, OTA_HEADER_SIZE);
    if(ret != 0)
    {
        mlog_e("read ota_image partition error");
        return -1;
    }
    memcpy(info, cache_buf, sizeof(ota_image_info_t));

    return 0;
}

static bool image_data_check(const ota_image_info_t* info)
{
    uint32_t offset = 0;
    uint32_t remaining;
    uint32_t crc_value = 0xFFFFFFFF;

    #if CONFIG_OTA_IMAGE_AES128_ENCRYPT
        remaining = info->encrypt_len;
    #else
        remaining = info->image_size;
    #endif

    //读取所有image数据，并进行分块CRC计算
    while (remaining)
    {
        if(remaining >= WRITE_BLOCK)
        {
            norflash_read(OTA_IMAGE_ADDRESS+offset+OTA_HEADER_SIZE, cache_buf, WRITE_BLOCK);
            crc_value = crc32(crc_value, cache_buf, WRITE_BLOCK, false);
            remaining = remaining - WRITE_BLOCK;
            offset += WRITE_BLOCK;
        }
        else
        {
            norflash_read(OTA_IMAGE_ADDRESS+offset+OTA_HEADER_SIZE, cache_buf, remaining);
            crc_value = crc32(crc_value, cache_buf, remaining, true);
            offset += remaining;
            remaining = 0;
        }
    }
    //CRC校验对比
    if(memcmp(&info->crc, &crc_value, 4) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


static int image_write_app(const ota_image_info_t* info)
{
    int ret;
    uint32_t offset = 0;
    uint32_t remaining;

    #if CONFIG_OTA_IMAGE_AES128_ENCRYPT
        image_decrypt_init(info->key_salt, sizeof(info->key_salt), 
                            info->iv_salt, sizeof(info->iv_salt));
    #endif

    //擦除APP区域
    mlog("erase app...");
    ret = ezb_flash_erase(APP_ADDRESS, APP_REGION_SIZE);
    if(ret < 0)
    {
        mlog("....FAIL\r\n");
        return OTA_FLASH_ERR;
    }
    mlog("....OK\r\n");
    remaining = info->image_size - offset;

    //将Image数据写入到APP区域中
    mlog("writing...\r");
    while (remaining)
    {
        if(remaining >= WRITE_BLOCK)
        {
            ret = norflash_read(OTA_IMAGE_ADDRESS+offset+OTA_HEADER_SIZE, cache_buf, WRITE_BLOCK);
            if(ret != 0)
            {
                mlog_e("read ota_image partition error, 0x%x", offset+OTA_HEADER_SIZE);
                return OTA_FLASH_ERR;
            }
            #if CONFIG_OTA_IMAGE_AES128_ENCRYPT
                image_decrypt_data(cache_buf, WRITE_BLOCK, write_buf);
                ret = ezb_flash_write(APP_ADDRESS+offset, write_buf, WRITE_BLOCK);
            #else
                ret = ezb_flash_write(APP_ADDRESS+offset, cache_buf, WRITE_BLOCK);
            #endif
            if(ret < 0)
            {
                mlog_e("write app partition error, 0x%x", offset);
                return OTA_FLASH_ERR;
            }
            // mlog_hex_d("hex: ", cache_buf, WRITE_BLOCK);
            remaining = remaining - WRITE_BLOCK;
            offset += WRITE_BLOCK;
        }
        else
        {
            #if CONFIG_OTA_IMAGE_AES128_ENCRYPT
                ret = norflash_read(OTA_IMAGE_ADDRESS+offset+OTA_HEADER_SIZE, cache_buf, align_length((uint16_t)remaining,16));
                if(ret != 0)
                {
                    mlog_e("read ota_image partition error, 0x%x", offset+OTA_HEADER_SIZE);
                    return OTA_FLASH_ERR;
                }
                image_decrypt_data(cache_buf, align_length((uint16_t)remaining,16), write_buf);
                ret = ezb_flash_write(APP_ADDRESS+offset, write_buf, remaining);
            #else
                ret = norflash_read(OTA_IMAGE_ADDRESS+offset+OTA_HEADER_SIZE, cache_buf, remaining);
                if(ret < 0)
                {
                    mlog_e("read ota_image partition error, 0x%x", offset+OTA_HEADER_SIZE);
                    return OTA_FLASH_ERR;
                }
                ret = ezb_flash_write(APP_ADDRESS+offset, cache_buf, remaining);
            #endif
            if(ret < 0)
            {
                mlog_e("write app partition error, 0x%x", offset);
                return OTA_FLASH_ERR;
            }
            // mlog_hex_d("hex: ", cache_buf, remaining);
            offset += remaining;
            remaining = 0;
        }
        mlog("writing......%u%%\r", offset*100/info->image_size);
    }
    mlog("\r\n");

    return OTA_SUCCESS;
}

/**
 * @brief 执行固件的OTA更新流程。
 * 
 * 该函数首先初始化FAL（Flash Abstraction Layer），然后从固件图像中获取头信息，
 * 校验固件图像的合法性，包括头信息校验、图像大小检查和数据完整性校验。
 * 若校验全部通过，则将新固件写入应用区域。
 * 
 * @return int 返回OTA更新过程中的错误码。成功时返回0，否则返回特定的错误码。
 */
int ota_firmware_update(void)
{
    static ota_image_info_t image_info;

    norflash_init();

    image_header_get(&image_info);
    mlog("\r\n");
    mlog_i("[head]: 0x%02x%02x%02x%02x", image_info.head[0],
		image_info.head[1], image_info.head[2], image_info.head[3]);
    mlog_i("[header_len]: %u", image_info.header_len);
    mlog_i("[data_version]: 0x%08x", image_info.data_version);
    mlog_i("[image_size]: %u", image_info.image_size);
    mlog_i("[crc]: 0x%08x", image_info.crc);
    #if CONFIG_OTA_IMAGE_AES128_ENCRYPT
        mlog_d("[encrypt_len]: %u", image_info.encrypt_len);
        mlog_hex_d("[key salt]: ", image_info.key_salt, sizeof(image_info.key_salt));
        mlog_hex_d("[iv salt]: ", image_info.iv_salt, sizeof(image_info.iv_salt));
    #endif

    mlog("checking head...");
    if(memcmp(image_info.head, "OTAB", sizeof(image_info.head)) == 0)
    {
        mlog("...OK\r\n");
    }
    else
    {
        mlog("...FAIL\r\n");
        return OTA_FLASH_ERR;
    }

    mlog("checking image size...");
    if(image_info.image_size <= APP_REGION_SIZE)
    {
        mlog("...OK\r\n");
    }
    else
    {
        mlog("...FAIL\r\n");
        return OTA_IMAGE_SIZE_ERR;
    }

    mlog("checking data...");
    if(image_data_check(&image_info))
    {
        mlog("...OK\r\n");
    }
    else
    {
        mlog("...FAIL\r\n");
        return OTA_IMAGE_CHECK_ERR;
    }

    return image_write_app(&image_info);
}

/**
 * @brief 擦除OTA图像分区
 * 
 * 这个函数用于擦除设备上的OTA图像分区。它通过查找名为"ota_image"的分区，并擦除该分区的所有内容。
 * 
 * @return 返回擦除操作的结果状态码。成功=0，失败返回错误码。
 */
int ota_image_erase(void)
{
    int ret = norflash_erase(OTA_IMAGE_ADDRESS, OTA_IMAGE_REGION_SIZE);
    if (ret == 0)
        return 0;
    else
        return -1;
}
