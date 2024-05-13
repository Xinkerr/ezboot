/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include <string.h>
#include <fal.h>

#include "ch32f20x.h"

static void td_lock_init(void)
{
}

static void td_lock(void)
{
}

static void td_unlock(void)
{
}

static int init(void)
{
    td_lock_init();
    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    size_t i;
    uint32_t addr = ch32f2_onchip_flash.addr + offset;
    td_lock();
    for (i = 0; i < size; i++, addr++, buf++)
    {
        *buf = *(uint8_t *) addr;
    }
    td_unlock();
    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    size_t i;
    uint32_t write_data;
    uint32_t read_data; 
    uint32_t addr = ch32f2_onchip_flash.addr + offset;
    if(size % 4)
    {
        return -4;
    }

    td_lock();
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP |FLASH_FLAG_WRPRTERR);
    for (i = 0; i < size; i+=4, buf+=4, addr+=4)
    {
        memcpy(&write_data, buf, 4); //用以保证FLASH_ProgramWord的第三个参数是内存首地址对齐
        /* write data */
        FLASH_ProgramWord(addr, write_data);
        read_data = *(uint32_t *) addr;
        /* check data */
        if (read_data != write_data)
        {
            FLASH_Lock();
            td_unlock();
            return -1;
        }
    }
    FLASH_Lock();
    td_unlock();

    return size;
}

static int erase(long offset, size_t size)
{
    FLASH_Status flash_status;
    size_t erased_size = 0;
    uint32_t addr = ch32f2_onchip_flash.addr + offset;
    
    /* start erase */
    td_lock();
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP |FLASH_FLAG_WRPRTERR);
    /* it will stop when erased size is greater than setting size */
    while (erased_size < size)
    {
        flash_status = FLASH_ErasePage(addr);
        if (flash_status != FLASH_COMPLETE)
        {
            log_e("FLASH_ErasePage err: %d\r\n", flash_status);
            FLASH_Lock();
            td_unlock();
            return -1;
        }

        erased_size += ch32f2_onchip_flash.blk_size;
        addr += ch32f2_onchip_flash.blk_size;
    }
    FLASH_Lock();
    td_unlock();

    return size;
}

const struct fal_flash_dev ch32f2_onchip_flash =
{
    .name       = "ch32_onchip",
    .addr       = 0x08000000,
    .len        = 128*1024,
    .blk_size   = 4096,
    .ops        = {init, read, write, erase},
    .write_gran = 32
};

