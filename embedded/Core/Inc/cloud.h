/**
 * @file    cloud.h
 * @brief   云端通信模块 - 位置上报、SOS告警、图片上传
 * @note    通过 SIM900A GPRS TCP 连接发送 JSON 数据到云端服务器
 */

#ifndef __CLOUD_H
#define __CLOUD_H

#include "main.h"
#include "gps.h"

/**
 * @brief  初始化云端通信（建立 GPRS 连接）
 * @retval 0=成功, -1=失败
 */
int8_t Cloud_Init(void);

/**
 * @brief  上报当前位置
 * @param  gps: GPS 数据
 * @param  battery_percent: 电池电量百分比
 * @retval 0=成功, -1=失败
 */
int8_t Cloud_ReportLocation(const GPS_Data_t *gps, uint8_t battery_percent);

/**
 * @brief  发送 SOS 告警
 * @param  gps: GPS 数据
 * @retval 0=成功, -1=失败
 */
int8_t Cloud_SendSOS(const GPS_Data_t *gps);

/**
 * @brief  上传图片数据（简化版，Base64编码）
 * @param  img_data: 图像数据（RGB565）
 * @param  data_len: 数据长度
 * @retval 0=成功, -1=失败
 * @note   由于 GPRS 带宽限制，建议上传缩略图
 */
int8_t Cloud_UploadImage(const uint8_t *img_data, uint16_t data_len);

/**
 * @brief  检查并处理云端下发的数据
 * @retval 1=有新数据, 0=无
 */
uint8_t Cloud_CheckReceived(void);

/**
 * @brief  检查云端连接状态，断线自动重连
 */
void Cloud_KeepAlive(void);

#endif /* __CLOUD_H */
