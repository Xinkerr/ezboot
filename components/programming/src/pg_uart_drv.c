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

#include <stdint.h>
#include <mcu_header.h>
#include <ezboot_config.h>
#include <pg_uart_drv.h>
#include <ringbuffer.h>

ringbuffer_t pg_recv_rb;
uint8_t pg_recv_buf[CONFIG_PG_UART_RB_SIZE];

void pg_uart_recv_callback(const void* buf, uint16_t len)
{
    ringbuffer_put(&pg_recv_rb, (uint8_t*)buf, len);
}

inline int pg_uart_drv_init(void)
{
    ringbuffer_init(&pg_recv_rb, pg_recv_buf, sizeof(pg_recv_buf));
    pg_uart_register_rx_callback(pg_uart_recv_callback);
    return pg_uart_init();
}

inline void pg_uart_drv_send(const void* pdata, uint16_t len)
{
    pg_uart_send(pdata, len);
}

inline uint16_t pg_uart_drv_get(uint8_t* pdata, uint16_t len)
{
    return ringbuffer_get(&pg_recv_rb, pdata, len);
}
