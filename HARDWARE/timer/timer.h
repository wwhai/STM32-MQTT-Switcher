#ifndef __TIMER_H__
#define __TIMER_H__

#include "stdint.h"

extern uint32_t sys_time_sec;

void timer3_init(uint16_t arr, uint16_t psc);

#endif
