/**
 * 
 * Copyright (c) 2025 Xinkerr
 * 
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

#ifndef __PG_UART_DRV_H__
#define __PG_UART_DRV_H__

#include <ezboot_config.h>

#if CONFIG_PROGRAMMING_SUPPORT && CONFIG_PG_UART_USE
#include <pg_uart.h>

/**
 * @brief 初始化PG UART驱动
 * 
 * 该函数用于初始化UART驱动，配置相关硬件资源并准备UART通讯。
 * 
 * @return 返回0表示初始化成功，非0值表示初始化失败。
 */
int pg_uart_drv_init(void);

/**
 * @brief 发送数据通过PG UART
 * 
 * 该函数用于将数据通过UART发送出去。数据由`pdata`指向，长度由`len`指定。
 * 请确保`pdata`指向有效数据，`len`为数据的字节长度。
 * 
 * @param pdata 数据指针，指向需要发送的数据。
 * @param len 数据的长度，单位为字节。
 */
void pg_uart_drv_send(const void* pdata, uint16_t len);

/**
 * @brief 接收数据通过PG UART
 * 
 * 该函数用于接收UART数据。接收到的数据存储在`pdata`指向的缓冲区中。
 * 请确保`pdata`指向足够大的缓冲区，以存储接收的数据。
 * 
 * @param pdata 用于接收数据的缓冲区指针。
 * @param len 要接收的数据的长度，单位为字节。
 * 
 * @return    实际获取数据的长度
 */
uint16_t pg_uart_drv_get(uint8_t* pdata, uint16_t len);

#endif

#endif
