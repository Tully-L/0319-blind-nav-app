/**
 * @file    battery.h
 * @brief   电池电压 ADC 采集驱动
 * @note    PA4 (ADC1_CH4)
 *          假设电池通过分压电阻接入，分压比 2:1
 *          即 ADC 测到 1.65V 时实际电压 3.3V
 */

#ifndef __BATTERY_H
#define __BATTERY_H

#include "main.h"

/* 电池参数（根据实际电池修改） */
#define BATTERY_FULL_MV     4200    /* 满电电压 mV (锂电池) */
#define BATTERY_EMPTY_MV    3300    /* 空电电压 mV */
#define BATTERY_DIVIDER     2       /* 分压比（实际电压 = ADC电压 * 分压比） */

/**
 * @brief  初始化电池 ADC 采集
 */
void Battery_Init(void);

/**
 * @brief  采集电池电压
 * @retval 电池电压 (mV)
 */
uint16_t Battery_GetVoltage(void);

/**
 * @brief  获取电池电量百分比
 * @retval 电量百分比 0-100
 */
uint8_t Battery_GetPercent(void);

/**
 * @brief  检查是否低电量
 * @retval 1=低电量, 0=正常
 */
uint8_t Battery_IsLow(void);

#endif /* __BATTERY_H */
