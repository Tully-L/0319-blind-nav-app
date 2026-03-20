/**
 * @file    warning.c
 * @brief   分级预警逻辑模块实现
 *
 * 预警策略：
 * 1. 综合超声波测距和红外避障的结果
 * 2. 超声波提供精确距离信息，红外提供近距离补充
 * 3. 取两者中更高的预警级别
 * 4. 根据级别驱动震动马达和蜂鸣器
 */

#include "warning.h"
#include "motor.h"
#include "buzzer.h"

/* 私有变量 */
static Warning_Level_t s_current_level = WARN_NONE;

/**
 * @brief  初始化预警模块
 */
void Warning_Init(void)
{
    s_current_level = WARN_NONE;
}

/**
 * @brief  根据传感器数据评估预警级别
 */
Warning_Level_t Warning_Evaluate(uint16_t distance_cm, uint8_t ir_obstacle)
{
    Warning_Level_t ultra_level = WARN_NONE;
    Warning_Level_t ir_level = WARN_NONE;

    /* 超声波距离判断 */
    if (distance_cm > 0) {
        if (distance_cm < WARN_DIST_MODERATE) {
            ultra_level = WARN_URGENT;          /* < 50cm 紧急 */
        } else if (distance_cm < WARN_DIST_MILD) {
            ultra_level = WARN_MODERATE;         /* 50-100cm 中度 */
        } else {
            ultra_level = WARN_NONE;             /* > 100cm 安全 */
        }
    }

    /* 红外避障补充判断 */
    if (ir_obstacle) {
        /* 红外检测到障碍物（近距离，约 2-30cm） */
        ir_level = WARN_MODERATE;
    }

    /* 取更高级别 */
    s_current_level = (ultra_level > ir_level) ? ultra_level : ir_level;

    return s_current_level;
}

/**
 * @brief  执行预警动作
 */
void Warning_Execute(Warning_Level_t level)
{
    switch (level) {
        case WARN_NONE:
            /* 安全：停止所有预警 */
            Motor_StopAll();
            Buzzer_Off();
            break;

        case WARN_MILD:
            /* 轻度：仅语音提示（由主循环处理语音播报） */
            Motor_StopAll();
            Buzzer_Off();
            break;

        case WARN_MODERATE:
            /* 中度：低频震动 */
            Motor_SetAll(MOTOR_LOW_FREQ);
            Buzzer_Off();
            break;

        case WARN_URGENT:
            /* 紧急：高频震动 + 蜂鸣器 */
            Motor_SetAll(MOTOR_HIGH_FREQ);
            Buzzer_On();
            break;
    }
}

/**
 * @brief  获取当前预警级别
 */
Warning_Level_t Warning_GetLevel(void)
{
    return s_current_level;
}
