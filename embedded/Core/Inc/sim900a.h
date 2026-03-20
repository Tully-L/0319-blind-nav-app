/**
 * @file    sim900a.h
 * @brief   SIM900A GSM/GPRS 模块驱动
 * @note    使用 USART3 (PB10/PB11)
 *          支持短信发送(SOS)和 GPRS TCP 通信(位置上报)
 */

#ifndef __SIM900A_H
#define __SIM900A_H

#include "main.h"

/* SIM900A 接收缓冲区大小 */
#define SIM_RX_BUF_SIZE     512

/* AT 指令超时时间 (ms) */
#define SIM_AT_TIMEOUT      2000
#define SIM_SMS_TIMEOUT     10000
#define SIM_TCP_TIMEOUT     15000

/* GPRS APN 配置（根据运营商修改） */
#define SIM_APN             "CMNET"     /* 中国移动 */

/**
 * @brief  初始化 SIM900A 模块（USART3 + AT 指令握手）
 * @retval 0=成功, -1=模块无响应, -2=SIM卡未就绪
 */
int8_t SIM900A_Init(void);

/**
 * @brief  发送短信
 * @param  phone: 手机号码（如 "13800138000"）
 * @param  message: 短信内容（ASCII 文本）
 * @retval 0=成功, -1=失败
 */
int8_t SIM900A_SendSMS(const char *phone, const char *message);

/**
 * @brief  建立 GPRS TCP 连接
 * @param  ip: 服务器 IP 地址
 * @param  port: 服务器端口
 * @retval 0=成功, -1=失败
 */
int8_t SIM900A_TCP_Connect(const char *ip, const char *port);

/**
 * @brief  通过 TCP 发送数据
 * @param  data: 要发送的数据
 * @param  len: 数据长度
 * @retval 0=成功, -1=失败
 */
int8_t SIM900A_TCP_Send(const char *data, uint16_t len);

/**
 * @brief  关闭 TCP 连接
 */
void SIM900A_TCP_Close(void);

/**
 * @brief  检查模块是否在线
 * @retval 1=在线, 0=离线
 */
uint8_t SIM900A_IsOnline(void);

/**
 * @brief  SIM900A UART 接收中断回调
 */
void SIM900A_UART_RxCallback(uint8_t byte);

/**
 * @brief  获取最近一次 TCP 接收到的数据
 * @param  buf: 输出缓冲区
 * @param  max_len: 最大长度
 * @retval 实际接收长度
 */
uint16_t SIM900A_TCP_GetReceived(char *buf, uint16_t max_len);

#endif /* __SIM900A_H */
