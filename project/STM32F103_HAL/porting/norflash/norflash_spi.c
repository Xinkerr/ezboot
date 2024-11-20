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

#include <norflash_spi.h>
#include <mcu_header.h>

#define FLASH_CS_Pin GPIO_PIN_6
#define FLASH_CS_GPIO_Port GPIOB

SPI_HandleTypeDef nf_hspi;

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : FLASH_CS_Pin */
  GPIO_InitStruct.Pin = FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(FLASH_CS_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* SPI1 init function */
void MX_SPI_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  nf_hspi.Instance = SPI1;
  nf_hspi.Init.Mode = SPI_MODE_MASTER;
  nf_hspi.Init.Direction = SPI_DIRECTION_2LINES;
  nf_hspi.Init.DataSize = SPI_DATASIZE_8BIT;
  nf_hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
  nf_hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
  nf_hspi.Init.NSS = SPI_NSS_SOFT;
  nf_hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  nf_hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  nf_hspi.Init.TIMode = SPI_TIMODE_DISABLE;
  nf_hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  nf_hspi.Init.CRCPolynomial = 10;
  HAL_SPI_Init(&nf_hspi);
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
}

static void gpio_configuration(void) 
{
    MX_GPIO_Init();
	HAL_SPI_MspInit(&nf_hspi);
}

static void spi_configuration(void) 
{
    MX_SPI_Init();
}

/**
 * @brief 初始化SPI NOR Flash
 * 
 * 本函数负责初始化SPI总线和相关的GPIO端口，为后续的NOR Flash操作做准备。
 * 
 * @param 无
 * @return 返回0，表示初始化成功。
 */
int norflash_spi_init(void)
{
    gpio_configuration();
    spi_configuration();
	
	return 0;
}

/**
 * @brief 控制SPI NorFlash的CS信号线
 * 
 * 本函数用于通过GPIO控制SPI NorFlash设备的片选信号线。根据输入参数决定是使能还是禁用该信号线。
 * 
 * @param enable 如果为真，则禁用SPI NorFlash的CS信号线（即选中设备）；如果为假，则启用CS信号线（即不选中设备）。
 * @return int 返回0，表示操作成功。
 */
int norflash_spi_cs(bool enable)
{
    if(enable)
		HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
	return 0;
}

/**
 * @brief 通过SPI与norflash进行单字节数据传输
 * 
 * @param send_data 需要发送的数据，类型为uint8_t
 * @return 接收到的数据，类型为uint8_t
 */
uint8_t norflash_spi_transfer(uint8_t send_data)
{
    uint8_t recv_data;
	HAL_SPI_TransmitReceive(&nf_hspi, &send_data, &recv_data, 1, 0xffff);
    return recv_data;
}
