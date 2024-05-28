#ifndef __NORFLASH_SPI_H__
#define __NORFLASH_SPI_H__
#include <stdint.h>
#include <stdbool.h>

int norflash_spi_init(void);

int norflash_spi_cs(bool enable);

uint8_t norflash_spi_transfer(uint8_t send_data);

#endif

