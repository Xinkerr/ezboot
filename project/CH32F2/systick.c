#include <mcu_header.h>
#include <systick.h>
#include <stdint.h>

__IO uint32_t uwTick = 0;

int system_init(void)
{
	return systick_init();
}

int systick_init(void)
{
	if (SysTick_Config(SystemCoreClock / (1000U)) > 0U)
	{
		return -1;
	}
	
	uint32_t prioritygroup = NVIC_GetPriorityGrouping();
	
	NVIC_SetPriority(SysTick_IRQn , NVIC_EncodePriority(prioritygroup, 15, 0));
	
	return 0;
}

void systick_counter(void)
{
	uwTick ++;
}

uint32_t systick_get(void)
{
	return uwTick;
}
