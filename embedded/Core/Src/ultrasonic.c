/**
 * @file    ultrasonic.c
 * @brief   HC-SR04 超声波测距模块驱动实现
 *
 * 工作原理：
 * 1. Trig 引脚输出 10us 高电平脉冲，触发超声波发射
 * 2. Echo 引脚返回高电平，高电平持续时间与距离成正比
 * 3. 距离(cm) = 高电平时间(us) / 58
 * 4. 使用 TIM2_CH2 输入捕获精确测量 Echo 高电平时间
 */

#include "ultrasonic.h"

/* 私有变量 */
static volatile uint32_t s_capture_rise = 0;    /* 上升沿捕获值 */
static volatile uint32_t s_capture_fall = 0;    /* 下降沿捕获值 */
static volatile uint8_t  s_capture_done = 0;    /* 捕获完成标志 */
static volatile uint8_t  s_capture_state = 0;   /* 0=等待上升沿, 1=等待下降沿 */
static uint16_t s_last_distance = 0;            /* 最近一次测量距离 */

/**
 * @brief  微秒级延时（简单循环实现）
 * @param  us: 延时微秒数
 * @note   72MHz 主频下约 9 个循环 = 1us
 */
static void delay_us(uint32_t us)
{
    uint32_t count = us * 9;
    while (count--) {
        __NOP();
    }
}

/**
 * @brief  初始化超声波模块
 */
void Ultrasonic_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    /* 使能时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* 配置 Trig 引脚 PA0 为推挽输出 */
    GPIO_InitStruct.Pin = ULTRA_TRIG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ULTRA_TRIG_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(ULTRA_TRIG_PORT, ULTRA_TRIG_PIN, GPIO_PIN_RESET);

    /* 配置 TIM2 基本参数：1us 计数 */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 72 - 1;             /* 72MHz / 72 = 1MHz, 1us 一个计数 */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFF;                 /* 最大计数 65535us ≈ 65ms */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_IC_Init(&htim2);

    /* 配置 TIM2_CH2 输入捕获：先捕获上升沿 */
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0x04;                  /* 适当滤波 */
    HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, ULTRA_TIM_CHANNEL);

    /* 使能 TIM2 中断 */
    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/**
 * @brief  触发一次测距并等待结果
 * @retval 距离(cm)，0 表示超时或无效
 */
uint16_t Ultrasonic_Measure(void)
{
    uint32_t pulse_us;
    uint32_t timeout;

    /* 复位捕获状态 */
    s_capture_done = 0;
    s_capture_state = 0;

    /* 设置为上升沿捕获 */
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, ULTRA_TIM_CHANNEL, TIM_INPUTCHANNELPOLARITY_RISING);

    /* 启动输入捕获中断 */
    HAL_TIM_IC_Start_IT(&htim2, ULTRA_TIM_CHANNEL);

    /* 发送 10us Trig 脉冲 */
    HAL_GPIO_WritePin(ULTRA_TRIG_PORT, ULTRA_TRIG_PIN, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(ULTRA_TRIG_PORT, ULTRA_TRIG_PIN, GPIO_PIN_RESET);

    /* 等待捕获完成，超时 30ms（约 5m 距离） */
    timeout = HAL_GetTick() + 30;
    while (!s_capture_done && HAL_GetTick() < timeout) {
        /* 等待 */
    }

    /* 停止输入捕获 */
    HAL_TIM_IC_Stop_IT(&htim2, ULTRA_TIM_CHANNEL);

    if (!s_capture_done) {
        s_last_distance = 0;
        return 0;   /* 超时，无回波 */
    }

    /* 计算高电平时间 (us) */
    if (s_capture_fall >= s_capture_rise) {
        pulse_us = s_capture_fall - s_capture_rise;
    } else {
        pulse_us = (0xFFFF - s_capture_rise) + s_capture_fall + 1;
    }

    /* 计算距离 */
    s_last_distance = (uint16_t)(pulse_us / ULTRA_SOUND_DIVISOR);

    /* 范围检查 */
    if (s_last_distance < ULTRA_MIN_DIST_CM || s_last_distance > ULTRA_MAX_DIST_CM) {
        s_last_distance = 0;
    }

    return s_last_distance;
}

/**
 * @brief  TIM2 输入捕获中断回调
 */
void Ultrasonic_TIM_IC_Callback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM2 || htim->Channel != HAL_TIM_ACTIVE_CHANNEL_2) {
        return;
    }

    if (s_capture_state == 0) {
        /* 上升沿：记录起始值，切换为下降沿捕获 */
        s_capture_rise = HAL_TIM_ReadCapturedValue(htim, ULTRA_TIM_CHANNEL);
        s_capture_state = 1;
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, ULTRA_TIM_CHANNEL, TIM_INPUTCHANNELPOLARITY_FALLING);
    } else {
        /* 下降沿：记录结束值，标记完成 */
        s_capture_fall = HAL_TIM_ReadCapturedValue(htim, ULTRA_TIM_CHANNEL);
        s_capture_done = 1;
        s_capture_state = 0;
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, ULTRA_TIM_CHANNEL, TIM_INPUTCHANNELPOLARITY_RISING);
    }
}

/**
 * @brief  获取最近一次测量的距离
 */
uint16_t Ultrasonic_GetDistance(void)
{
    return s_last_distance;
}
