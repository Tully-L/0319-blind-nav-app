/**
 * @file    motor.h
 * @brief   震动马达 PWM 控制驱动
 * @note    使用 TIM3 的 3 个通道输出 PWM
 *          左马达: PA6 (TIM3_CH1)
 *          中马达: PA7 (TIM3_CH2)
 *          右马达: PB0 (TIM3_CH3)
 */

#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

/* 马达通道定义 */
typedef enum {
    MOTOR_LEFT   = 0,   /* 左侧马达 */
    MOTOR_CENTER = 1,   /* 中间马达 */
    MOTOR_RIGHT  = 2    /* 右侧马达 */
} Motor_Channel_t;

/* 震动强度定义 (PWM 占空比百分比) */
#define MOTOR_OFF           0       /* 关闭 */
#define MOTOR_LOW_FREQ      30      /* 低频震动 (预警) */
#define MOTOR_HIGH_FREQ     80      /* 高频震动 (紧急) */
#define MOTOR_MAX           100     /* 最大强度 */

/**
 * @brief  初始化震动马达 PWM (TIM3)
 */
void Motor_Init(void);

/**
 * @brief  设置单个马达的震动强度
 * @param  ch: 马达通道
 * @param  intensity: 强度 0-100 (PWM 占空比%)
 */
void Motor_SetIntensity(Motor_Channel_t ch, uint8_t intensity);

/**
 * @brief  设置所有马达的震动强度
 * @param  intensity: 强度 0-100
 */
void Motor_SetAll(uint8_t intensity);

/**
 * @brief  停止所有马达
 */
void Motor_StopAll(void);

#endif /* __MOTOR_H */
