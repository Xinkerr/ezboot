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
