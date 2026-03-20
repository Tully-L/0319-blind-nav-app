/**
 * @file    infrared.h
 * @brief   红外避障模块驱动
 * @note    左路: PB1, 右路: PB3
 *          低电平(0) = 有障碍物, 高电平(1) = 无障碍物
 */

#ifndef __INFRARED_H
#define __INFRARED_H

#include "main.h"

/* 红外状态定义 */
#define IR_OBSTACLE     0   /* 检测到障碍物（低电平） */
#define IR_CLEAR        1   /* 无障碍物（高电平） */

/* 红外检测结果结构 */
typedef struct {
    uint8_t left;       /* 左侧传感器: IR_OBSTACLE / IR_CLEAR */
    uint8_t right;      /* 右侧传感器: IR_OBSTACLE / IR_CLEAR */
} IR_Status_t;

/**
 * @brief  初始化红外避障模块 GPIO
 */
void Infrared_Init(void);

/**
 * @brief  读取红外传感器状态
 * @retval 红外检测结果
 */
IR_Status_t Infrared_Read(void);

/**
 * @brief  检查是否有任一方向障碍物
 * @retval 1=有障碍物, 0=无障碍物
 */
uint8_t Infrared_HasObstacle(void);

#endif /* __INFRARED_H */
