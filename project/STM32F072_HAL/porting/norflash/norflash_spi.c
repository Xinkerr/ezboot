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

static SPI_HandleTypeDef* spi_handle = NULL;

/**
 * @brief 初始化SPI
 * 
 * @param 无
 * @return 返回0，表示初始化成功。
 */
int norflash_spi_init(void)
{
    spi_handle = &hspi2;
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
	HAL_SPI_TransmitReceive(spi_handle, &send_data, &recv_data, 1, 0xffff);
    return recv_data;
}
