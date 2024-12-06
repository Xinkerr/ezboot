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

#include <ezb_flash.h>
#include <string.h>
#include <mcu_header.h>

#define WORD_ALIGNMENT          4

int ezb_flash_erase(uint32_t addr, uint32_t size) {
    FLASH_EraseInitTypeDef eraseInitStruct;
    uint32_t pageError = 0;
    uint32_t numberOfPages;

    // 解锁Flash
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return -2; // 解锁失败
    }

    // 清除Flash标志
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPERR);

    // 计算页数
    numberOfPages = size / FLASH_PAGE_SIZE;

    // 配置擦除结构
    eraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInitStruct.PageAddress = addr;
    eraseInitStruct.NbPages = numberOfPages;

    // 擦除Flash页
    if (HAL_FLASHEx_Erase(&eraseInitStruct, &pageError) != HAL_OK) {
        // 擦除失败，记录错误
        HAL_FLASH_Lock(); // 锁定Flash
        return -1; // 返回错误码
    }

    // 锁定Flash
    HAL_FLASH_Lock();

    return 0; // 返回成功
}

int ezb_flash_write(uint32_t addr, const uint8_t *pdata, uint32_t size) {
    uint32_t write_data;
    uint32_t read_data;

    // 检查数据大小是否为字对齐
    if (size % WORD_ALIGNMENT != 0) {
        return -4; // 返回错误码
    }

    // 解锁Flash
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return -2; // 解锁失败
    }

    // 清除Flash标志
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPERR);

    // 写入数据
    for (uint32_t i = 0; i < size; i += WORD_ALIGNMENT, pdata += WORD_ALIGNMENT, addr += WORD_ALIGNMENT) {
        memcpy(&write_data, pdata, WORD_ALIGNMENT); // 确保内存对齐

        // 写入数据到Flash
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, write_data) != HAL_OK) {
            HAL_FLASH_Lock(); // 写入失败，锁定Flash
            return -1; // 返回错误码
        }

        // 验证写入的数据
        read_data = *(uint32_t *)addr;
        if (read_data != write_data) {
            HAL_FLASH_Lock(); // 验证失败，锁定Flash
            return -1; // 返回错误码
        }
    }

    // 锁定Flash
    HAL_FLASH_Lock();

    return 0; // 返回成功
}

int ezb_flash_read(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    int i;
    for (i = 0; i < size; i++, addr++, pdata++)
    {
        *pdata = *(uint8_t *) addr;
    }
    return 0;
}
