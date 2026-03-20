/**
 * @file    stm32f1xx_it.h
 * @brief   中断服务程序头文件
 */

#ifndef __STM32F1XX_IT_H
#define __STM32F1XX_IT_H

#include "main.h"

/* Cortex-M3 系统中断 */
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/* 外设中断 */
void TIM2_IRQHandler(void);         /* 超声波输入捕获 */
void USART1_IRQHandler(void);       /* CI1303 语音模块 */
void USART2_IRQHandler(void);       /* GPS */
void USART3_IRQHandler(void);       /* SIM900A */
void EXTI15_10_IRQHandler(void);    /* SOS 按键 (PB13) */

#endif /* __STM32F1XX_IT_H */
