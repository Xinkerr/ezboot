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

#include <string.h>
#include <ezboot_config.h>
#include <ota_mgr.h>

#if CONFIG_LOG_LEVEL
#include <mlog.h>
#define otamgr_log_hex_d    mlog_hex_d
#define otamgr_log_d        mlog_d
#else
#define otamgr_log_hex_d(...)
#define otamgr_log_d(...)
#endif

#if OTA_MGR_EXTERN_FLASH
#include <norflash.h>
static inline int ota_mgr_erase(uint32_t addr, uint32_t size)
{
    return norflash_erase(addr, size);
}

static inline int ota_mgr_write(uint32_t addr, const uint8_t *pdata, uint32_t size)
{
    return norflash_write(addr, pdata, size);
}

static inline int ota_mgr_read(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    return norflash_read(addr, pdata, size);
}

#else
#include <ezb_flash.h>
static inline int ota_mgr_erase(uint32_t addr, uint32_t size)
{
    return ezb_flash_erase(addr, size);
}

static inline int ota_mgr_write(uint32_t addr, const uint8_t *pdata, uint32_t size)
{
    return ezb_flash_write(addr, pdata, size);
}

static inline int ota_mgr_read(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    return ezb_flash_read(addr, pdata, size);
}
#endif

// Adler-32 算法
uint32_t calculate_adler32(const uint8_t *data, size_t len) {
    uint32_t a = 1, b = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        a = (a + data[i]) % 65521;
        b = (b + a) % 65521;
    }

    return (b << 16) | a;
}

int ota_mgr_state_set(ota_mgr_state_t state)
{
    ota_mgr_data_t data;
    data.ota_state = state;
    data.check_sum = calculate_adler32((const uint8_t *)&data, sizeof(data)-4);
    ota_mgr_erase(OTA_MGR_DATA_ADDRESS, OTA_MGR_REGION_SIZE);
    ota_mgr_write(OTA_MGR_DATA_ADDRESS, (uint8_t*)&data, sizeof(ota_mgr_data_t));
    return 0;
}

ota_mgr_state_t ota_mgr_state_get(void)
{
    ota_mgr_data_t data;
    ota_mgr_read(OTA_MGR_DATA_ADDRESS, (uint8_t*)&data, sizeof(ota_mgr_data_t));
    uint32_t adler32 = calculate_adler32((const uint8_t *)&data, sizeof(data)-4);
    otamgr_log_hex_d("mgr data: ", &data, sizeof(data));
    otamgr_log_d("adler32=0x%08x", adler32);
    otamgr_log_d("check_sum=0x%08x", data.check_sum);
    if(adler32 == data.check_sum)
    {
        return data.ota_state;
    }
    else
    {
        return OTA_MRG_DATA_ERR;
    }
}

#if CONFIG_TEST
int ota_mgr_test(void)
{
    int ret = 0;
    mlog("[TEST]: ota_mgr checking ......");
    uint8_t read_buf[8];
    const uint8_t test_buf[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};

    memset(read_buf, 0, sizeof(read_buf));
    ota_mgr_erase(OTA_MGR_DATA_ADDRESS, OTA_MGR_REGION_SIZE);
    ota_mgr_write(OTA_MGR_DATA_ADDRESS, test_buf, sizeof(test_buf));
    ota_mgr_read(OTA_MGR_DATA_ADDRESS, read_buf, sizeof(test_buf));
    if(memcmp(test_buf, read_buf, sizeof(test_buf)) != 0)
    {
        ret = -1;
        goto error;
    }

    memset(read_buf, 0, sizeof(read_buf));
    ota_mgr_erase(OTA_MGR_DATA_ADDRESS, OTA_MGR_REGION_SIZE);
    ota_mgr_read(OTA_MGR_DATA_ADDRESS, read_buf, sizeof(test_buf));
    if(memcmp(test_buf, read_buf, sizeof(test_buf)) == 0)
    {
        ret = -2;
        goto error;
    }

    mlog("OK\r\n");
    return ret;

    error:
        mlog("FAIL %d\r\n", ret);
        return ret;
}
#endif
