/**
 * 
 * Copyright (c) 2025 Xinkerr
 * 
 * SPDX-License-Identifier: Apache-2.0
 *
 * Disclaimer / ��������
 *
 * This software is provided "as is", without warranty of any kind, express or implied, 
 * including but not limited to the warranties of merchantability, fitness for a 
 * particular purpose, or non-infringement. In no event shall the authors or copyright 
 * holders be liable for any claim, damages, or other liability, whether in an action 
 * of contract, tort, or otherwise, arising from, out of, or in connection with the 
 * software or the use or other dealings in the software.
 *
 * ���������ԭ�����ṩ���������κ���ʾ��ʾ�ĵ����������������ڶ������ԡ��ض���;������
 * �����Ȩ�ı�֤�����κ�����£����߻��Ȩ�����˾������������ʹ�ñ�������������κ�
 * ���⡢�𺦻��������θ��������Ǻ�ͬ���ϡ���Ȩ��Ϊ�������������
 */

#include <stdint.h>
#include <stdio.h>
#include <mcu_header.h>
#include <ezboot_config.h>
#include <pg_uart.h>

static recv_callback_t recv_callback = NULL;

void USART1_IRQHandler(void)
{
    if( USART_GetITStatus(USART1, USART_IT_RXNE) != RESET )
    {
		if(recv_callback)
		{
			uint8_t rx_data = USART_ReceiveData(USART1);
			recv_callback(&rx_data, 1);
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

inline int pg_uart_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	USART_Cmd(USART1, DISABLE); 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  // RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // ��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
   
	USART_InitStructure.USART_BaudRate = CONFIG_LOG_UART_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure); 
	USART_Cmd(USART1, ENABLE);  

	NVIC_EnableIRQ(USART1_IRQn);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
    return 0;
}

void pg_uart_send(const void* buf, uint16_t len)
{
	const uint8_t* send_buf = buf;
	int i;
	for(i=0; i<len; i++)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		USART_SendData(USART1, send_buf[i]);
	}
}

inline void pg_uart_register_rx_callback(recv_callback_t callback)
{
    recv_callback = callback;
}
