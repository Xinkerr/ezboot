#include <mcu_header.h>
#include <stdio.h>
#include <ezboot_config.h>
#define LOG_UART1           1
#define LOG_UART2           2
#define LOG_UART3           3


#define USE_LOG_UART        LOG_UART1  

#define LOG_UART_BAUDRATE   CONFIG_LOG_UART_BAUDRATE         

#if USE_LOG_UART

UART_HandleTypeDef huart1;

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_8;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT|UART_ADVFEATURE_DMADISABLEONERROR_INIT;
  huart1.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  huart1.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
  HAL_UART_Init(&huart1);
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
	HAL_UART_Transmit(&huart1, (const uint8_t*)&data, 1, 0xffff);
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
	MX_USART1_UART_Init();
}

#else

void log_uart_init(void) {}

#endif //USE_LOG_UART
