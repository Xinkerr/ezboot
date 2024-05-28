#ifndef __NORFLASH_H__
#define __NORFLASH_H__
#include <stdint.h>

int norflash_init(void);

int norflash_erase(uint32_t addr, uint32_t size);

int norflash_read(uint32_t addr, void* buf, uint32_t size);

int norflash_write(uint32_t addr, const void* buf, uint32_t size);

#endif
