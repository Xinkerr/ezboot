/**
 * SPDX-License-Identifier: Apache-2.0
 *
 * Disclaimer / 免责声明
 *
 * This software is provided "as is", without warranty of any kind, express or implied, 
 * including but not limited to the warranties of merchantability, fitness for a 
 * particular purpose, or non-infringement. In no event shall the authors or copyright 
 * holders be liable for any claim, damages, or other liability, whether in an action 
 * of contract, tort, or otherwise, arising from, out of, or in connection with the 
 * software or the use or other dealings in the software.
 *
 * 本软件按“原样”提供，不附带任何明示或暗示的担保，包括但不限于对适销性、特定用途适用性
 * 或非侵权的保证。在任何情况下，作者或版权持有人均不对因本软件或使用本软件而产生的任何
 * 索赔、损害或其他责任负责，无论是合同诉讼、侵权行为还是其他情况。
 */

#ifndef __NORFLASH_H__
#define __NORFLASH_H__
#include <stdint.h>
#include <ezboot_config.h>

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

#if CONFIG_TEST
int norflash_test(void);
#endif

#endif
