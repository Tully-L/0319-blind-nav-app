/**
 * @file    gps.h
 * @brief   ATGM336H GPS 模块驱动 - NMEA 协议解析
 * @note    使用 USART2 (PA2/PA3) 接收 NMEA 数据
 *          解析 $GNRMC 和 $GNGGA 语句
 */

#ifndef __GPS_H
#define __GPS_H

#include "main.h"

/* GPS 接收缓冲区大小 */
#define GPS_RX_BUF_SIZE     256

/* GPS 数据结构 */
typedef struct {
    double   latitude;      /* 纬度（十进制度） */
    double   longitude;     /* 经度（十进制度） */
    float    speed;         /* 速度 (km/h) */
    float    altitude;      /* 海拔 (m) */
    uint8_t  hour;          /* UTC 时 */
    uint8_t  minute;        /* UTC 分 */
    uint8_t  second;        /* UTC 秒 */
    uint8_t  satellites;    /* 使用的卫星数 */
    uint8_t  fix_valid;     /* 定位有效标志 (1=有效, 0=无效) */
    char     ns;            /* 南北半球 N/S */
    char     ew;            /* 东西半球 E/W */
} GPS_Data_t;

/**
 * @brief  初始化 GPS 模块（USART2 + DMA/中断接收）
 */
void GPS_Init(void);

/**
 * @brief  处理接收到的 GPS 数据（在主循环中定期调用）
 * @note   内部解析 NMEA 语句并更新 GPS 数据
 */
void GPS_Process(void);

/**
 * @brief  获取当前 GPS 数据
 * @retval GPS 数据结构指针
 */
const GPS_Data_t* GPS_GetData(void);

/**
 * @brief  GPS UART 接收中断回调
 * @param  byte: 接收到的字节
 */
void GPS_UART_RxCallback(uint8_t byte);

/**
 * @brief  检查 GPS 是否已定位
 * @retval 1=已定位, 0=未定位
 */
uint8_t GPS_IsFixed(void);

#endif /* __GPS_H */
