/**
 * @file    warning.h
 * @brief   分级预警逻辑模块
 * @note    根据超声波距离和红外避障状态，触发不同级别的预警
 *
 * 预警级别：
 * - NONE:     距离 > 1m 且无红外障碍 → 无预警
 * - MILD:     距离 > 1m 但有红外障碍 → 语音提示
 * - MODERATE: 距离 0.5m ~ 1m → 语音 + 低频震动
 * - URGENT:   距离 < 0.5m → 语音 + 高频震动 + 蜂鸣器
 */

#ifndef __WARNING_H
#define __WARNING_H

#include "main.h"

/* 预警级别 */
typedef enum {
    WARN_NONE     = 0,  /* 安全 */
    WARN_MILD     = 1,  /* 轻度 */
    WARN_MODERATE = 2,  /* 中度 */
    WARN_URGENT   = 3   /* 紧急 */
} Warning_Level_t;

/**
 * @brief  初始化预警模块
 */
void Warning_Init(void);

/**
 * @brief  根据当前传感器数据评估预警级别
 * @param  distance_cm: 超声波距离 (cm)，0 表示无有效数据
 * @param  ir_obstacle: 红外检测到障碍物 (1=有, 0=无)
 * @retval 预警级别
 */
Warning_Level_t Warning_Evaluate(uint16_t distance_cm, uint8_t ir_obstacle);

/**
 * @brief  执行预警动作（震动+蜂鸣器）
 * @param  level: 预警级别
 */
void Warning_Execute(Warning_Level_t level);

/**
 * @brief  获取当前预警级别
 */
Warning_Level_t Warning_GetLevel(void);

#endif /* __WARNING_H */
