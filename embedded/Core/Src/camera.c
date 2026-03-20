/**
 * @file    camera.c
 * @brief   OV7670 摄像头模块驱动实现（简化版）
 *
 * 工作原理：
 * 1. 通过 SCCB (兼容I2C) 协议配置 OV7670 寄存器
 * 2. OV7670 输出 QQVGA(160x120) RGB565 格式图像
 * 3. VSYNC 下降沿表示新帧开始
 * 4. HREF 高电平期间为有效像素数据
 * 5. 每个 PCLK 上升沿采集一个字节（RGB565 每像素2字节）
 *
 * 限制说明：
 * - F103C8T6 只有 20KB RAM，无法存完整帧
 * - 采用逐行采集方式，外部调用者需要逐行处理
 * - 数据线分散在不同端口，读取速度有限
 * - 本驱动为毕设演示用简化版本
 */

#include "camera.h"

/* ========== OV7670 寄存器地址 ========== */
#define REG_PID         0x0A    /* 产品ID高字节 (应为 0x76) */
#define REG_VER         0x0B    /* 产品ID低字节 (应为 0x73) */
#define REG_COM7        0x12    /* 控制寄存器7 */
#define REG_COM3        0x0C    /* 控制寄存器3 */
#define REG_COM14       0x3E    /* 控制寄存器14 */
#define REG_SCALING_XSC 0x70    /* 缩放X */
#define REG_SCALING_YSC 0x71    /* 缩放Y */
#define REG_SCALING_DCWCTR  0x72
#define REG_SCALING_PCLK_DIV 0x73
#define REG_SCALING_PCLK_DELAY 0xA2
#define REG_COM10       0x15    /* 控制寄存器10 */
#define REG_CLKRC       0x11    /* 时钟控制 */
#define REG_COM1        0x04
#define REG_HSTART      0x17
#define REG_HSTOP       0x18
#define REG_VSTART      0x19
#define REG_VSTOP       0x1A
#define REG_HREF        0x32
#define REG_TSLB        0x3A
#define REG_COM15       0x40

/* OV7670 QQVGA + RGB565 初始化寄存器表 */
static const uint8_t ov7670_init_regs[][2] = {
    /* 软复位 */
    {REG_COM7,  0x80},

    /* 时钟分频：内部时钟不分频 */
    {REG_CLKRC, 0x01},

    /* RGB565 输出 + QQVGA */
    {REG_COM7,  0x04},      /* RGB 输出 */
    {REG_COM15,  0xD0},     /* RGB565 格式 */
    {REG_TSLB,  0x04},      /* 输出格式 UV */

    /* QQVGA 缩放配置 */
    {REG_COM3,  0x04},      /* 使能缩放 */
    {REG_COM14, 0x1A},      /* 手动缩放, PCLK/4 */
    {REG_SCALING_XSC, 0x3A},
    {REG_SCALING_YSC, 0x35},
    {REG_SCALING_DCWCTR, 0x22},     /* 水平和垂直 1/4 */
    {REG_SCALING_PCLK_DIV, 0xF2},   /* PCLK 分频 */
    {REG_SCALING_PCLK_DELAY, 0x02},

    /* 窗口设置 */
    {REG_HSTART, 0x16},
    {REG_HSTOP,  0x04},
    {REG_HREF,   0x24},
    {REG_VSTART, 0x02},
    {REG_VSTOP,  0x7A},

    /* VSYNC 低电平有效, HREF 高电平有效 */
    {REG_COM10, 0x02},

    /* 结束标记 */
    {0xFF, 0xFF}
};

/* ========== SCCB (I2C) 通信 ========== */

/**
 * @brief  SCCB 写寄存器
 */
HAL_StatusTypeDef Camera_WriteReg(uint8_t reg, uint8_t val)
{
    uint8_t data[2] = {reg, val};
    return HAL_I2C_Master_Transmit(&hi2c1, OV7670_ADDR, data, 2, 100);
}

/**
 * @brief  SCCB 读寄存器
 */
HAL_StatusTypeDef Camera_ReadReg(uint8_t reg, uint8_t *val)
{
    HAL_StatusTypeDef ret;

    /* 先写寄存器地址 */
    ret = HAL_I2C_Master_Transmit(&hi2c1, OV7670_ADDR, &reg, 1, 100);
    if (ret != HAL_OK) return ret;

    /* 再读数据 */
    return HAL_I2C_Master_Receive(&hi2c1, OV7670_ADDR | 0x01, val, 1, 100);
}

/**
 * @brief  从并行数据线读取一个字节
 * @note   数据线分散在不同引脚，需要逐位组装
 */
static uint8_t Camera_ReadDataByte(void)
{
    uint8_t data = 0;

    /* D0 = PA8  */
    if (HAL_GPIO_ReadPin(CAM_D0_PORT, CAM_D0_PIN)) data |= 0x01;
    /* D1 = PB4  */
    if (HAL_GPIO_ReadPin(CAM_D1_PORT, CAM_D1_PIN)) data |= 0x02;
    /* D2 = PB5  */
    if (HAL_GPIO_ReadPin(CAM_D2_PORT, CAM_D2_PIN)) data |= 0x04;
    /* D3 = PA11 */
    if (HAL_GPIO_ReadPin(CAM_D3_PORT, CAM_D3_PIN)) data |= 0x08;
    /* D4 = PA12 */
    if (HAL_GPIO_ReadPin(CAM_D4_PORT, CAM_D4_PIN)) data |= 0x10;
    /* D5 = PB14 */
    if (HAL_GPIO_ReadPin(CAM_D5_PORT, CAM_D5_PIN)) data |= 0x20;
    /* D6 = PB15 */
    if (HAL_GPIO_ReadPin(CAM_D6_PORT, CAM_D6_PIN)) data |= 0x40;
    /* D7 = PA15 */
    if (HAL_GPIO_ReadPin(CAM_D7_PORT, CAM_D7_PIN)) data |= 0x80;

    return data;
}

/* ========== 公开函数 ========== */

/**
 * @brief  初始化 OV7670 摄像头
 */
int8_t Camera_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    uint8_t pid, ver;
    uint16_t i;

    /* 使能时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* 关闭 JTAG 保留 SWD（释放 PA15/PB3/PB4） */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();

    /* 配置 I2C1 引脚: PB6(SCL), PB7(SDA) */
    GPIO_InitStruct.Pin = CAM_SCL_PIN | CAM_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 配置 I2C1 */
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;     /* SCCB 100KHz */
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);

    /* 配置数据线 D0-D7 为浮空输入 */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    /* PA8 (D0), PA11 (D3), PA12 (D4), PA15 (D7) */
    GPIO_InitStruct.Pin = CAM_D0_PIN | CAM_D3_PIN | CAM_D4_PIN | CAM_D7_PIN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB4 (D1), PB5 (D2), PB14 (D5), PB15 (D6) */
    GPIO_InitStruct.Pin = CAM_D1_PIN | CAM_D2_PIN | CAM_D5_PIN | CAM_D6_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 配置控制信号: PA5(PCLK), PB8(VSYNC), PB9(HREF) 为浮空输入 */
    GPIO_InitStruct.Pin = CAM_PCLK_PIN;
    HAL_GPIO_Init(CAM_PCLK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CAM_VSYNC_PIN | CAM_HREF_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 等待 OV7670 上电稳定 */
    HAL_Delay(100);

    /* 检查设备 ID */
    if (Camera_ReadReg(REG_PID, &pid) != HAL_OK) return -1;
    if (Camera_ReadReg(REG_VER, &ver) != HAL_OK) return -1;

    if (pid != 0x76 || ver != 0x73) {
        /* 设备 ID 不匹配，可能连接有问题 */
        return -1;
    }

    /* 写入初始化寄存器 */
    for (i = 0; ov7670_init_regs[i][0] != 0xFF; i++) {
        Camera_WriteReg(ov7670_init_regs[i][0], ov7670_init_regs[i][1]);

        /* 软复位后需要较长延时 */
        if (ov7670_init_regs[i][0] == REG_COM7 && ov7670_init_regs[i][1] == 0x80) {
            HAL_Delay(100);
        } else {
            HAL_Delay(1);
        }
    }

    return 0;
}

/**
 * @brief  采集一行图像数据
 * @note   等待 VSYNC 同步，然后跳过前 line_num 行，采集目标行
 */
int8_t Camera_CaptureLine(uint16_t line_num, uint8_t *buf)
{
    uint32_t timeout;
    uint16_t skip_lines;
    uint16_t byte_count;

    if (line_num >= CAM_IMG_HEIGHT || buf == NULL) {
        return -1;
    }

    /* 等待 VSYNC 下降沿（新帧开始） */
    timeout = HAL_GetTick() + 500;
    /* 先等 VSYNC 变高 */
    while (HAL_GPIO_ReadPin(CAM_VSYNC_PORT, CAM_VSYNC_PIN) == GPIO_PIN_RESET) {
        if (HAL_GetTick() > timeout) return -1;
    }
    /* 再等 VSYNC 变低（帧开始） */
    while (HAL_GPIO_ReadPin(CAM_VSYNC_PORT, CAM_VSYNC_PIN) == GPIO_PIN_SET) {
        if (HAL_GetTick() > timeout) return -1;
    }

    /* 跳过前 line_num 行 */
    for (skip_lines = 0; skip_lines < line_num; skip_lines++) {
        /* 等待 HREF 变高（行开始） */
        timeout = HAL_GetTick() + 100;
        while (HAL_GPIO_ReadPin(CAM_HREF_PORT, CAM_HREF_PIN) == GPIO_PIN_RESET) {
            if (HAL_GetTick() > timeout) return -1;
        }
        /* 等待 HREF 变低（行结束） */
        while (HAL_GPIO_ReadPin(CAM_HREF_PORT, CAM_HREF_PIN) == GPIO_PIN_SET) {
            /* 空读，跳过这行的数据 */
        }
    }

    /* 采集目标行 */
    /* 等待 HREF 变高 */
    timeout = HAL_GetTick() + 100;
    while (HAL_GPIO_ReadPin(CAM_HREF_PORT, CAM_HREF_PIN) == GPIO_PIN_RESET) {
        if (HAL_GetTick() > timeout) return -1;
    }

    /* 在 HREF 高电平期间，逐 PCLK 采集数据 */
    byte_count = 0;
    while (HAL_GPIO_ReadPin(CAM_HREF_PORT, CAM_HREF_PIN) == GPIO_PIN_SET
           && byte_count < CAM_LINE_BUF_SIZE) {
        /* 等待 PCLK 上升沿 */
        while (HAL_GPIO_ReadPin(CAM_PCLK_PORT, CAM_PCLK_PIN) == GPIO_PIN_RESET);
        /* 读取数据 */
        buf[byte_count++] = Camera_ReadDataByte();
        /* 等待 PCLK 下降沿 */
        while (HAL_GPIO_ReadPin(CAM_PCLK_PORT, CAM_PCLK_PIN) == GPIO_PIN_SET);
    }

    return 0;
}

/**
 * @brief  采集缩略图（40x30, 降采样4倍）
 * @note   每隔4行采集一行，每行每隔4个像素取一个
 *          输出尺寸: 40x30 RGB565 = 2400 字节
 */
int8_t Camera_CaptureThumbnail(uint8_t *buf, uint16_t buf_size,
                                uint16_t *out_width, uint16_t *out_height)
{
    uint8_t line_buf[CAM_LINE_BUF_SIZE];
    uint16_t row, col;
    uint16_t out_idx = 0;
    uint16_t thumb_w = CAM_IMG_WIDTH / 4;   /* 40 */
    uint16_t thumb_h = CAM_IMG_HEIGHT / 4;  /* 30 */

    if (buf_size < thumb_w * thumb_h * CAM_BYTES_PER_PIXEL) {
        return -1;
    }

    *out_width = thumb_w;
    *out_height = thumb_h;

    for (row = 0; row < CAM_IMG_HEIGHT; row += 4) {
        /* 采集这一行 */
        if (Camera_CaptureLine(row, line_buf) != 0) {
            return -1;
        }

        /* 每隔4个像素取一个 */
        for (col = 0; col < CAM_IMG_WIDTH; col += 4) {
            uint16_t src_idx = col * CAM_BYTES_PER_PIXEL;
            buf[out_idx++] = line_buf[src_idx];
            buf[out_idx++] = line_buf[src_idx + 1];
        }
    }

    return 0;
}
