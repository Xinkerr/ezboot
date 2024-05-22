#include <ch32f20x.h>
#include <stdio.h>
#include <ezboot_config.h>
#define LOG_UART1           1
#define LOG_UART2           2
#define LOG_UART3           3


#define USE_LOG_UART        LOG_UART1  

#define LOG_UART_BAUDRATE   CONFIG_LOG_UART_BAUDRATE         

#if USE_LOG_UART

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
#if (USE_LOG_UART == LOG_UART1)
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	USART_SendData(USART1, (u8) data);
#elif (USE_LOG_UART == LOG_UART2)
  while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
  USART_SendData(USART2, (u8) data);	
#elif (USE_LOG_UART == LOG_UART3)
  while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);	
  USART_SendData(USART3, (u8) data);	
#endif
	
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
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
	
#if (USE_LOG_UART == LOG_UART1)	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
#elif (USE_LOG_UART == LOG_UART2)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
#elif (USE_LOG_UART == LOG_UART3)	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
#endif	
   
  USART_InitStructure.USART_BaudRate = LOG_UART_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx;

#if (USE_LOG_UART == LOG_UART1)	
  USART_Init(USART1, &USART_InitStructure); 
  USART_Cmd(USART1, ENABLE);  
	
#elif (USE_LOG_UART == LOG_UART2)	
  USART_Init(USART2, &USART_InitStructure); 
  USART_Cmd(USART2, ENABLE);
	
#elif (USE_LOG_UART == LOG_UART3)
  USART_Init(USART3, &USART_InitStructure); 
  USART_Cmd(USART3, ENABLE);
	
#endif	
}

#else

void log_uart_init(void) {}

#endif //USE_LOG_UART
