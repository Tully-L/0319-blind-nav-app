/**
 * @file    navigation.h
 * @brief   导航逻辑模块 - GPS 数据处理与方向计算
 * @note    处理 GPS 原始数据，计算到目标点的距离和方位角
 */

#ifndef __NAVIGATION_H
#define __NAVIGATION_H

#include "main.h"
#include "gps.h"

/* 导航状态 */
typedef enum {
    NAV_IDLE     = 0,   /* 空闲（未导航） */
    NAV_ACTIVE   = 1,   /* 导航中 */
    NAV_ARRIVED  = 2    /* 已到达目标 */
} Nav_State_t;

/* 导航数据结构 */
typedef struct {
    Nav_State_t state;          /* 导航状态 */
    double      target_lat;     /* 目标纬度 */
    double      target_lng;     /* 目标经度 */
    double      distance_m;     /* 到目标距离 (m) */
    double      bearing_deg;    /* 到目标方位角 (度, 0=北) */
    double      current_lat;    /* 当前纬度 */
    double      current_lng;    /* 当前经度 */
} Nav_Data_t;

/**
 * @brief  初始化导航模块
 */
void Navigation_Init(void);

/**
 * @brief  设置导航目标点
 * @param  lat: 目标纬度
 * @param  lng: 目标经度
 */
void Navigation_SetTarget(double lat, double lng);

/**
 * @brief  更新导航数据（在 GPS 数据更新后调用）
 */
void Navigation_Update(void);

/**
 * @brief  获取当前导航数据
 * @retval 导航数据指针
 */
const Nav_Data_t* Navigation_GetData(void);

/**
 * @brief  停止导航
 */
void Navigation_Stop(void);

/**
 * @brief  计算两个 GPS 坐标之间的距离 (m)
 * @param  lat1, lng1: 起点坐标
 * @param  lat2, lng2: 终点坐标
 * @retval 距离 (m)
 */
double Navigation_CalcDistance(double lat1, double lng1, double lat2, double lng2);

#endif /* __NAVIGATION_H */
