#include <stdint.h>
#include <string.h>
#include <tlv.h>
#include <ezboot_config.h>

#define TAG_SIZE        1
#define LEN_SIZE        2

int tlv_parse(tlv_t* tlv, uint8_t* pdata, uint16_t pdata_size)
{
    int i;
    uint16_t count = tlv->tag_count;
    tlv_tb_t* tlv_table = tlv->tlv_table;

    if(pdata_size < (TAG_SIZE+LEN_SIZE))
        return TLV_SIZE_SMALL;

    for(i=0; i<count; i++)
    {
        if(pdata[0] == tlv_table[i].tag)
        {
            tlv_len_t len;
            memcpy(&len, pdata + TAG_SIZE, sizeof(tlv_len_t));
            if(len > pdata_size-(TAG_SIZE+LEN_SIZE))
                return TLV_DATA_INCOMPLETE;
            
            if(tlv_table[i].handler(pdata+(TAG_SIZE+LEN_SIZE), len) == 0)
                return len + (TAG_SIZE+LEN_SIZE);
            else
                return TLV_HANDLE_ERROR;
        }
    }
    return TLV_TAG_NOT_FOUND;
}

uint16_t tlv_add(uint8_t* buf, uint8_t tag, uint16_t len, void* pdata)
{
    buf[0] = tag;
    memcpy(buf+1, &len, 2);
    memcpy(buf+3, pdata, len);

    return len+3;
}
