/**
 * @file    buzzer.c
 * @brief   蜂鸣器控制驱动实现
 *
 * 有源蜂鸣器，给高电平就响，低电平就停。
 */

#include "buzzer.h"

/**
 * @brief  初始化蜂鸣器 GPIO
 */
void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = BUZZER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);

    /* 默认关闭 */
    Buzzer_Off();
}

void Buzzer_On(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void Buzzer_Off(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

void Buzzer_Beep(uint32_t ms)
{
    Buzzer_On();
    HAL_Delay(ms);
    Buzzer_Off();
}

void Buzzer_Alert(uint8_t count)
{
    uint8_t i;
    for (i = 0; i < count; i++) {
        Buzzer_Beep(100);
        if (i < count - 1) {
            HAL_Delay(100);
        }
    }
}
