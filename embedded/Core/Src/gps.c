/**
 * @file    gps.c
 * @brief   ATGM336H GPS 模块驱动 - NMEA 协议解析实现
 *
 * 工作原理：
 * 1. USART2 中断方式逐字节接收 NMEA 数据
 * 2. 检测到换行符时标记一行完整
 * 3. GPS_Process() 解析 $GNRMC 和 $GNGGA 语句
 * 4. $GNRMC: 时间、定位状态、经纬度、速度
 * 5. $GNGGA: 卫星数、海拔
 *
 * NMEA 格式示例：
 * $GNRMC,083000.00,A,3954.2514,N,11624.4445,E,0.5,,190326,,,A*6E
 * $GNGGA,083000.00,3954.2514,N,11624.4445,E,1,08,1.0,50.0,M,,M,,*4B
 */

#include "gps.h"
#include <string.h>
#include <stdlib.h>

/* 私有变量 */
static GPS_Data_t s_gps_data = {0};
static char s_rx_buf[GPS_RX_BUF_SIZE];
static volatile uint16_t s_rx_idx = 0;
static volatile uint8_t s_line_ready = 0;
static char s_line_buf[GPS_RX_BUF_SIZE];
static uint8_t s_uart_byte;     /* 单字节接收缓冲 */

/* ========== 私有函数 ========== */

/**
 * @brief  从 NMEA 字段中获取下一个逗号分隔的字段
 * @param  str: 输入字符串指针的指针
 * @retval 当前字段起始指针（空字段返回空字符串""）
 */
static char* nmea_next_field(char **str)
{
    char *start = *str;
    char *p = *str;

    while (*p && *p != ',') {
        p++;
    }
    if (*p == ',') {
        *p = '\0';
        *str = p + 1;
    } else {
        *str = p;   /* 到末尾了 */
    }
    return start;
}

/**
 * @brief  将 NMEA 经纬度格式 (ddmm.mmmm) 转换为十进制度
 * @param  nmea_val: NMEA 格式数值 (如 3954.2514)
 * @retval 十进制度数 (如 39.903523)
 */
static double nmea_to_decimal(const char *nmea_val)
{
    double val = atof(nmea_val);
    int degrees = (int)(val / 100);
    double minutes = val - degrees * 100;
    return degrees + minutes / 60.0;
}

/**
 * @brief  解析 $GNRMC 语句
 * @note   $GNRMC,时间,状态,纬度,N/S,经度,E/W,速度,航向,日期,...
 */
static void parse_gnrmc(char *line)
{
    char *p = line;
    char *field;

    /* 跳过 $GNRMC */
    nmea_next_field(&p);

    /* 时间 hhmmss.ss */
    field = nmea_next_field(&p);
    if (strlen(field) >= 6) {
        s_gps_data.hour   = (field[0] - '0') * 10 + (field[1] - '0');
        s_gps_data.minute = (field[2] - '0') * 10 + (field[3] - '0');
        s_gps_data.second = (field[4] - '0') * 10 + (field[5] - '0');
    }

    /* 定位状态 A=有效, V=无效 */
    field = nmea_next_field(&p);
    s_gps_data.fix_valid = (field[0] == 'A') ? 1 : 0;

    /* 纬度 */
    field = nmea_next_field(&p);
    if (strlen(field) > 0) {
        s_gps_data.latitude = nmea_to_decimal(field);
    }

    /* N/S */
    field = nmea_next_field(&p);
    if (field[0] == 'S') {
        s_gps_data.latitude = -s_gps_data.latitude;
    }
    s_gps_data.ns = field[0];

    /* 经度 */
    field = nmea_next_field(&p);
    if (strlen(field) > 0) {
        s_gps_data.longitude = nmea_to_decimal(field);
    }

    /* E/W */
    field = nmea_next_field(&p);
    if (field[0] == 'W') {
        s_gps_data.longitude = -s_gps_data.longitude;
    }
    s_gps_data.ew = field[0];

    /* 速度 (节 → km/h) */
    field = nmea_next_field(&p);
    if (strlen(field) > 0) {
        s_gps_data.speed = (float)(atof(field) * 1.852);   /* 1节 = 1.852 km/h */
    }
}

/**
 * @brief  解析 $GNGGA 语句
 * @note   $GNGGA,时间,纬度,N,经度,E,质量,卫星数,HDOP,海拔,M,...
 */
static void parse_gngga(char *line)
{
    char *p = line;
    char *field;
    int i;

    /* 跳过前 7 个字段（到卫星数） */
    for (i = 0; i < 7; i++) {
        field = nmea_next_field(&p);
    }

    /* 卫星数 */
    field = nmea_next_field(&p);
    if (strlen(field) > 0) {
        s_gps_data.satellites = (uint8_t)atoi(field);
    }

    /* HDOP（跳过） */
    nmea_next_field(&p);

    /* 海拔 */
    field = nmea_next_field(&p);
    if (strlen(field) > 0) {
        s_gps_data.altitude = (float)atof(field);
    }
}

/* ========== 公开函数 ========== */

/**
 * @brief  初始化 GPS 模块
 */
void GPS_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    /* 配置 PA2(TX) 为复用推挽输出 */
    GPIO_InitStruct.Pin = GPS_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPS_TX_PORT, &GPIO_InitStruct);

    /* 配置 PA3(RX) 为浮空输入 */
    GPIO_InitStruct.Pin = GPS_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPS_RX_PORT, &GPIO_InitStruct);

    /* 配置 USART2 */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = GPS_BAUDRATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart2);

    /* 使能 USART2 中断 */
    HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    /* 开始接收第一个字节 */
    HAL_UART_Receive_IT(&huart2, &s_uart_byte, 1);

    /* 初始化数据 */
    memset(&s_gps_data, 0, sizeof(s_gps_data));
}

/**
 * @brief  GPS UART 接收中断回调（每接收一字节调用一次）
 */
void GPS_UART_RxCallback(uint8_t byte)
{
    if (byte == '$') {
        /* 新语句开始，重置索引 */
        s_rx_idx = 0;
    }

    if (s_rx_idx < GPS_RX_BUF_SIZE - 1) {
        s_rx_buf[s_rx_idx++] = byte;
    }

    if (byte == '\n') {
        /* 一行接收完成 */
        s_rx_buf[s_rx_idx] = '\0';
        memcpy(s_line_buf, s_rx_buf, s_rx_idx + 1);
        s_line_ready = 1;
        s_rx_idx = 0;
    }
}

/**
 * @brief  处理 GPS 数据（主循环中调用）
 */
void GPS_Process(void)
{
    if (!s_line_ready) {
        return;
    }
    s_line_ready = 0;

    /* 去掉校验和部分（*号之后） */
    char *asterisk = strchr(s_line_buf, '*');
    if (asterisk) {
        *asterisk = '\0';
    }

    /* 解析不同类型的 NMEA 语句 */
    if (strncmp(s_line_buf, "$GNRMC", 6) == 0 || strncmp(s_line_buf, "$GPRMC", 6) == 0) {
        parse_gnrmc(s_line_buf);
    } else if (strncmp(s_line_buf, "$GNGGA", 6) == 0 || strncmp(s_line_buf, "$GPGGA", 6) == 0) {
        parse_gngga(s_line_buf);
    }
}

/**
 * @brief  获取当前 GPS 数据
 */
const GPS_Data_t* GPS_GetData(void)
{
    return &s_gps_data;
}

/**
 * @brief  检查 GPS 是否已定位
 */
uint8_t GPS_IsFixed(void)
{
    return s_gps_data.fix_valid;
}
