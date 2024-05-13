#include <ch32f20x.h>
#include <string.h>
#include <fw_partition.h>
#include <ota_mgr.h>

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
	int i;
    int ret;
    uint32_t* p;
    ota_mgr_data_t data;
    data.ota_state = state;
    data.check_sum = calculate_adler32((const uint8_t *)&data, sizeof(data)-4);
    p = (uint32_t*)&data;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP |FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(OTA_MGR_DATA_ADDRESS);
    for(i=0; i<sizeof(ota_mgr_data_t); i +=4)
    {
        ret = FLASH_ProgramWord(OTA_MGR_DATA_ADDRESS+i, p[i]);
        if(ret != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return ret;
        }
    }
    FLASH_Lock();
    return 0;
}

ota_mgr_state_t ota_mgr_state_get(void)
{
    ota_mgr_data_t data;
    memcpy(&data, (const void*)OTA_MGR_DATA_ADDRESS, sizeof(ota_mgr_data_t));
    if(calculate_adler32((const uint8_t *)&data, sizeof(data)-4))
    {
        return data.ota_state;
    }
    else
    {
        return OTA_MRG_DATA_ERR;
    }
}
