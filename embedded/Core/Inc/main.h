/**
 * @file    main.h
 * @brief   智能盲人辅助导航系统 - 主头文件
 */

#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f1xx_hal.h"
#include "pin_config.h"

/* ========== 系统参数 ========== */
#define SYS_TICK_MS             1       /* SysTick 周期 1ms */

/* 主循环各任务执行周期 (ms) */
#define ULTRASONIC_PERIOD_MS    200     /* 超声波测距周期 */
#define INFRARED_PERIOD_MS      100     /* 红外避障检测周期 */
#define GPS_PERIOD_MS           1000    /* GPS 数据解析周期 */
#define WARNING_PERIOD_MS       200     /* 预警判断周期 */
#define CLOUD_PERIOD_MS         10000   /* 云端上报周期 */
#define BATTERY_PERIOD_MS       30000   /* 电池检测周期 */

/* ========== 预警距离阈值 (cm) ========== */
#define WARN_DIST_MILD          100     /* 轻度预警：> 100cm */
#define WARN_DIST_MODERATE      50      /* 中度预警：50-100cm */
/* < 50cm 为紧急预警 */

/* ========== 电池阈值 ========== */
#define BATTERY_LOW_PERCENT     20      /* 低电量阈值 20% */

/* ========== 云端服务器配置 ========== */
#define CLOUD_SERVER_IP         "123.456.789.0"  /* 替换为实际服务器IP */
#define CLOUD_SERVER_PORT       "8080"           /* 替换为实际端口 */

/* ========== SOS 紧急联系人 ========== */
#define SOS_PHONE_NUMBER        "13800138000"    /* 替换为实际号码 */

/* ========== 全局外设句柄声明 ========== */
extern TIM_HandleTypeDef htim2;     /* 超声波输入捕获 */
extern TIM_HandleTypeDef htim3;     /* 马达 PWM */
extern UART_HandleTypeDef huart1;   /* CI1303 语音 */
extern UART_HandleTypeDef huart2;   /* GPS */
extern UART_HandleTypeDef huart3;   /* SIM900A */
extern ADC_HandleTypeDef hadc1;     /* 电池电压 */
extern I2C_HandleTypeDef hi2c1;     /* OV7670 SCCB */

/* ========== 全局标志位 ========== */
extern volatile uint8_t g_sos_triggered;    /* SOS 按键触发标志 */
extern volatile uint8_t g_voice_cmd_ready;  /* 语音指令就绪标志 */
extern volatile uint8_t g_voice_cmd_id;     /* 语音指令编号 */

/* ========== 函数声明 ========== */
void SystemClock_Config(void);
void Error_Handler(void);

#endif /* __MAIN_H */
