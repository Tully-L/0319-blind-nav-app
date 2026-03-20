/**
 * @file    sim900a.c
 * @brief   SIM900A GSM/GPRS 模块驱动实现
 *
 * 工作原理：
 * 1. 通过 AT 指令集控制 SIM900A 模块
 * 2. 短信功能：AT+CMGF (文本模式) → AT+CMGS (发送短信)
 * 3. GPRS 功能：配置 APN → 激活 PDP → 建立 TCP 连接 → 发送数据
 *
 * AT 指令流程：
 * - 初始化：AT → ATE0 → AT+CPIN? → AT+CSQ
 * - 发短信：AT+CMGF=1 → AT+CMGS="号码" → 内容 → Ctrl+Z
 * - GPRS：AT+CGATT=1 → AT+CSTT="APN" → AT+CIICR → AT+CIFSR → AT+CIPSTART
 */

#include "sim900a.h"
#include <string.h>
#include <stdio.h>

/* 私有变量 */
static char s_rx_buf[SIM_RX_BUF_SIZE];
static volatile uint16_t s_rx_idx = 0;
static volatile uint8_t s_rx_complete = 0;  /* 收到 OK/ERROR/> 等结束标记 */
static uint8_t s_uart_byte;
static uint8_t s_tcp_connected = 0;

/* TCP 接收缓冲 */
static char s_tcp_rx_buf[256];
static volatile uint16_t s_tcp_rx_len = 0;

/* ========== 私有函数 ========== */

/**
 * @brief  清空接收缓冲区
 */
static void sim_clear_rx(void)
{
    s_rx_idx = 0;
    s_rx_complete = 0;
    memset(s_rx_buf, 0, SIM_RX_BUF_SIZE);
}

/**
 * @brief  发送 AT 指令并等待响应
 * @param  cmd: AT 指令字符串
 * @param  expected: 期望的响应关键字（如 "OK"）
 * @param  timeout: 超时时间 (ms)
 * @retval 0=收到期望响应, -1=超时或错误
 */
static int8_t sim_send_cmd(const char *cmd, const char *expected, uint32_t timeout)
{
    uint32_t start;

    sim_clear_rx();

    /* 发送指令 */
    HAL_UART_Transmit(&huart3, (uint8_t *)cmd, strlen(cmd), 1000);
    HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, 100);

    /* 等待响应 */
    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout) {
        if (s_rx_idx > 0 && strstr(s_rx_buf, expected) != NULL) {
            return 0;   /* 收到期望响应 */
        }
        if (strstr(s_rx_buf, "ERROR") != NULL) {
            return -1;  /* 收到错误 */
        }
        HAL_Delay(10);
    }
    return -1;  /* 超时 */
}

/**
 * @brief  发送原始数据（不加 \r\n）
 */
static void sim_send_raw(const char *data, uint16_t len)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)data, len, 2000);
}

/* ========== 公开函数 ========== */

/**
 * @brief  初始化 SIM900A 模块
 */
int8_t SIM900A_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    /* 配置 PB10(TX) 为复用推挽输出 */
    GPIO_InitStruct.Pin = SIM_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SIM_TX_PORT, &GPIO_InitStruct);

    /* 配置 PB11(RX) 为浮空输入 */
    GPIO_InitStruct.Pin = SIM_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SIM_RX_PORT, &GPIO_InitStruct);

    /* 配置 USART3 */
    huart3.Instance = USART3;
    huart3.Init.BaudRate = SIM_BAUDRATE;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart3);

    /* 使能 USART3 中断 */
    HAL_NVIC_SetPriority(USART3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    /* 开始接收 */
    HAL_UART_Receive_IT(&huart3, &s_uart_byte, 1);

    /* 等待模块启动 */
    HAL_Delay(3000);

    /* AT 握手测试 */
    if (sim_send_cmd("AT", "OK", SIM_AT_TIMEOUT) != 0) {
        return -1;  /* 模块无响应 */
    }

    /* 关闭回显 */
    sim_send_cmd("ATE0", "OK", SIM_AT_TIMEOUT);

    /* 检查 SIM 卡 */
    if (sim_send_cmd("AT+CPIN?", "READY", SIM_AT_TIMEOUT) != 0) {
        return -2;  /* SIM 卡未就绪 */
    }

    /* 设置短信为文本模式 */
    sim_send_cmd("AT+CMGF=1", "OK", SIM_AT_TIMEOUT);

    /* 设置短信字符集 */
    sim_send_cmd("AT+CSCS=\"GSM\"", "OK", SIM_AT_TIMEOUT);

    s_tcp_connected = 0;
    return 0;
}

/**
 * @brief  发送短信
 */
int8_t SIM900A_SendSMS(const char *phone, const char *message)
{
    char cmd[64];

    /* 组装发送指令 */
    snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"", phone);

    sim_clear_rx();
    HAL_UART_Transmit(&huart3, (uint8_t *)cmd, strlen(cmd), 1000);
    HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, 100);

    /* 等待 > 提示符 */
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 5000) {
        if (strstr(s_rx_buf, ">") != NULL) {
            break;
        }
        HAL_Delay(10);
    }

    if (strstr(s_rx_buf, ">") == NULL) {
        return -1;
    }

    /* 发送短信内容 */
    HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen(message), 2000);

    /* 发送 Ctrl+Z (0x1A) 结束 */
    uint8_t ctrl_z = 0x1A;
    sim_clear_rx();
    HAL_UART_Transmit(&huart3, &ctrl_z, 1, 100);

    /* 等待发送结果 */
    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < SIM_SMS_TIMEOUT) {
        if (strstr(s_rx_buf, "+CMGS") != NULL) {
            return 0;   /* 发送成功 */
        }
        if (strstr(s_rx_buf, "ERROR") != NULL) {
            return -1;
        }
        HAL_Delay(10);
    }

    return -1;
}

/**
 * @brief  建立 GPRS TCP 连接
 */
int8_t SIM900A_TCP_Connect(const char *ip, const char *port)
{
    char cmd[128];

    /* 关闭可能存在的旧连接 */
    sim_send_cmd("AT+CIPSHUT", "SHUT OK", 5000);

    /* 附着 GPRS */
    if (sim_send_cmd("AT+CGATT=1", "OK", 5000) != 0) {
        return -1;
    }

    /* 设置 APN */
    snprintf(cmd, sizeof(cmd), "AT+CSTT=\"%s\"", SIM_APN);
    if (sim_send_cmd(cmd, "OK", SIM_AT_TIMEOUT) != 0) {
        return -1;
    }

    /* 激活无线连接 */
    if (sim_send_cmd("AT+CIICR", "OK", 10000) != 0) {
        return -1;
    }

    /* 获取本地 IP */
    sim_clear_rx();
    HAL_UART_Transmit(&huart3, (uint8_t *)"AT+CIFSR\r\n", 10, 1000);
    HAL_Delay(2000);
    /* 只要没返回 ERROR 就算成功 */

    /* 建立 TCP 连接 */
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",\"%s\"", ip, port);
    if (sim_send_cmd(cmd, "CONNECT OK", SIM_TCP_TIMEOUT) != 0) {
        /* 某些固件版本返回 ALREADY CONNECT */
        if (strstr(s_rx_buf, "ALREADY CONNECT") == NULL) {
            return -1;
        }
    }

    s_tcp_connected = 1;
    return 0;
}

/**
 * @brief  通过 TCP 发送数据
 */
int8_t SIM900A_TCP_Send(const char *data, uint16_t len)
{
    char cmd[32];

    if (!s_tcp_connected) {
        return -1;
    }

    /* 发起发送请求 */
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d", len);
    sim_clear_rx();
    HAL_UART_Transmit(&huart3, (uint8_t *)cmd, strlen(cmd), 1000);
    HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, 100);

    /* 等待 > 提示符 */
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 5000) {
        if (strstr(s_rx_buf, ">") != NULL) break;
        HAL_Delay(10);
    }
    if (strstr(s_rx_buf, ">") == NULL) return -1;

    /* 发送数据 */
    sim_clear_rx();
    sim_send_raw(data, len);

    /* 等待发送确认 */
    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < SIM_TCP_TIMEOUT) {
        if (strstr(s_rx_buf, "SEND OK") != NULL) return 0;
        if (strstr(s_rx_buf, "ERROR") != NULL) return -1;
        HAL_Delay(10);
    }

    return -1;
}

/**
 * @brief  关闭 TCP 连接
 */
void SIM900A_TCP_Close(void)
{
    sim_send_cmd("AT+CIPCLOSE", "CLOSE OK", 5000);
    s_tcp_connected = 0;
}

/**
 * @brief  检查模块在线状态
 */
uint8_t SIM900A_IsOnline(void)
{
    return (sim_send_cmd("AT", "OK", 1000) == 0) ? 1 : 0;
}

/**
 * @brief  UART 接收中断回调
 */
void SIM900A_UART_RxCallback(uint8_t byte)
{
    if (s_rx_idx < SIM_RX_BUF_SIZE - 1) {
        s_rx_buf[s_rx_idx++] = byte;
        s_rx_buf[s_rx_idx] = '\0';
    }

    /*
     * 检测服务器下发数据（+IPD,<len>:<data>）
     * 简化处理：收到 +IPD 时将数据存入 TCP 接收缓冲
     */
    if (s_rx_idx > 5) {
        char *ipd = strstr(s_rx_buf, "+IPD,");
        if (ipd != NULL) {
            char *colon = strchr(ipd, ':');
            if (colon != NULL) {
                uint16_t data_len = (uint16_t)atoi(ipd + 5);
                uint16_t available = (uint16_t)(s_rx_idx - (colon + 1 - s_rx_buf));
                if (available >= data_len && data_len < sizeof(s_tcp_rx_buf)) {
                    memcpy(s_tcp_rx_buf, colon + 1, data_len);
                    s_tcp_rx_len = data_len;
                    s_tcp_rx_buf[data_len] = '\0';
                }
            }
        }
    }
}

/**
 * @brief  获取 TCP 接收到的数据
 */
uint16_t SIM900A_TCP_GetReceived(char *buf, uint16_t max_len)
{
    uint16_t len = s_tcp_rx_len;
    if (len == 0) return 0;

    if (len > max_len) len = max_len;
    memcpy(buf, s_tcp_rx_buf, len);
    s_tcp_rx_len = 0;  /* 清空 */
    return len;
}
