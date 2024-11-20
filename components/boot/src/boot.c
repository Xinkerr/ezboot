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

#include <boot.h>
#include <ezboot_config.h>
#include <mcu_header.h>


uint32_t JumpAddress =0;

typedef void(*pFunction)(void);
pFunction Jump_To_Application;

void app_enter(void)
{
    /* 屏蔽所有中断，防止在跳转过程中，中断干扰出现异常 */
    __disable_irq();
	// APP_ADDRESS + 4地址的值
    JumpAddress = *(__IO uint32_t*)(APP_ADDRESS + 4);
    // 强制转换
    Jump_To_Application = (pFunction) JumpAddress;
    // 设置堆栈指针指向目标app地址
    __set_MSP(*(__IO uint32_t*)APP_ADDRESS);
    // 跳转到app
    Jump_To_Application();
}
