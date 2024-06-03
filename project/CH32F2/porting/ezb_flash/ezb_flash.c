#include <ezb_flash.h>
#include <string.h>
#include "ch32f20x.h"
#include <mlog.h>

#define FLASH_SECTOR_SIZE       4096
#define WORD_ALIGNMENT          4

int ezb_flash_erase(uint32_t addr, uint32_t size)
{
    FLASH_Status flash_status;
    uint32_t erased_size = 0;
    
    /* start erase */
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP |FLASH_FLAG_WRPRTERR);
    /* it will stop when erased size is greater than setting size */
    while (erased_size < size)
    {
        flash_status = FLASH_ErasePage(addr);
        if (flash_status != FLASH_COMPLETE)
        {
            mlog_e("FLASH_ErasePage err: %d\r\n", flash_status);
            FLASH_Lock();
            return -1;
        }

        erased_size += FLASH_SECTOR_SIZE;
        addr += FLASH_SECTOR_SIZE;
    }
    FLASH_Lock();

    return 0;
}

int ezb_flash_write(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    int i;
    uint32_t write_data;
    uint32_t read_data; 
    if(size % WORD_ALIGNMENT)
    {
        return -4;
    }

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP |FLASH_FLAG_WRPRTERR);
    for (i = 0; i < size; i+=WORD_ALIGNMENT, pdata+=WORD_ALIGNMENT, addr+=WORD_ALIGNMENT)
    {
        memcpy(&write_data, pdata, WORD_ALIGNMENT); //用以保证FLASH_ProgramWord的第三个参数是内存首地址对齐
        /* write data */
        FLASH_ProgramWord(addr, write_data);
        read_data = *(uint32_t *) addr;
        /* check data */
        if (read_data != write_data)
        {
            FLASH_Lock();
            return -1;
        }
    }
    FLASH_Lock();

    return 0;
}

int ezb_flash_read(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    int i;
    for (i = 0; i < size; i++, addr++, pdata++)
    {
        *pdata = *(uint8_t *) addr;
    }
    return 0;
}
