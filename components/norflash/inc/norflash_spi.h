#ifndef __NORFLASH_SPI_H__
#define __NORFLASH_SPI_H__
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 初始化SPI NOR Flash
 * 
 * 本函数负责初始化SPI总线和相关的GPIO端口，为后续的NOR Flash操作做准备。
 * 
 * @param 无
 * @return 返回0，表示初始化成功。
 */
int norflash_spi_init(void);

/**
 * @brief 控制SPI NorFlash的CS信号线
 * 
 * 本函数用于通过GPIO控制SPI NorFlash设备的片选信号线。根据输入参数决定是使能还是禁用该信号线。
 * 
 * @param enable 如果为真，则禁用SPI NorFlash的CS信号线（即选中设备）；如果为假，则启用CS信号线（即不选中设备）。
 * @return int 返回0，表示操作成功。
 */
int norflash_spi_cs(bool enable);

/**
 * @brief 通过SPI与norflash进行单字节数据传输
 * 
 * @param send_data 需要发送的数据，类型为uint8_t
 * @return 接收到的数据，类型为uint8_t
 */
uint8_t norflash_spi_transfer(uint8_t send_data);

#endif

