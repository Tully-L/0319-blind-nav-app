# Keil MDK 工程配置说明

## 一、创建 Keil 工程

### 1. 新建项目
1. 打开 Keil uVision5
2. `Project` → `New uVision Project`
3. 保存到 `MDK-ARM/blind_nav.uvprojx`
4. 选择芯片：`STMicroelectronics` → `STM32F103C8`
5. 弹出的 Manage Run-Time Environment 窗口中，勾选：
   - CMSIS → CORE
   - Device → Startup

### 2. 添加 STM32 HAL 库文件
需要从 ST 官网下载 STM32CubeF1 固件包（或使用 STM32CubeMX 生成）。

将以下 HAL 库源文件复制到 `Drivers/` 目录：
```
Drivers/
├── CMSIS/
│   ├── Include/            ← CMSIS 核心头文件 (core_cm3.h 等)
│   └── Device/ST/STM32F1xx/
│       ├── Include/        ← stm32f1xx.h, stm32f103xb.h
│       └── Source/
│           └── Templates/
│               └── arm/    ← startup_stm32f103xb.s (启动文件)
└── STM32F1xx_HAL_Driver/
    ├── Inc/                ← HAL 库头文件
    └── Src/                ← HAL 库源文件
```

需要的 HAL 库源文件（Src/）：
- `stm32f1xx_hal.c`
- `stm32f1xx_hal_cortex.c`
- `stm32f1xx_hal_rcc.c`
- `stm32f1xx_hal_rcc_ex.c`
- `stm32f1xx_hal_gpio.c`
- `stm32f1xx_hal_tim.c`
- `stm32f1xx_hal_tim_ex.c`
- `stm32f1xx_hal_uart.c`
- `stm32f1xx_hal_adc.c`
- `stm32f1xx_hal_adc_ex.c`
- `stm32f1xx_hal_i2c.c`
- `stm32f1xx_hal_dma.c`
- `stm32f1xx_hal_flash.c`
- `stm32f1xx_hal_pwr.c`

### 3. 配置 Source Groups
在 Keil 左侧 Project 面板中创建以下 Groups 并添加文件：

**Application/User/Core:**
- `Core/Src/main.c`
- `Core/Src/stm32f1xx_it.c`
- `Core/Src/system_stm32f1xx.c`
- `Core/Src/ultrasonic.c`
- `Core/Src/gps.c`
- `Core/Src/infrared.c`
- `Core/Src/camera.c`
- `Core/Src/sim900a.c`
- `Core/Src/voice.c`
- `Core/Src/motor.c`
- `Core/Src/buzzer.c`
- `Core/Src/battery.c`
- `Core/Src/warning.c`
- `Core/Src/navigation.c`
- `Core/Src/cloud.c`

**Drivers/STM32F1xx_HAL_Driver:**
- 上面列出的所有 HAL 库 .c 文件

**Drivers/CMSIS:**
- `startup_stm32f103xb.s` (启动文件)

## 二、项目设置

### 1. Target 设置
`Project` → `Options for Target` → `Target` 选项卡：
- Xtal (MHz): `8.0`
- IROM1: Start=`0x08000000`, Size=`0x10000` (64KB Flash)
- IRAM1: Start=`0x20000000`, Size=`0x5000` (20KB RAM)

### 2. C/C++ 设置
`C/C++` 选项卡：
- **Define**: `USE_HAL_DRIVER,STM32F103xB`
- **Include Paths** (点击 `...` 按钮添加):
  ```
  ../Core/Inc
  ../Drivers/STM32F1xx_HAL_Driver/Inc
  ../Drivers/CMSIS/Include
  ../Drivers/CMSIS/Device/ST/STM32F1xx/Include
  ```
- **Optimization**: Level 1 (-O1) 推荐
- **C99 Mode**: 勾选

### 3. Linker 设置
`Linker` 选项卡：
- 使用默认分散加载文件即可

### 4. Debug 设置
`Debug` 选项卡：
- 选择 `ST-Link Debugger`（或 `J-Link`）
- Settings → Flash Download → 勾选 `Reset and Run`

### 5. Utilities 设置
`Utilities` 选项卡：
- 选择 `Use Target Driver for Flash Programming`
- 选择对应的调试器

## 三、编译与烧录

### 编译
1. 点击工具栏 `Build` 按钮（F7）
2. 首次编译可能有警告，确保无错误即可
3. 注意 RAM 使用量不超过 20KB

### 烧录
1. 连接 ST-Link/J-Link 调试器到 STM32 的 SWD 接口
   - SWDIO → PA13
   - SWCLK → PA14
   - GND → GND
   - 3.3V → 3.3V
2. 点击 `Download` 按钮（F8）

## 四、使用 STM32CubeMX 替代方案

如果更习惯用 CubeMX 图形化配置，可以：

1. 新建 CubeMX 项目，选择 STM32F103C8Tx
2. 按 `Core/Inc/pin_config.h` 中的引脚分配配置各外设
3. 生成代码（选择 MDK-ARM 工具链）
4. 将本项目的 `Core/Src/*.c` 和 `Core/Inc/*.h` 文件复制到生成的项目中
5. 替换 CubeMX 生成的 `main.c`

## 五、重要提醒

1. **JTAG 重映射**：本项目使用了 PA15/PB3/PB4 作为普通 GPIO，
   必须关闭 JTAG（代码中已处理 `__HAL_AFIO_REMAP_SWJ_NOJTAG()`）。
   调试只能使用 SWD 方式，不能用 JTAG。

2. **RAM 限制**：STM32F103C8T6 只有 20KB RAM，
   OV7670 无法存储完整帧图像。代码中采用逐行采集 + 缩略图方式处理。

3. **SIM900A 上电**：SIM900A 模块上电需要 2-3 秒，
   代码中有相应延时。请确保模块供电充足（峰值电流可达 2A）。

4. **GPS 冷启动**：ATGM336H 首次定位（冷启动）可能需要 30-60 秒，
   在此期间 GPS 数据无效，不影响避障等本地功能。

5. **服务器配置**：`main.h` 中的 `CLOUD_SERVER_IP` 和 `CLOUD_SERVER_PORT`
   需要替换为实际的云端服务器地址。`SOS_PHONE_NUMBER` 需替换为真实号码。
