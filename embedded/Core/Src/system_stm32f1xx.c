/**
 * @file    system_stm32f1xx.c
 * @brief   CMSIS 系统初始化文件
 * @note    配置系统时钟和中断向量表
 *          这是 STM32 HAL 库要求的系统文件
 */

#include "stm32f1xx.h"

/* 系统时钟频率变量（HAL 库使用） */
uint32_t SystemCoreClock = 72000000U;

/* AHB 分频表 */
const uint8_t AHBPrescTable[16U] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
/* APB 分频表 */
const uint8_t APBPrescTable[8U]  = {0, 0, 0, 0, 1, 2, 3, 4};

/**
 * @brief  系统初始化
 * @note   在进入 main() 之前由启动代码调用
 *         配置 FPU、中断向量表偏移等
 */
void SystemInit(void)
{
    /* 复位 RCC 时钟配置到默认状态 */
    /* 使能 HSI */
    RCC->CR |= 0x00000001U;

    /* 复位 SW, HPRE, PPRE1, PPRE2, ADCPRE, MCO */
    RCC->CFGR &= 0xF8FF0000U;

    /* 复位 HSEON, CSSON, PLLON */
    RCC->CR &= 0xFEF6FFFFU;

    /* 复位 HSEBYP */
    RCC->CR &= 0xFFFBFFFFU;

    /* 复位 PLLSRC, PLLXTPRE, PLLMUL, USBPRE/OTGFSPRE */
    RCC->CFGR &= 0xFF80FFFFU;

    /* 关闭所有中断 */
    RCC->CIR = 0x009F0000U;

    /* 中断向量表位置（Flash 起始地址） */
    SCB->VTOR = FLASH_BASE;
}
