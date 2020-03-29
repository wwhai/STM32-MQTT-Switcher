/*
 * delay.c
 *
 *  Created on: 2017年5月7日
 *      Author: xianlee
 */
#include "system.h"

uint8_t  __fac_us=0;
uint16_t __fac_ms=0;

void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	__fac_us=SystemCoreClock/8000000;
	__fac_ms=(u16)__fac_us*1000;
}

//延时nus
//nus为要延时的us数.
void delay_us(u32 nus)
{
	u32 temp;
	SysTick->LOAD=nus*__fac_us; 					//时间加载
	SysTick->VAL=0x00;        					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;      					 //清空计数器
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对72M条件下,nms<=1864
void delay_ms(u16 nms)
{
#if 1
   while (nms--) {
	   delay_us(1000);
    }
#else 
	u32 temp;
	SysTick->LOAD=(u32)nms*__fac_ms;			//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;       					//清空计数器
#endif
}
