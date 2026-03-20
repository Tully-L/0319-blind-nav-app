/**
 * @file    buzzer.h
 * @brief   蜂鸣器控制驱动
 * @note    PB12 GPIO 输出，高电平驱动有源蜂鸣器
 */

#ifndef __BUZZER_H
#define __BUZZER_H

#include "main.h"

/**
 * @brief  初始化蜂鸣器 GPIO
 */
void Buzzer_Init(void);

/**
 * @brief  打开蜂鸣器
 */
void Buzzer_On(void);

/**
 * @brief  关闭蜂鸣器
 */
void Buzzer_Off(void);

/**
 * @brief  蜂鸣器响指定时长
 * @param  ms: 响铃时长 (毫秒)
 */
void Buzzer_Beep(uint32_t ms);

/**
 * @brief  蜂鸣器短促鸣叫（提示音）
 * @param  count: 鸣叫次数
 */
void Buzzer_Alert(uint8_t count);

#endif /* __BUZZER_H */
