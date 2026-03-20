/**
 * @file    voice.c
 * @brief   CI1303 语音识别模块驱动实现
 *
 * 工作原理：
 * 1. CI1303 是离线语音识别芯片，内置语音模型
 * 2. 识别到唤醒词后进入命令模式
 * 3. 识别到命令后通过 UART 发送对应编号（1字节）
 * 4. 本驱动通过 USART1 中断接收指令编号
 *
 * 通信协议（简化版）：
 * CI1303 → STM32: [指令编号(1byte)]
 * 部分型号可能有帧头帧尾，根据实际模块调整
 */

#include "voice.h"

/* 私有变量 */
static uint8_t s_uart_byte;

/**
 * @brief  初始化 CI1303 语音模块
 */
void Voice_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    /* 配置 PA9(TX) 为复用推挽输出 */
    GPIO_InitStruct.Pin = VOICE_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(VOICE_TX_PORT, &GPIO_InitStruct);

    /* 配置 PA10(RX) 为浮空输入 */
    GPIO_InitStruct.Pin = VOICE_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(VOICE_RX_PORT, &GPIO_InitStruct);

    /* 配置 USART1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = VOICE_BAUDRATE;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart1);

    /* 使能 USART1 中断 */
    HAL_NVIC_SetPriority(USART1_IRQn, 2, 1);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    /* 开始接收 */
    HAL_UART_Receive_IT(&huart1, &s_uart_byte, 1);

    /* 初始化全局标志 */
    g_voice_cmd_ready = 0;
    g_voice_cmd_id = VOICE_CMD_NONE;
}

/**
 * @brief  CI1303 UART 接收中断回调
 * @note   每收到一个字节检查是否是有效指令
 */
void Voice_UART_RxCallback(uint8_t byte)
{
    /* CI1303 发送的指令编号范围 0x01 ~ 0x06 */
    if (byte >= VOICE_CMD_NAVIGATE && byte <= VOICE_CMD_STOP) {
        g_voice_cmd_id = byte;
        g_voice_cmd_ready = 1;
    }
}

/**
 * @brief  获取最新的语音指令编号（读后清零）
 */
uint8_t Voice_GetCommand(void)
{
    uint8_t cmd = VOICE_CMD_NONE;

    if (g_voice_cmd_ready) {
        cmd = g_voice_cmd_id;
        g_voice_cmd_id = VOICE_CMD_NONE;
        g_voice_cmd_ready = 0;
    }

    return cmd;
}
