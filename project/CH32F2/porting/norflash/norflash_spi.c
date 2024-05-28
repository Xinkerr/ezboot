#include <norflash_spi.h>
#include <ch32f20x.h>
#include <mlog.h>

static void gpio_configuration(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure );
    GPIO_SetBits(GPIOA, GPIO_Pin_2);
    //CLK
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOA, &GPIO_InitStructure );
    //MISO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init( GPIOA, &GPIO_InitStructure );
    //MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOA, &GPIO_InitStructure );
}

static void spi_configuration(void) 
{
    SPI_InitTypeDef SPI_InitStructure;

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init( SPI1, &SPI_InitStructure );

    SPI_I2S_DeInit(SPI1);
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_CalculateCRC(SPI1, DISABLE);
    SPI_Cmd(SPI1, ENABLE);
}

int norflash_spi_init(void)
{
    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_SPI1, ENABLE);
    gpio_configuration();
    spi_configuration();
	
	return 0;
}

int norflash_spi_cs(bool enable)
{
    if(enable)
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);
    else
        GPIO_SetBits(GPIOA, GPIO_Pin_2);
	return 0;
}

uint8_t norflash_spi_transfer(uint8_t send_data)
{
    uint16_t recv_data;
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);    //等待发送缓冲器空
    SPI_I2S_SendData(SPI1, send_data);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    recv_data = SPI_I2S_ReceiveData(SPI1);
    return (uint8_t)recv_data;
}
