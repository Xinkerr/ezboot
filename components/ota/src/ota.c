#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ota.h>
#include <fal.h>
#include <fw_partition.h>
#include <mlog.h>

#define WRITE_BLOCK         4096

static uint8_t cache_buf[WRITE_BLOCK];

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
    const struct fal_partition *image_pt;
    int ret;
    image_pt = fal_partition_find("ota_image");
    if(image_pt == NULL)
    {
        mlog_e("ota_image partition not found");
        return -1;
    }
    ret = fal_partition_read(image_pt, 0, cache_buf, OTA_HEADER_SIZE);
    if(ret < 0)
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
    const struct fal_partition *image_pt;

    image_pt = fal_partition_find("ota_image");

    remaining = info->image_size;

    //读取所有image数据，并进行分块CRC计算
    while (remaining)
    {
        if(remaining >= WRITE_BLOCK)
        {
            fal_partition_read(image_pt, offset+OTA_HEADER_SIZE, cache_buf, WRITE_BLOCK);
            crc_value = crc32(crc_value, cache_buf, WRITE_BLOCK, false);
            remaining = remaining - WRITE_BLOCK;
            offset += WRITE_BLOCK;
        }
        else
        {
            fal_partition_read(image_pt, offset+OTA_HEADER_SIZE, cache_buf, remaining);
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
    const struct fal_partition *app_pt, *image_pt;

    app_pt = fal_partition_find("app");
    image_pt = fal_partition_find("ota_image");
    if(app_pt == NULL || image_pt == NULL)
    {
        mlog_e("app or ota_image partition not found");
        return OTA_FLASH_ERR;
    }

    //擦除APP区域
    mlog("erase app...");
    ret = fal_partition_erase_all(app_pt);
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
            ret = fal_partition_read(image_pt, offset+OTA_HEADER_SIZE, cache_buf, WRITE_BLOCK);
            if(ret < 0)
            {
                mlog_e("read ota_image partition error, 0x%x", offset+OTA_HEADER_SIZE);
                return OTA_FLASH_ERR;
            }
            ret = fal_partition_write(app_pt, offset, cache_buf, WRITE_BLOCK);
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
            ret = fal_partition_read(image_pt, offset+OTA_HEADER_SIZE, cache_buf, remaining);
            if(ret < 0)
            {
                mlog_e("read ota_image partition error, 0x%x", offset+OTA_HEADER_SIZE);
                return OTA_FLASH_ERR;
            }
            ret = fal_partition_write(app_pt, offset, cache_buf, remaining);
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

    fal_init();

    image_header_get(&image_info);
    mlog("\r\n");
    mlog_d("[head]: 0x%08x", image_info.head);
    mlog_d("[header_len]: %u", image_info.header_len);
    mlog_d("[data_version]: 0x%08x", image_info.data_version);
    mlog_d("[image_size]: %u", image_info.image_size);
    mlog_d("[crc]: 0x%08x", image_info.crc);
    mlog_d("[random]: 0x%08x", image_info.random);

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
 * @return 返回擦除操作的结果状态码。成功>=0，失败返回错误码。
 */
int ota_image_erase(void)
{
    return fal_partition_erase_all(fal_partition_find("ota_image"));
}
