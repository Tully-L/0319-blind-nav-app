/**
 * @file    voice.h
 * @brief   CI1303 语音识别模块驱动
 * @note    使用 USART1 (PA9/PA10)
 *          CI1303 识别到语音后通过 UART 发送指令编号
 *
 * 语音指令定义（需与 CI1303 离线语音模型一致）：
 * 0x01 - "导航模式" / "开始导航"
 * 0x02 - "我在哪里" / "查询位置"
 * 0x03 - "紧急求助" / "帮帮我"
 * 0x04 - "电量查询" / "还有多少电"
 * 0x05 - "拍照" / "看看前方"
 * 0x06 - "停止" / "关闭"
 */

#ifndef __VOICE_H
#define __VOICE_H

#include "main.h"

/* 语音指令编号定义 */
#define VOICE_CMD_NAVIGATE      0x01    /* 开始导航 */
#define VOICE_CMD_QUERY_POS     0x02    /* 查询位置 */
#define VOICE_CMD_SOS           0x03    /* 紧急求助 */
#define VOICE_CMD_BATTERY       0x04    /* 电量查询 */
#define VOICE_CMD_PHOTO         0x05    /* 拍照 */
#define VOICE_CMD_STOP          0x06    /* 停止 */
#define VOICE_CMD_NONE          0x00    /* 无指令 */

/**
 * @brief  初始化 CI1303 语音模块（USART1）
 */
void Voice_Init(void);

/**
 * @brief  获取最新的语音指令编号（读后清零）
 * @retval 指令编号，VOICE_CMD_NONE 表示无新指令
 */
uint8_t Voice_GetCommand(void);

/**
 * @brief  CI1303 UART 接收中断回调
 */
void Voice_UART_RxCallback(uint8_t byte);

#endif /* __VOICE_H */
