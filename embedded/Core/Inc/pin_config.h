/**
 * @file    pin_config.h
 * @brief   智能盲人辅助导航系统 - 引脚集中定义
 * @note    所有外设引脚在此统一管理，避免冲突
 *          基于 STM32F103C8T6 (48pin LQFP) 引脚资源
 *
 * 引脚分配总览：
 * ┌────────────┬──────────┬──────────────────────────────┐
 * │ PA0        │ OUTPUT   │ 超声波 Trig                  │
 * │ PA1        │ TIM2_CH2 │ 超声波 Echo (输入捕获)       │
 * │ PA2        │ USART2TX │ GPS 模块 TX                  │
 * │ PA3        │ USART2RX │ GPS 模块 RX                  │
 * │ PA4        │ ADC1_CH4 │ 电池电压采集                 │
 * │ PA5        │ INPUT    │ OV7670 PCLK                  │
 * │ PA6        │ TIM3_CH1 │ 震动马达 左                  │
 * │ PA7        │ TIM3_CH2 │ 震动马达 中                  │
 * │ PA8        │ INPUT    │ OV7670 D0                    │
 * │ PA9        │ USART1TX │ CI1303 语音模块 TX           │
 * │ PA10       │ USART1RX │ CI1303 语音模块 RX           │
 * │ PA11       │ INPUT    │ OV7670 D3                    │
 * │ PA12       │ INPUT    │ OV7670 D4                    │
 * │ PA13       │ SWD      │ SWDIO (调试保留)             │
 * │ PA14       │ SWD      │ SWCLK (调试保留)             │
 * │ PA15       │ INPUT    │ OV7670 D7 (需关闭JTAG)      │
 * │ PB0        │ TIM3_CH3 │ 震动马达 右                  │
 * │ PB1        │ INPUT    │ 红外避障 左                  │
 * │ PB3        │ INPUT    │ 红外避障 右 (需关闭JTAG)     │
 * │ PB4        │ INPUT    │ OV7670 D1 (需关闭JTAG)      │
 * │ PB5        │ INPUT    │ OV7670 D2                    │
 * │ PB6        │ I2C1_SCL │ OV7670 SCCB 时钟             │
 * │ PB7        │ I2C1_SDA │ OV7670 SCCB 数据             │
 * │ PB8        │ INPUT    │ OV7670 VSYNC                 │
 * │ PB9        │ INPUT    │ OV7670 HREF                  │
 * │ PB10       │ USART3TX │ SIM900A TX                   │
 * │ PB11       │ USART3RX │ SIM900A RX                   │
 * │ PB12       │ OUTPUT   │ 蜂鸣器                       │
 * │ PB13       │ EXTI     │ SOS 按键                     │
 * │ PB14       │ INPUT    │ OV7670 D5                    │
 * │ PB15       │ INPUT    │ OV7670 D6                    │
 * └────────────┴──────────┴──────────────────────────────┘
 *
 * 冲突解决说明：
 * 1. 红外避障从 PB0/PB1 移到 PB1/PB3，避开 TIM3_CH3(PB0)
 * 2. OV7670 PCLK 从 PB10 移到 PA5，避开 USART3(PB10/PB11)
 * 3. OV7670 D0-D7 分散分配，避开 PA9/PA10(USART1) 和 PA13/PA14(SWD)
 * 4. 需要关闭 JTAG 保留 SWD，释放 PA15/PB3/PB4
 */

#ifndef __PIN_CONFIG_H
#define __PIN_CONFIG_H

#include "stm32f1xx_hal.h"

/* ========== 超声波模块 HC-SR04 ========== */
#define ULTRA_TRIG_PORT         GPIOA
#define ULTRA_TRIG_PIN          GPIO_PIN_0
#define ULTRA_ECHO_PORT         GPIOA
#define ULTRA_ECHO_PIN          GPIO_PIN_1
/* Echo 使用 TIM2_CH2 输入捕获 */
#define ULTRA_TIM               TIM2
#define ULTRA_TIM_CHANNEL       TIM_CHANNEL_2

/* ========== GPS 模块 ATGM336H (USART2) ========== */
#define GPS_UART                USART2
#define GPS_TX_PORT             GPIOA
#define GPS_TX_PIN              GPIO_PIN_2
#define GPS_RX_PORT             GPIOA
#define GPS_RX_PIN              GPIO_PIN_3
#define GPS_BAUDRATE            9600

/* ========== 电池电压 ADC ========== */
#define BATTERY_ADC             ADC1
#define BATTERY_ADC_CHANNEL     ADC_CHANNEL_4
#define BATTERY_PORT            GPIOA
#define BATTERY_PIN             GPIO_PIN_4

/* ========== 震动马达 PWM (TIM3) ========== */
#define MOTOR_TIM               TIM3
#define MOTOR_LEFT_PORT         GPIOA
#define MOTOR_LEFT_PIN          GPIO_PIN_6
#define MOTOR_LEFT_CHANNEL      TIM_CHANNEL_1
#define MOTOR_CENTER_PORT       GPIOA
#define MOTOR_CENTER_PIN        GPIO_PIN_7
#define MOTOR_CENTER_CHANNEL    TIM_CHANNEL_2
#define MOTOR_RIGHT_PORT        GPIOB
#define MOTOR_RIGHT_PIN         GPIO_PIN_0
#define MOTOR_RIGHT_CHANNEL     TIM_CHANNEL_3

/* ========== 红外避障模块 ========== */
#define IR_LEFT_PORT            GPIOB
#define IR_LEFT_PIN             GPIO_PIN_1
#define IR_RIGHT_PORT           GPIOB
#define IR_RIGHT_PIN            GPIO_PIN_3  /* 需关闭 JTAG */

/* ========== OV7670 摄像头 ========== */
/* SCCB (I2C1) */
#define CAM_SCL_PORT            GPIOB
#define CAM_SCL_PIN             GPIO_PIN_6
#define CAM_SDA_PORT            GPIOB
#define CAM_SDA_PIN             GPIO_PIN_7
/* 控制信号 */
#define CAM_VSYNC_PORT          GPIOB
#define CAM_VSYNC_PIN           GPIO_PIN_8
#define CAM_HREF_PORT           GPIOB
#define CAM_HREF_PIN            GPIO_PIN_9
#define CAM_PCLK_PORT           GPIOA
#define CAM_PCLK_PIN            GPIO_PIN_5
/* 数据线 D0-D7（分散在不同端口） */
#define CAM_D0_PORT             GPIOA       /* PA8  */
#define CAM_D0_PIN              GPIO_PIN_8
#define CAM_D1_PORT             GPIOB       /* PB4  需关闭JTAG */
#define CAM_D1_PIN              GPIO_PIN_4
#define CAM_D2_PORT             GPIOB       /* PB5  */
#define CAM_D2_PIN              GPIO_PIN_5
#define CAM_D3_PORT             GPIOA       /* PA11 */
#define CAM_D3_PIN              GPIO_PIN_11
#define CAM_D4_PORT             GPIOA       /* PA12 */
#define CAM_D4_PIN              GPIO_PIN_12
#define CAM_D5_PORT             GPIOB       /* PB14 */
#define CAM_D5_PIN              GPIO_PIN_14
#define CAM_D6_PORT             GPIOB       /* PB15 */
#define CAM_D6_PIN              GPIO_PIN_15
#define CAM_D7_PORT             GPIOA       /* PA15 需关闭JTAG */
#define CAM_D7_PIN              GPIO_PIN_15

/* OV7670 SCCB 设备地址 (7位地址左移1位) */
#define OV7670_ADDR             0x42

/* ========== SIM900A 模块 (USART3) ========== */
#define SIM_UART                USART3
#define SIM_TX_PORT             GPIOB
#define SIM_TX_PIN              GPIO_PIN_10
#define SIM_RX_PORT             GPIOB
#define SIM_RX_PIN              GPIO_PIN_11
#define SIM_BAUDRATE            115200

/* ========== CI1303 语音识别模块 (USART1) ========== */
#define VOICE_UART              USART1
#define VOICE_TX_PORT           GPIOA
#define VOICE_TX_PIN            GPIO_PIN_9
#define VOICE_RX_PORT           GPIOA
#define VOICE_RX_PIN            GPIO_PIN_10
#define VOICE_BAUDRATE          9600

/* ========== 蜂鸣器 ========== */
#define BUZZER_PORT             GPIOB
#define BUZZER_PIN              GPIO_PIN_12

/* ========== SOS 按键 ========== */
#define SOS_PORT                GPIOB
#define SOS_PIN                 GPIO_PIN_13
#define SOS_EXTI_IRQn           EXTI15_10_IRQn

/* ========== 时钟使能宏 ========== */
#define __GPIOA_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()
#define __GPIOB_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()

#endif /* __PIN_CONFIG_H */
