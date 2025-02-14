#ifndef __SYSTICK_H__
#define __SYSTICK_H__
#include <stdint.h>


int systick_init(void);

void systick_counter(void);

uint32_t systick_get(void);

#endif
