/**
 * @file    motor.c
 * @brief   震动马达 PWM 控制驱动实现
 *
 * 工作原理：
 * - TIM3 工作在 PWM 模式，频率固定约 1KHz
 * - 通过调整占空比控制震动强度
 * - 低占空比 = 轻微震动（预警）
 * - 高占空比 = 强烈震动（紧急）
 *
 * PWM 参数：
 * - 时钟源: 72MHz
 * - 预分频: 72 → 1MHz
 * - 计数周期: 1000 → PWM 频率 1KHz
 */

#include "motor.h"

/**
 * @brief  初始化震动马达 PWM
 */
void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* 使能时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* 配置 PWM 输出引脚为复用推挽输出 */
    /* PA6 (TIM3_CH1), PA7 (TIM3_CH2) */
    GPIO_InitStruct.Pin = MOTOR_LEFT_PIN | MOTOR_CENTER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB0 (TIM3_CH3) */
    GPIO_InitStruct.Pin = MOTOR_RIGHT_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 配置 TIM3 基本参数 */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 72 - 1;             /* 72MHz / 72 = 1MHz */
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1000 - 1;              /* 1MHz / 1000 = 1KHz PWM */
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim3);

    /* 配置 PWM 通道 */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;                       /* 初始占空比 0 */
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, MOTOR_LEFT_CHANNEL);
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, MOTOR_CENTER_CHANNEL);
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, MOTOR_RIGHT_CHANNEL);

    /* 启动 PWM 输出 */
    HAL_TIM_PWM_Start(&htim3, MOTOR_LEFT_CHANNEL);
    HAL_TIM_PWM_Start(&htim3, MOTOR_CENTER_CHANNEL);
    HAL_TIM_PWM_Start(&htim3, MOTOR_RIGHT_CHANNEL);
}

/**
 * @brief  设置单个马达的震动强度
 */
void Motor_SetIntensity(Motor_Channel_t ch, uint8_t intensity)
{
    uint32_t pulse;
    uint32_t channel;

    /* 限制范围 */
    if (intensity > 100) intensity = 100;

    /* 计算 PWM 脉冲值 (0-999) */
    pulse = (uint32_t)intensity * 999 / 100;

    /* 选择通道 */
    switch (ch) {
        case MOTOR_LEFT:    channel = MOTOR_LEFT_CHANNEL;   break;
        case MOTOR_CENTER:  channel = MOTOR_CENTER_CHANNEL; break;
        case MOTOR_RIGHT:   channel = MOTOR_RIGHT_CHANNEL;  break;
        default: return;
    }

    __HAL_TIM_SET_COMPARE(&htim3, channel, pulse);
}

/**
 * @brief  设置所有马达的震动强度
 */
void Motor_SetAll(uint8_t intensity)
{
    Motor_SetIntensity(MOTOR_LEFT, intensity);
    Motor_SetIntensity(MOTOR_CENTER, intensity);
    Motor_SetIntensity(MOTOR_RIGHT, intensity);
}

/**
 * @brief  停止所有马达
 */
void Motor_StopAll(void)
{
    Motor_SetAll(MOTOR_OFF);
}
