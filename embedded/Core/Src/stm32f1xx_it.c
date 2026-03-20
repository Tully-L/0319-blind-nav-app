/**
 * @file    stm32f1xx_it.c
 * @brief   中断服务程序实现
 * @note    所有中断处理函数集中在此文件
 *          各模块的具体处理逻辑通过回调函数分发
 */

#include "stm32f1xx_it.h"
#include "ultrasonic.h"
#include "gps.h"
#include "sim900a.h"
#include "voice.h"

/* 外部变量 */
extern volatile uint8_t g_sos_triggered;

/* UART 单字节接收缓冲（每个 UART 独立） */
static uint8_t s_uart1_byte;
static uint8_t s_uart2_byte;
static uint8_t s_uart3_byte;

/* ========== Cortex-M3 系统中断 ========== */

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    /* 硬件错误，死循环（实际项目可加看门狗复位） */
    while (1) {
    }
}

void MemManage_Handler(void)
{
    while (1) {
    }
}

void BusFault_Handler(void)
{
    while (1) {
    }
}

void UsageFault_Handler(void)
{
    while (1) {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

/* ========== 外设中断 ========== */

/**
 * @brief  TIM2 中断 - 超声波 Echo 输入捕获
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/**
 * @brief  USART1 中断 - CI1303 语音模块
 */
void USART1_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) {
        s_uart1_byte = (uint8_t)(huart1.Instance->DR & 0xFF);
        Voice_UART_RxCallback(s_uart1_byte);

        /* 继续接收下一字节 */
        HAL_UART_Receive_IT(&huart1, &s_uart1_byte, 1);
    }

    /* 清除溢出错误（防止接收卡死） */
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE) != RESET) {
        __HAL_UART_CLEAR_OREFLAG(&huart1);
    }
}

/**
 * @brief  USART2 中断 - GPS 模块
 */
void USART2_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET) {
        s_uart2_byte = (uint8_t)(huart2.Instance->DR & 0xFF);
        GPS_UART_RxCallback(s_uart2_byte);

        /* 继续接收下一字节 */
        HAL_UART_Receive_IT(&huart2, &s_uart2_byte, 1);
    }

    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE) != RESET) {
        __HAL_UART_CLEAR_OREFLAG(&huart2);
    }
}

/**
 * @brief  USART3 中断 - SIM900A 模块
 */
void USART3_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != RESET) {
        s_uart3_byte = (uint8_t)(huart3.Instance->DR & 0xFF);
        SIM900A_UART_RxCallback(s_uart3_byte);

        /* 继续接收下一字节 */
        HAL_UART_Receive_IT(&huart3, &s_uart3_byte, 1);
    }

    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_ORE) != RESET) {
        __HAL_UART_CLEAR_OREFLAG(&huart3);
    }
}

/**
 * @brief  EXTI15_10 中断 - SOS 按键 (PB13)
 */
void EXTI15_10_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(SOS_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(SOS_PIN);

        /* 简单防抖：检查引脚当前状态 */
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(SOS_PORT, SOS_PIN) == GPIO_PIN_RESET) {
            g_sos_triggered = 1;
        }
    }
}

/**
 * @brief  HAL TIM 输入捕获回调（由 HAL 库调用）
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    Ultrasonic_TIM_IC_Callback(htim);
}
