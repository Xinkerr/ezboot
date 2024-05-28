#include <string.h>
#include <ezboot_config.h>
#include <ota_mgr.h>
#include <ezb_flash.h>
#include <mlog.h>

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
    ezb_flash_erase(OTA_MGR_DATA_ADDRESS, OTA_MGR_REGION_SIZE);
    ezb_flash_write(OTA_MGR_DATA_ADDRESS, (uint8_t*)&data, sizeof(ota_mgr_data_t));
    return 0;
}

ota_mgr_state_t ota_mgr_state_get(void)
{
    ota_mgr_data_t data;
    ezb_flash_read(OTA_MGR_DATA_ADDRESS, (uint8_t*)&data, sizeof(ota_mgr_data_t));
    uint32_t adler32 = calculate_adler32((const uint8_t *)&data, sizeof(data)-4);
    mlog_hex_d("mgr data: ", &data, sizeof(data));
    mlog_d("adler32=0x%08x", adler32);
    mlog_d("check_sum=0x%08x", data.check_sum);
    if(adler32 == data.check_sum)
    {
        return data.ota_state;
    }
    else
    {
        return OTA_MRG_DATA_ERR;
    }
}
