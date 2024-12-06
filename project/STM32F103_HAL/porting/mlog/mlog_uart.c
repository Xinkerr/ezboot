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

#include <mcu_header.h>
#include <stdio.h>
#include <stdint.h>
#include <ezboot_config.h>

#define LOG_UART_BAUDRATE   CONFIG_LOG_UART_BAUDRATE         

#if CONFIG_LOG_LEVEL

static UART_HandleTypeDef* uart_handle = NULL;



/* Support Printf Function Definition */
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;

/*********************************************************************
 * @fn      fputc
 *
 * @brief   Support Printf Function 
 *
 * @param   data - UART send Data.
 *
 * @return  data - UART send Data.
 */
int fputc(int data, FILE *f)
{
	HAL_UART_Transmit(uart_handle, (const uint8_t*)&data, 1, 0xffff);
	return data;
}


/*********************************************************************
 * @fn      log_uart_init
 *
 * @brief   Initializes the USARTx peripheral.
 *
 * @return  None
 */
void log_uart_init(void)
{
	uart_handle = &huart2;
}

#else

void log_uart_init(void) {}

#endif //USE_LOG_UART
