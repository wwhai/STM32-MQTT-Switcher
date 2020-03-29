/*
 * delay.h
 *
 *  Created on: 2017年5月7日
 *      Author: xianlee
 */

#ifndef DELAY_DELAY_H_
#define DELAY_DELAY_H_

#include "stm32f10x.h"
#include "stdint.h"

extern uint8_t  __fac_us;
extern uint16_t __fac_ms;

#define TIME_LOOP_MS(TIME_OUT){							\
	uint32_t __sys_tick__ = 0x00000001;					\
	SysTick->LOAD = (uint32_t) TIME_OUT*__fac_ms;		\
	SysTick->VAL = 0x00;								\
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;			\
	for(;(__sys_tick__&0x01)&&!(__sys_tick__&(1<<16)); 	\
	__sys_tick__=SysTick->CTRL)

void delay_init(void);
void delay_us(u32 nus);
void delay_ms(u16 nms);

#endif /* DELAY_DELAY_H_ */
