/**
 * @file    main.c
 * @brief   智能盲人辅助导航系统 - 主程序
 * @author  林雨 毕设项目
 *
 * 系统功能概述：
 * 1. 超声波测距 + 红外避障 → 分级预警（震动+蜂鸣器）
 * 2. GPS 定位 → 位置上报到云端 → 导航
 * 3. OV7670 摄像头 → 图片采集上传
 * 4. SIM900A → 短信 SOS + GPRS 数据传输
 * 5. CI1303 语音识别 → 语音控制
 * 6. SOS 按键 → 紧急求助
 *
 * 主循环任务调度（非RTOS，基于 SysTick 定时轮询）：
 * - 超声波测距：每 200ms
 * - 红外避障检测：每 100ms
 * - GPS 数据解析：每 1s
 * - 分级预警判断：每 200ms
 * - 云端位置上报：每 10s
 * - 低电量检测：每 30s
 * - 语音指令/SOS按键：中断方式，主循环检查标志
 *
 * 硬件平台：STM32F103C8T6 (72MHz, 20KB RAM, 64KB Flash)
 */

#include "main.h"
#include "ultrasonic.h"
#include "gps.h"
#include "infrared.h"
#include "camera.h"
#include "sim900a.h"
#include "voice.h"
#include "motor.h"
#include "buzzer.h"
#include "battery.h"
#include "warning.h"
#include "navigation.h"
#include "cloud.h"

/* ========== 全局外设句柄 ========== */
TIM_HandleTypeDef htim2;        /* 超声波输入捕获 */
TIM_HandleTypeDef htim3;        /* 马达 PWM */
UART_HandleTypeDef huart1;      /* CI1303 语音 */
UART_HandleTypeDef huart2;      /* GPS */
UART_HandleTypeDef huart3;      /* SIM900A */
ADC_HandleTypeDef hadc1;        /* 电池电压 */
I2C_HandleTypeDef hi2c1;        /* OV7670 SCCB */

/* ========== 全局标志位 ========== */
volatile uint8_t g_sos_triggered = 0;       /* SOS 按键触发标志 */
volatile uint8_t g_voice_cmd_ready = 0;     /* 语音指令就绪标志 */
volatile uint8_t g_voice_cmd_id = 0;        /* 语音指令编号 */

/* ========== 私有变量 ========== */
static uint32_t s_tick_ultrasonic = 0;      /* 超声波定时 */
static uint32_t s_tick_infrared = 0;        /* 红外定时 */
static uint32_t s_tick_gps = 0;             /* GPS 定时 */
static uint32_t s_tick_warning = 0;         /* 预警定时 */
static uint32_t s_tick_cloud = 0;           /* 云端上报定时 */
static uint32_t s_tick_battery = 0;         /* 电池检测定时 */

static uint16_t s_distance_cm = 0;          /* 当前超声波距离 */
static uint8_t  s_ir_obstacle = 0;          /* 当前红外障碍状态 */
static uint8_t  s_battery_percent = 100;    /* 当前电量百分比 */

/* ========== 函数声明 ========== */
static void SOS_Button_Init(void);
static void Handle_SOS(void);
static void Handle_VoiceCommand(uint8_t cmd);

/**
 * @brief  系统时钟配置
 * @note   HSE(8MHz) → PLL(x9) → SYSCLK(72MHz)
 *         AHB: 72MHz, APB1: 36MHz, APB2: 72MHz
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* 配置 HSE + PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;      /* 8MHz x 9 = 72MHz */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* 配置总线时钟 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     /* AHB 72MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;      /* APB1 36MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;      /* APB2 72MHz */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }

    /* 配置 ADC 时钟分频 (APB2/6 = 12MHz，不超过14MHz) */
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

/**
 * @brief  初始化 SOS 按键（PB13，外部中断，下降沿触发）
 */
static void SOS_Button_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = SOS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;    /* 下降沿触发（按下） */
    GPIO_InitStruct.Pull = GPIO_PULLUP;             /* 内部上拉 */
    HAL_GPIO_Init(SOS_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(SOS_EXTI_IRQn, 0, 0);     /* 最高优先级 */
    HAL_NVIC_EnableIRQ(SOS_EXTI_IRQn);
}

/**
 * @brief  处理 SOS 紧急求助
 * @note   1. 获取当前 GPS 位置
 *         2. 通过 SIM900A 发送 SOS 短信
 *         3. 通过 GPRS 上报 SOS 到云端
 *         4. 蜂鸣器持续报警 3 秒
 */
static void Handle_SOS(void)
{
    const GPS_Data_t *gps = GPS_GetData();
    char sms_buf[128];

    /* 蜂鸣器报警 */
    Buzzer_On();

    /* 构建短信内容 */
    if (gps->fix_valid) {
        snprintf(sms_buf, sizeof(sms_buf),
            "SOS! I need help! Location: lat=%.6f, lng=%.6f",
            gps->latitude, gps->longitude);
    } else {
        snprintf(sms_buf, sizeof(sms_buf),
            "SOS! I need help! GPS not available.");
    }

    /* 发送短信 */
    SIM900A_SendSMS(SOS_PHONE_NUMBER, sms_buf);

    /* 上报到云端 */
    Cloud_SendSOS(gps);

    /* 蜂鸣器响 3 秒 */
    HAL_Delay(3000);
    Buzzer_Off();
}

/**
 * @brief  处理语音指令
 * @param  cmd: 指令编号
 */
static void Handle_VoiceCommand(uint8_t cmd)
{
    const GPS_Data_t *gps;

    switch (cmd) {
        case VOICE_CMD_NAVIGATE:
            /* 开始导航 - TODO: 需要从云端获取目标位置 */
            /* 暂时使用固定目标点作为演示 */
            Navigation_SetTarget(39.9042, 116.4074);
            Buzzer_Alert(1);    /* 确认提示音 */
            break;

        case VOICE_CMD_QUERY_POS:
            /* 查询位置 - 触发位置上报 */
            gps = GPS_GetData();
            Cloud_ReportLocation(gps, s_battery_percent);
            Buzzer_Alert(1);
            break;

        case VOICE_CMD_SOS:
            /* 语音 SOS */
            Handle_SOS();
            break;

        case VOICE_CMD_BATTERY:
            /* 电量查询 - 用蜂鸣器次数表示十位数 */
            /* 例如 85% → 响 8 次，后续可改为语音播报 */
            Buzzer_Alert(s_battery_percent / 10);
            break;

        case VOICE_CMD_PHOTO:
            /* 拍照并上传 */
            {
                uint8_t img_buf[2400];  /* 40x30 缩略图 */
                uint16_t w, h;
                if (Camera_CaptureThumbnail(img_buf, sizeof(img_buf), &w, &h) == 0) {
                    Cloud_UploadImage(img_buf, w * h * 2);
                }
                Buzzer_Alert(1);
            }
            break;

        case VOICE_CMD_STOP:
            /* 停止导航 */
            Navigation_Stop();
            Motor_StopAll();
            Buzzer_Off();
            Buzzer_Alert(2);
            break;

        default:
            break;
    }
}

/**
 * @brief  主函数
 */
int main(void)
{
    uint8_t voice_cmd;

    /* ========== 系统初始化 ========== */
    HAL_Init();
    SystemClock_Config();

    /* ========== 外设初始化 ========== */
    Buzzer_Init();          /* 蜂鸣器（最先，用于指示启动） */
    Buzzer_Alert(2);        /* 启动提示：响两声 */

    Ultrasonic_Init();      /* 超声波 */
    GPS_Init();             /* GPS */
    Infrared_Init();        /* 红外避障 */
    Motor_Init();           /* 震动马达 */
    Battery_Init();         /* 电池 ADC */
    Voice_Init();           /* 语音识别 */
    SOS_Button_Init();      /* SOS 按键 */

    /* OV7670 初始化（可能失败，不影响其他功能） */
    Camera_Init();

    /* SIM900A 初始化（耗时较长） */
    SIM900A_Init();

    /* ========== 逻辑模块初始化 ========== */
    Warning_Init();
    Navigation_Init();

    /* 尝试建立云端连接（失败不影响本地功能） */
    Cloud_Init();

    /* 启动完成提示 */
    Buzzer_Alert(3);        /* 响三声表示系统就绪 */

    /* 记录初始时间 */
    s_tick_ultrasonic = HAL_GetTick();
    s_tick_infrared = HAL_GetTick();
    s_tick_gps = HAL_GetTick();
    s_tick_warning = HAL_GetTick();
    s_tick_cloud = HAL_GetTick();
    s_tick_battery = HAL_GetTick();

    /* ========== 主循环 ========== */
    while (1) {
        uint32_t now = HAL_GetTick();

        /* --- 1. 红外避障检测 (每 100ms) --- */
        if ((now - s_tick_infrared) >= INFRARED_PERIOD_MS) {
            s_tick_infrared = now;
            s_ir_obstacle = Infrared_HasObstacle();
        }

        /* --- 2. 超声波测距 (每 200ms) --- */
        if ((now - s_tick_ultrasonic) >= ULTRASONIC_PERIOD_MS) {
            s_tick_ultrasonic = now;
            s_distance_cm = Ultrasonic_Measure();
        }

        /* --- 3. GPS 数据解析 (每 1s) --- */
        if ((now - s_tick_gps) >= GPS_PERIOD_MS) {
            s_tick_gps = now;
            GPS_Process();
            Navigation_Update();
        }

        /* --- 4. 分级预警判断 (每 200ms) --- */
        if ((now - s_tick_warning) >= WARNING_PERIOD_MS) {
            s_tick_warning = now;
            Warning_Level_t level = Warning_Evaluate(s_distance_cm, s_ir_obstacle);
            Warning_Execute(level);
        }

        /* --- 5. 云端位置上报 (每 10s) --- */
        if ((now - s_tick_cloud) >= CLOUD_PERIOD_MS) {
            s_tick_cloud = now;
            const GPS_Data_t *gps = GPS_GetData();
            if (gps->fix_valid) {
                Cloud_ReportLocation(gps, s_battery_percent);
            }
            /* 检查云端下发数据 */
            Cloud_CheckReceived();
            /* 保持连接 */
            Cloud_KeepAlive();
        }

        /* --- 6. 低电量检测 (每 30s) --- */
        if ((now - s_tick_battery) >= BATTERY_PERIOD_MS) {
            s_tick_battery = now;
            s_battery_percent = Battery_GetPercent();
            if (Battery_IsLow()) {
                /* 低电量预警：蜂鸣器短响 + 震动 */
                Buzzer_Alert(5);
            }
        }

        /* --- 7. 处理 SOS 按键 (中断触发) --- */
        if (g_sos_triggered) {
            g_sos_triggered = 0;
            Handle_SOS();
        }

        /* --- 8. 处理语音指令 (中断触发) --- */
        voice_cmd = Voice_GetCommand();
        if (voice_cmd != VOICE_CMD_NONE) {
            Handle_VoiceCommand(voice_cmd);
        }

        /* GPS 数据持续接收处理（中断接收的数据在此解析） */
        GPS_Process();
    }
}

/**
 * @brief  错误处理函数
 * @note   出错时蜂鸣器长鸣，LED 闪烁
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {
        /* 实际项目中可以加看门狗复位 */
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* 用户可在此添加断言失败处理 */
}
#endif
