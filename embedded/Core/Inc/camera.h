/**
 * @file    camera.h
 * @brief   OV7670 摄像头模块驱动（简化版）
 * @note    SCCB(I2C1): PB6(SCL)/PB7(SDA)
 *          数据线 D0-D7: 分散在 PA/PB 端口
 *          控制线: PA5(PCLK) PB8(VSYNC) PB9(HREF)
 *
 *          !! 重要限制 !!
 *          STM32F103C8T6 只有 20KB RAM
 *          QQVGA(160x120) RGB565 = 38400 字节，无法存完整帧
 *          本驱动采用逐行扫描方式，每次只缓存一行数据(320字节)
 */

#ifndef __CAMERA_H
#define __CAMERA_H

#include "main.h"

/* 图像参数 */
#define CAM_IMG_WIDTH       160     /* QQVGA 宽度 */
#define CAM_IMG_HEIGHT      120     /* QQVGA 高度 */
#define CAM_BYTES_PER_PIXEL 2       /* RGB565 每像素2字节 */
#define CAM_LINE_BUF_SIZE   (CAM_IMG_WIDTH * CAM_BYTES_PER_PIXEL)  /* 320字节/行 */

/**
 * @brief  初始化 OV7670 摄像头（SCCB 通信 + 寄存器配置）
 * @retval 0=成功, -1=失败（通信错误或设备ID不匹配）
 */
int8_t Camera_Init(void);

/**
 * @brief  采集一行图像数据
 * @param  line_num: 行号 (0 ~ CAM_IMG_HEIGHT-1)
 * @param  buf: 输出缓冲区，至少 CAM_LINE_BUF_SIZE 字节
 * @retval 0=成功, -1=超时
 */
int8_t Camera_CaptureLine(uint16_t line_num, uint8_t *buf);

/**
 * @brief  采集一帧缩略图（大幅降采样，适合有限RAM）
 * @param  buf: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @param  out_width: 输出实际宽度
 * @param  out_height: 输出实际高度
 * @retval 0=成功, -1=失败
 * @note   会降采样到 40x30 (2400字节)，适合通过 GPRS 上传
 */
int8_t Camera_CaptureThumbnail(uint8_t *buf, uint16_t buf_size,
                                uint16_t *out_width, uint16_t *out_height);

/**
 * @brief  SCCB 写寄存器
 * @param  reg: 寄存器地址
 * @param  val: 写入值
 * @retval HAL 状态
 */
HAL_StatusTypeDef Camera_WriteReg(uint8_t reg, uint8_t val);

/**
 * @brief  SCCB 读寄存器
 * @param  reg: 寄存器地址
 * @param  val: 读出值
 * @retval HAL 状态
 */
HAL_StatusTypeDef Camera_ReadReg(uint8_t reg, uint8_t *val);

#endif /* __CAMERA_H */
