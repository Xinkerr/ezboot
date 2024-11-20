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
#include <ezboot_config.h>
#define LOG_UART1           1
#define LOG_UART2           2
#define LOG_UART3           3


#define LOG_UART_BAUDRATE   CONFIG_LOG_UART_BAUDRATE         

UART_HandleTypeDef log_huart;

/* USART init function */

void MX_USART_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  log_huart.Instance = USART2;
  log_huart.Init.BaudRate = 115200;
  log_huart.Init.WordLength = UART_WORDLENGTH_8B;
  log_huart.Init.StopBits = UART_STOPBITS_1;
  log_huart.Init.Parity = UART_PARITY_NONE;
  log_huart.Init.Mode = UART_MODE_TX;
  log_huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  log_huart.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&log_huart);
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}



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
	HAL_UART_Transmit(&log_huart, (const uint8_t*)&data, 1, 0xffff);
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
	HAL_UART_MspInit(&log_huart);
	MX_USART_UART_Init();
}

