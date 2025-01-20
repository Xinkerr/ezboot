#include <stdint.h>
#include <tlv.h>

tlv_err_t tlv_parse(tlv_t* tlv, uint8_t* pdata, uint16_t pdata_size)
{
    int i;
    uint16_t count = tlv->tag_count;
    tlv_tb_t* tlv_table = tlv->tlv_table;

    if(pdata_size <= 3)
        return TLV_SIZE_SMALL;

    for(i=0; i<count; i++)
    {
        if(pdata[0] == tlv_table[i].tag)
        {
            uint16_t len = *(uint16_t*)(pdata+1);
            if(len > pdata_size)
                return TLV_DATA_INCOMPLETE;
            
            if(tlv_table[i].handler(pdata+3, len) == 0)
                return TLV_OK;
            else
                return TLV_HANDLE_ERROR;
        }
    }
    return TLV_TAG_NOT_FOUND;
}