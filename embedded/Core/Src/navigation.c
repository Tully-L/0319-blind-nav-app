/**
 * @file    navigation.c
 * @brief   导航逻辑模块实现
 *
 * 功能：
 * 1. 使用 Haversine 公式计算两点间距离
 * 2. 计算到目标点的方位角
 * 3. 维护导航状态（空闲/导航中/已到达）
 *
 * 公式说明：
 * Haversine 公式用于在球面上计算两点间的大圆距离
 * a = sin²(Δlat/2) + cos(lat1) * cos(lat2) * sin²(Δlng/2)
 * c = 2 * atan2(√a, √(1-a))
 * d = R * c （R = 地球半径 6371km）
 */

#include "navigation.h"
#include <math.h>

/* 地球半径 (m) */
#define EARTH_RADIUS_M      6371000.0

/* 角度转弧度 */
#define DEG_TO_RAD(d)       ((d) * 3.14159265358979 / 180.0)
#define RAD_TO_DEG(r)       ((r) * 180.0 / 3.14159265358979)

/* 到达判定距离 (m) */
#define NAV_ARRIVE_DIST_M   10.0

/* 私有变量 */
static Nav_Data_t s_nav_data = {0};

/**
 * @brief  初始化导航模块
 */
void Navigation_Init(void)
{
    s_nav_data.state = NAV_IDLE;
    s_nav_data.target_lat = 0;
    s_nav_data.target_lng = 0;
    s_nav_data.distance_m = 0;
    s_nav_data.bearing_deg = 0;
}

/**
 * @brief  计算两个 GPS 坐标之间的距离 (Haversine)
 */
double Navigation_CalcDistance(double lat1, double lng1, double lat2, double lng2)
{
    double dlat = DEG_TO_RAD(lat2 - lat1);
    double dlng = DEG_TO_RAD(lng2 - lng1);
    double a, c;

    lat1 = DEG_TO_RAD(lat1);
    lat2 = DEG_TO_RAD(lat2);

    a = sin(dlat / 2) * sin(dlat / 2) +
        cos(lat1) * cos(lat2) * sin(dlng / 2) * sin(dlng / 2);
    c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS_M * c;
}

/**
 * @brief  计算从点1到点2的方位角
 * @retval 方位角 (度, 0=北, 90=东, 180=南, 270=西)
 */
static double calc_bearing(double lat1, double lng1, double lat2, double lng2)
{
    double dlng = DEG_TO_RAD(lng2 - lng1);
    double bearing;

    lat1 = DEG_TO_RAD(lat1);
    lat2 = DEG_TO_RAD(lat2);

    bearing = atan2(sin(dlng) * cos(lat2),
                    cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dlng));

    bearing = RAD_TO_DEG(bearing);

    /* 归一化到 0-360 */
    if (bearing < 0) bearing += 360.0;

    return bearing;
}

/**
 * @brief  设置导航目标点
 */
void Navigation_SetTarget(double lat, double lng)
{
    s_nav_data.target_lat = lat;
    s_nav_data.target_lng = lng;
    s_nav_data.state = NAV_ACTIVE;
}

/**
 * @brief  更新导航数据
 */
void Navigation_Update(void)
{
    const GPS_Data_t *gps;

    if (s_nav_data.state != NAV_ACTIVE) {
        return;
    }

    gps = GPS_GetData();
    if (!gps->fix_valid) {
        return; /* 未定位，不更新 */
    }

    /* 更新当前位置 */
    s_nav_data.current_lat = gps->latitude;
    s_nav_data.current_lng = gps->longitude;

    /* 计算到目标的距离 */
    s_nav_data.distance_m = Navigation_CalcDistance(
        gps->latitude, gps->longitude,
        s_nav_data.target_lat, s_nav_data.target_lng);

    /* 计算方位角 */
    s_nav_data.bearing_deg = calc_bearing(
        gps->latitude, gps->longitude,
        s_nav_data.target_lat, s_nav_data.target_lng);

    /* 到达判定 */
    if (s_nav_data.distance_m < NAV_ARRIVE_DIST_M) {
        s_nav_data.state = NAV_ARRIVED;
    }
}

/**
 * @brief  获取导航数据
 */
const Nav_Data_t* Navigation_GetData(void)
{
    return &s_nav_data;
}

/**
 * @brief  停止导航
 */
void Navigation_Stop(void)
{
    s_nav_data.state = NAV_IDLE;
}
