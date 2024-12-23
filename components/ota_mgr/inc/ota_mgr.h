/**
 * SPDX-License-Identifier: Apache-2.0
 *
 * Disclaimer / 免责声明
 *
 * This software is provided "as is", without warranty of any kind, express or implied, 
 * including but not limited to the warranties of merchantability, fitness for a 
 * particular purpose, or non-infringement. In no event shall the authors or copyright 
 * holders be liable for any claim, damages, or other liability, whether in an action 
 * of contract, tort, or otherwise, arising from, out of, or in connection with the 
 * software or the use or other dealings in the software.
 *
 * 本软件按“原样”提供，不附带任何明示或暗示的担保，包括但不限于对适销性、特定用途适用性
 * 或非侵权的保证。在任何情况下，作者或版权持有人均不对因本软件或使用本软件而产生的任何
 * 索赔、损害或其他责任负责，无论是合同诉讼、侵权行为还是其他情况。
 */

#ifndef __OTA_MGR_H__
#define __OTA_MGR_H__
#include <stdint.h>

typedef enum
{
    OTA_NONE = 0,
    OTA_REQUEST,
    OTA_WRITE_COMPLETED,        //固件写入完成
    OTA_FW_OVER_SIZE,           //固件大小超过最大值
    OTA_FW_ILLEGAL,             //固件数据非法
    OTA_MRG_DATA_ERR,           //MGR数据错误
    OTA_IMAGE_CHECKSUM_ERR,     //固件校验错误
    OTA_STA_WR_ERROR,           //读写错误
    OTA_ENCRYPT_DATA_OVERFLOW,  //加密数据长度溢出
}ota_mgr_state_t;

typedef struct 
{
    ota_mgr_state_t ota_state;
    uint32_t check_sum;
}ota_mgr_data_t;

int ota_mgr_state_set(ota_mgr_state_t state);

ota_mgr_state_t ota_mgr_state_get(void);

int ota_mgr_image_hw_init(void);

int ota_mgr_image_erase(uint32_t addr, uint32_t size);

int ota_mgr_image_read(uint32_t addr, uint8_t *pdata, uint32_t size);

int ota_mgr_image_write(uint32_t addr, uint8_t *pdata, uint32_t size);

#if CONFIG_TEST
int ota_mgr_test(void);
#endif

#if OTA_IMAGE_EXTERN_FLASH
#include <norflash.h>

static int ota_mgr_image_hw_init(void)
{
    return norflash_init();
}

static inline int ota_mgr_image_erase(uint32_t addr, uint32_t size)
{
    return norflash_erase(addr, size);
}

static inline int ota_mgr_image_read(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    return norflash_read(addr, pdata, size);
}

static inline int ota_mgr_image_write(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    return norflash_write(addr, pdata, size);
}

#else
#include <ezb_flash.h>

static int ota_mgr_image_hw_init(void)
{
    return 0;
}

static inline int ota_mgr_image_erase(uint32_t addr, uint32_t size)
{
    return ezb_flash_erase(addr, size);
}

static inline int ota_mgr_image_read(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    return ezb_flash_read(addr, pdata, size);
}

static inline int ota_mgr_image_write(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    return ezb_flash_read(addr, pdata, size);
}
#endif

#endif
