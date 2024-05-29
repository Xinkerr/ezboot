#ifndef __EZB_FLASH_H__
#define __EZB_FLASH_H__

#include <stdint.h>

int ezb_flash_erase(uint32_t addr, uint32_t size);

int ezb_flash_write(uint32_t addr, uint8_t *pdata, uint32_t size);

int ezb_flash_read(uint32_t addr, uint8_t *pdata, uint32_t size);

#endif
