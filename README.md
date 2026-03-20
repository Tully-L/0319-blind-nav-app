# 视障辅助导航系统

毕业设计项目：基于 STM32 + Flutter 的视障人群辅助导航系统。

## 目录结构

```
blind-nav-project/
├── app/                  # Flutter Android 客户端
│   └── blind_nav_app/
├── embedded/             # STM32 嵌入式工程
│   └── blind_nav_stm32/
├── docs/                 # 项目文档
└── README.md
```

## 模块说明

### App 端 — Flutter Android 客户端

- **路径**: `app/blind_nav_app/`
- **代码量**: 5353 行
- **6 个页面**:
  - 首页（地图）— 实时位置显示，模拟位置更新
  - 轨迹 — 历史轨迹回放，按日期筛选
  - SOS — 告警列表 + 一键 SOS
  - 视觉识别 — 拍照 → 大模型 API 识别（预留接口）
  - 设置 — 紧急联系人、预警灵敏度、设备绑定、API Key 配置
  - 关于 — 版本号、开发者、日期
- **地图方案**: flutter_map + OpenStreetMap（免 Key），模拟数据已内置
- **运行方式**:
  ```bash
  cd app/blind_nav_app
  flutter pub get
  flutter run
  ```

### 嵌入式端 — STM32 工程

- **路径**: `embedded/blind_nav_stm32/`
- **芯片**: STM32F1xx
- **主要模块**:
  - GPS 定位 (`gps.c/h`)
  - 超声波避障 (`ultrasonic.c/h`)
  - 红外检测 (`infrared.c/h`)
  - 摄像头 (`camera.c/h`)
  - 语音播报 (`voice.c/h`)
  - 蜂鸣器报警 (`buzzer.c/h`)
  - 振动电机 (`motor.c/h`)
  - SIM900A 通信 (`sim900a.c/h`)
  - 云端通信 (`cloud.c/h`)
  - 导航逻辑 (`navigation.c/h`)
  - 电池管理 (`battery.c/h`)
  - 预警系统 (`warning.c/h`)
- **开发环境**: Keil MDK-ARM（见 `MDK-ARM/README_KEIL_CONFIG.md`）
