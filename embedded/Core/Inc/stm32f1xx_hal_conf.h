/**
 * @file    stm32f1xx_hal_conf.h
 * @brief   HAL 库配置文件 - 启用本项目需要的 HAL 模块
 */

#ifndef __STM32F1XX_HAL_CONF_H
#define __STM32F1XX_HAL_CONF_H

/* ========== 模块启用 ========== */
#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED

/* ========== 时钟配置 ========== */
/* 外部高速晶振频率 (8MHz) */
#if !defined(HSE_VALUE)
  #define HSE_VALUE    8000000U
#endif

#if !defined(HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    100U
#endif

/* 内部高速 RC 振荡器频率 (8MHz) */
#if !defined(HSI_VALUE)
  #define HSI_VALUE    8000000U
#endif

/* 外部低速晶振频率 (32.768KHz) */
#if !defined(LSE_VALUE)
  #define LSE_VALUE    32768U
#endif

#if !defined(LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    5000U
#endif

/* ========== 系统配置 ========== */
#define VDD_VALUE                  3300U   /* VDD 电压 3.3V (mV) */
#define TICK_INT_PRIORITY          0x0FU   /* SysTick 中断优先级 */
#define USE_RTOS                   0U
#define PREFETCH_ENABLE            1U

/* ========== 引入 HAL 模块头文件 ========== */
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f1xx_hal_rcc.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f1xx_hal_gpio.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f1xx_hal_dma.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f1xx_hal_cortex.h"
#endif

#ifdef HAL_ADC_MODULE_ENABLED
  #include "stm32f1xx_hal_adc.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f1xx_hal_flash.h"
#endif

#ifdef HAL_I2C_MODULE_ENABLED
  #include "stm32f1xx_hal_i2c.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32f1xx_hal_pwr.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
  #include "stm32f1xx_hal_tim.h"
#endif

#ifdef HAL_UART_MODULE_ENABLED
  #include "stm32f1xx_hal_uart.h"
#endif

/* ========== 断言配置 ========== */
/* #define USE_FULL_ASSERT    1U */

#ifdef USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif

#endif /* __STM32F1XX_HAL_CONF_H */
