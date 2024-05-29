#ifndef __NORFLASH_H__
#define __NORFLASH_H__
#include <stdint.h>

/**
 * @brief 初始化NORFLASH设备
 * 
 * 本函数负责初始化SPI总线并验证NORFLASH设备的存在。它首先初始化用于与NORFLASH通信的SPI总线，
 * 然后读取NORFLASH的设备ID，并记录该ID以供调试使用。
 * 
 * @return 返回0表示初始化成功；返回-1表示SPI总线初始化失败；返回-2表示无法读取到有效的NORFLASH设备ID。
 */
int norflash_init(void);

/**
 * @brief 擦除NORFLASH指定区域
 * 
 * @param addr 擦除操作的起始地址
 * @param size 擦除的大小，以字节为单位
 * @return int 操作成功返回0，失败返回非0值
 */
int norflash_erase(uint32_t addr, uint32_t size);

/**
 * @brief 从NORFLASH中读取数据。
 * 
 * @param addr 要读取数据的起始地址。
 * @param buf 用于存储读取数据的缓冲区指针。
 * @param size 要读取的数据大小，以字节为单位。
 * @return 0，表示成功。
 */
int norflash_read(uint32_t addr, void* buf, uint32_t size);

/**
 * @brief 将数据写入NORFLASH。
 *
 * 此函数将缓冲区中的数据以页为单位写入NORFLASH。如果数据大小不是页大小的倍数，
 * 则会在最后一个页中写入剩余数据。
 *
 * @param addr 要开始写入的NORFLASH地址。
 * @param buf 待写入数据的缓冲区指针。
 * @param size 从缓冲区中要写入的数据字节数。
 * @return int 0，表示成功。
 */
int norflash_write(uint32_t addr, const void* buf, uint32_t size);

#endif
