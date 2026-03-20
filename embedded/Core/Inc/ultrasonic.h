/**
 * @file    ultrasonic.h
 * @brief   HC-SR04 超声波测距模块驱动
 * @note    使用 TIM2_CH2 输入捕获测量 Echo 高电平时间
 *          Trig: PA0 (GPIO输出)
 *          Echo: PA1 (TIM2_CH2 输入捕获)
 */

#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "main.h"

/* 超声波测距范围 (cm) */
#define ULTRA_MIN_DIST_CM       2
#define ULTRA_MAX_DIST_CM       400

/* 声速常数：距离(cm) = 高电平时间(us) / 58 */
#define ULTRA_SOUND_DIVISOR     58

/**
 * @brief  初始化超声波模块（GPIO + TIM2 输入捕获）
 */
void Ultrasonic_Init(void);

/**
 * @brief  触发一次测距
 * @retval 距离值(cm)，0 表示超出范围或超时
 */
uint16_t Ultrasonic_Measure(void);

/**
 * @brief  TIM2 输入捕获中断回调（在 stm32f1xx_it.c 中调用）
 */
void Ultrasonic_TIM_IC_Callback(TIM_HandleTypeDef *htim);

/**
 * @brief  获取最近一次测量的距离
 * @retval 距离值(cm)
 */
uint16_t Ultrasonic_GetDistance(void);

#endif /* __ULTRASONIC_H */
