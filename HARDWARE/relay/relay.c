/*
 * relay.c
 *
 *  Created on: 2017年5月16日
 *      Author: xianlee
 */

#include "stm32f10x.h"
#include "sys.h"
#include "system.h"
#include "relay.h"

#if 0
void relay_init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);
}

void relay_on()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}

void relay_off()
{
	GPIO_SetBits(GPIOA, GPIO_Pin_15);
}
#else
void relay_init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_14);
}

void relay_on()
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_14);
}

void relay_off()
{
	GPIO_SetBits(GPIOC, GPIO_Pin_14);
}
#endif