/*
 * usart.h
 *
 *  Created on: 2017年5月6日
 *      Author: xianlee
 */
#include "stdio.h"	
#include "sys.h" 

#ifndef USART_USART_H_
#define USART_USART_H_

#define AT_DEBUG						//开启GSM模块串口DEBUG
#define DEBUG_USART			USART1		//指定调试串口

extern u16 USART_RX_STA;         		//接收状态标记	
extern u16 USART_RX_CNT;				//接收的字节数	

void usart1_init(u32 bound);
void usart3_init(u32 bound);
void usart_printf(USART_TypeDef* USARTx, char *Data, ...);
void usart_dump(USART_TypeDef* USARTx, uint8_t* addr, uint32_t len);

#endif /* USART_USART_H_ */
