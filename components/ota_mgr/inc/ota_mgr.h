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
    OTA_STA_WR_ERROR            //读写错误
}ota_mgr_state_t;

typedef struct 
{
    ota_mgr_state_t ota_state;
    uint32_t check_sum;
}ota_mgr_data_t;

int ota_mgr_state_set(ota_mgr_state_t state);

ota_mgr_state_t ota_mgr_state_get(void);

#endif
