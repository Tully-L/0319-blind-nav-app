/**
 * @file    battery.c
 * @brief   电池电压 ADC 采集驱动实现
 *
 * 工作原理：
 * 1. PA4 接电池分压电路，分压比 2:1
 * 2. ADC 12位分辨率 (0-4095)，参考电压 3.3V
 * 3. ADC值 → 电压(mV) = ADC * 3300 / 4095 * 分压比
 * 4. 电压 → 百分比：线性映射 (3300mV=0%, 4200mV=100%)
 */

#include "battery.h"

/**
 * @brief  初始化电池 ADC 采集
 */
void Battery_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    ADC_ChannelConfTypeDef sConfig = {0};

    /* 使能时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();

    /* 配置 PA4 为模拟输入 */
    GPIO_InitStruct.Pin = BATTERY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(BATTERY_PORT, &GPIO_InitStruct);

    /* 配置 ADC1 */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;         /* 单通道不扫描 */
    hadc1.Init.ContinuousConvMode = DISABLE;             /* 单次转换 */
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;    /* 软件触发 */
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;          /* 右对齐 */
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);

    /* 配置 ADC 通道 */
    sConfig.Channel = BATTERY_ADC_CHANNEL;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;   /* 最长采样时间，更稳定 */
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /* ADC 校准 */
    HAL_ADCEx_Calibration_Start(&hadc1);
}

/**
 * @brief  采集电池电压 (mV)
 */
uint16_t Battery_GetVoltage(void)
{
    uint32_t adc_val;

    /* 启动 ADC 转换 */
    HAL_ADC_Start(&hadc1);

    /* 等待转换完成 */
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        return 0;
    }

    /* 读取 ADC 值 */
    adc_val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    /* 计算电压: ADC值 * 参考电压(3300mV) / 满量程(4095) * 分压比 */
    return (uint16_t)(adc_val * 3300 / 4095 * BATTERY_DIVIDER);
}

/**
 * @brief  获取电池电量百分比
 */
uint8_t Battery_GetPercent(void)
{
    uint16_t voltage = Battery_GetVoltage();

    if (voltage >= BATTERY_FULL_MV) return 100;
    if (voltage <= BATTERY_EMPTY_MV) return 0;

    /* 线性映射 */
    return (uint8_t)((uint32_t)(voltage - BATTERY_EMPTY_MV) * 100
                     / (BATTERY_FULL_MV - BATTERY_EMPTY_MV));
}

/**
 * @brief  检查是否低电量
 */
uint8_t Battery_IsLow(void)
{
    return (Battery_GetPercent() < BATTERY_LOW_PERCENT) ? 1 : 0;
}
