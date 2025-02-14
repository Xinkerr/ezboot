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
#include <pg_uart.h>

static UART_HandleTypeDef* uart_handle = NULL;

static recv_callback_t recv_callback = NULL;

static uint8_t uart_rx_buffer[1];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(recv_callback)
        recv_callback(uart_rx_buffer, 1);
    // 重新启用接收中断
    HAL_UART_Receive_IT(huart, uart_rx_buffer, 1);
}

inline int pg_uart_init(void)
{
    uart_handle = &huart2;
    HAL_UART_Receive_IT(uart_handle, uart_rx_buffer, 1);
    return 0;
}

inline void pg_uart_send(const void* buf, uint16_t len)
{
    HAL_UART_Transmit(uart_handle, (const uint8_t*)buf, len, 0xffff);
}

inline void pg_uart_register_rx_callback(recv_callback_t callback)
{
    recv_callback = callback;
}
