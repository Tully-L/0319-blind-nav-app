/**
 * @file    cloud.c
 * @brief   云端通信模块实现
 *
 * 通信协议（JSON over TCP）：
 *
 * 1. 位置上报：
 *    {"type":"location","lat":39.9042,"lng":116.4074,"speed":1.2,"battery":85,"ts":1679000000}
 *
 * 2. SOS 告警：
 *    {"type":"sos","lat":39.9042,"lng":116.4074,"ts":1679000000}
 *
 * 3. 图像上传（Base64）：
 *    {"type":"image","data":"<base64>","ts":1679000000}
 *
 * 注意：SIM900A 的 GPRS 带宽约 40-50 Kbps，上传大图片非常慢
 *       建议只上传缩略图（40x30 RGB565 ≈ 2.4KB → Base64 ≈ 3.2KB）
 */

#include "cloud.h"
#include "sim900a.h"
#include <string.h>
#include <stdio.h>

/* Base64 编码表 */
static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* JSON 构建缓冲区 */
static char s_json_buf[512];

/* 连接状态 */
static uint8_t s_connected = 0;

/* ========== 私有函数 ========== */

/**
 * @brief  获取简易时间戳（使用 SysTick，非真实 Unix 时间）
 * @note   实际项目中应使用 GPS 时间或 RTC
 */
static uint32_t get_timestamp(void)
{
    return HAL_GetTick() / 1000;
}

/**
 * @brief  Base64 编码
 * @param  src: 源数据
 * @param  src_len: 源数据长度
 * @param  dst: 目标缓冲区
 * @param  dst_size: 目标缓冲区大小
 * @retval 编码后长度，-1=缓冲区不够
 */
static int16_t base64_encode(const uint8_t *src, uint16_t src_len,
                              char *dst, uint16_t dst_size)
{
    uint16_t i, j;
    uint16_t out_len = ((src_len + 2) / 3) * 4;

    if (out_len + 1 > dst_size) return -1;

    for (i = 0, j = 0; i < src_len; ) {
        uint32_t a = (i < src_len) ? src[i++] : 0;
        uint32_t b = (i < src_len) ? src[i++] : 0;
        uint32_t c = (i < src_len) ? src[i++] : 0;
        uint32_t triple = (a << 16) | (b << 8) | c;

        dst[j++] = base64_table[(triple >> 18) & 0x3F];
        dst[j++] = base64_table[(triple >> 12) & 0x3F];
        dst[j++] = base64_table[(triple >> 6) & 0x3F];
        dst[j++] = base64_table[triple & 0x3F];
    }

    /* 填充 '=' */
    if (src_len % 3 == 1) {
        dst[j - 1] = '=';
        dst[j - 2] = '=';
    } else if (src_len % 3 == 2) {
        dst[j - 1] = '=';
    }

    dst[j] = '\0';
    return (int16_t)j;
}

/* ========== 公开函数 ========== */

/**
 * @brief  初始化云端通信
 */
int8_t Cloud_Init(void)
{
    /* 尝试建立 TCP 连接 */
    if (SIM900A_TCP_Connect(CLOUD_SERVER_IP, CLOUD_SERVER_PORT) != 0) {
        s_connected = 0;
        return -1;
    }

    s_connected = 1;
    return 0;
}

/**
 * @brief  上报当前位置
 */
int8_t Cloud_ReportLocation(const GPS_Data_t *gps, uint8_t battery_percent)
{
    int len;

    if (!s_connected || !gps->fix_valid) {
        return -1;
    }

    /* 构建 JSON */
    len = snprintf(s_json_buf, sizeof(s_json_buf),
        "{\"type\":\"location\","
        "\"lat\":%.6f,"
        "\"lng\":%.6f,"
        "\"speed\":%.1f,"
        "\"battery\":%d,"
        "\"ts\":%lu}",
        gps->latitude, gps->longitude,
        (double)gps->speed,
        battery_percent,
        (unsigned long)get_timestamp());

    if (len <= 0 || len >= (int)sizeof(s_json_buf)) {
        return -1;
    }

    return SIM900A_TCP_Send(s_json_buf, (uint16_t)len);
}

/**
 * @brief  发送 SOS 告警
 */
int8_t Cloud_SendSOS(const GPS_Data_t *gps)
{
    int len;

    if (!s_connected) {
        return -1;
    }

    len = snprintf(s_json_buf, sizeof(s_json_buf),
        "{\"type\":\"sos\","
        "\"lat\":%.6f,"
        "\"lng\":%.6f,"
        "\"ts\":%lu}",
        gps->fix_valid ? gps->latitude : 0.0,
        gps->fix_valid ? gps->longitude : 0.0,
        (unsigned long)get_timestamp());

    if (len <= 0 || len >= (int)sizeof(s_json_buf)) {
        return -1;
    }

    return SIM900A_TCP_Send(s_json_buf, (uint16_t)len);
}

/**
 * @brief  上传图片数据
 * @note   由于 JSON 缓冲区和 GPRS 带宽限制，一次最多约 300 字节原始数据
 *         缩略图 40x30 = 2400 字节需要分包发送
 *         这里简化为只发送前 300 字节的示例
 */
int8_t Cloud_UploadImage(const uint8_t *img_data, uint16_t data_len)
{
    char b64_buf[512];
    int len;
    uint16_t chunk_size;

    if (!s_connected || img_data == NULL || data_len == 0) {
        return -1;
    }

    /* 限制单次发送大小（Base64 膨胀 4/3 + JSON 开销） */
    chunk_size = (data_len > 300) ? 300 : data_len;

    /* Base64 编码 */
    if (base64_encode(img_data, chunk_size, b64_buf, sizeof(b64_buf)) < 0) {
        return -1;
    }

    /* 构建 JSON */
    len = snprintf(s_json_buf, sizeof(s_json_buf),
        "{\"type\":\"image\","
        "\"data\":\"%s\","
        "\"ts\":%lu}",
        b64_buf,
        (unsigned long)get_timestamp());

    if (len <= 0 || len >= (int)sizeof(s_json_buf)) {
        return -1;
    }

    return SIM900A_TCP_Send(s_json_buf, (uint16_t)len);
}

/**
 * @brief  检查云端下发数据
 */
uint8_t Cloud_CheckReceived(void)
{
    char buf[256];
    uint16_t len;

    len = SIM900A_TCP_GetReceived(buf, sizeof(buf) - 1);
    if (len == 0) {
        return 0;
    }

    buf[len] = '\0';

    /*
     * TODO: 解析云端下发的指令
     * 例如：
     * {"cmd":"set_target","lat":39.90,"lng":116.40}  → 设置导航目标
     * {"cmd":"photo"}                                 → 请求拍照
     * {"cmd":"sos_ack"}                               → SOS 确认
     *
     * 简化版暂不实现解析，预留接口
     */

    return 1;
}

/**
 * @brief  保持连接（断线重连）
 */
void Cloud_KeepAlive(void)
{
    if (!s_connected) {
        /* 尝试重连 */
        if (SIM900A_TCP_Connect(CLOUD_SERVER_IP, CLOUD_SERVER_PORT) == 0) {
            s_connected = 1;
        }
    }
}
