/**
 * @file    infrared.c
 * @brief   红外避障模块驱动实现
 *
 * 工作原理：
 * - 红外避障模块内部有红外发射管和接收管
 * - 前方有障碍物时反射红外线，输出低电平
 * - 无障碍物时输出高电平
 * - 通过读取 GPIO 状态判断左/右两侧是否有障碍物
 */

#include "infrared.h"

/**
 * @brief  初始化红外避障模块 GPIO
 */
void Infrared_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /*
     * PB3 默认是 JTDO 功能，需要关闭 JTAG 才能用作普通 GPIO
     * 关闭 JTAG 但保留 SWD，释放 PA15/PB3/PB4
     */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();

    /* 配置 PB1(左) 和 PB3(右) 为上拉输入 */
    GPIO_InitStruct.Pin = IR_LEFT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(IR_LEFT_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = IR_RIGHT_PIN;
    HAL_GPIO_Init(IR_RIGHT_PORT, &GPIO_InitStruct);
}

/**
 * @brief  读取红外传感器状态
 */
IR_Status_t Infrared_Read(void)
{
    IR_Status_t status;

    status.left  = (uint8_t)HAL_GPIO_ReadPin(IR_LEFT_PORT, IR_LEFT_PIN);
    status.right = (uint8_t)HAL_GPIO_ReadPin(IR_RIGHT_PORT, IR_RIGHT_PIN);

    return status;
}

/**
 * @brief  检查是否有任一方向障碍物
 */
uint8_t Infrared_HasObstacle(void)
{
    IR_Status_t s = Infrared_Read();
    return (s.left == IR_OBSTACLE || s.right == IR_OBSTACLE) ? 1 : 0;
}
