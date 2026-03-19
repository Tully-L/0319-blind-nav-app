# 智能盲人辅助导航系统 - Android App

基于STM32的智能盲人辅助导航系统 —— 导航与远程管理 App 端

## 功能

- **实时位置**：地图展示盲杖当前位置，精度圆显示，实时更新
- **历史轨迹**：按日期查看行走轨迹，起终点标记，距离统计
- **SOS 告警**：告警列表、状态管理、地图定位、模拟一键 SOS
- **视觉识别**：拍照 → 大模型识别（红绿灯/障碍物/盲道）→ 语音播报
- **远程设置**：紧急联系人、预警灵敏度、设备绑定、同步频率
- **关于页面**：项目信息、系统架构、技术栈

## 快速开始

```bash
# 确保已安装 Flutter SDK (>=3.0.0)
flutter pub get
flutter run
```

## 项目结构

```
lib/
├── main.dart              # 入口 + 底部导航
├── models/                # 数据模型
│   ├── location_point.dart
│   ├── sos_alert.dart
│   └── contact.dart
├── pages/                 # 页面
│   ├── home_page.dart     # 实时位置地图
│   ├── track_page.dart    # 历史轨迹
│   ├── sos_page.dart      # SOS 告警
│   ├── vision_page.dart   # 视觉识别
│   ├── settings_page.dart # 设置
│   └── about_page.dart    # 关于
├── services/              # 服务层
│   ├── mock_data_service.dart   # 模拟数据
│   ├── vision_service.dart      # 视觉识别 API
│   └── settings_service.dart    # 本地设置存储
├── utils/
│   └── app_theme.dart     # 主题配置
└── widgets/               # 通用组件（预留）
```

## 地图说明

使用 flutter_map + OpenStreetMap 瓦片，无需 API Key，开箱即用。
如需切换为高德地图，替换 `TileLayer` 的 `urlTemplate` 为高德瓦片地址并配置 API Key。

## 视觉识别

- 默认使用演示模式（mock 返回）
- 在「设置」中配置通义千问 VL API Key 后，自动切换为真实 API 调用
- API 接口：阿里云 DashScope qwen-vl-max

## 注意事项

- 当前版本使用模拟数据，无需真实后端服务器
- 实际部署时，位置数据应来自 STM32 通过 GPRS 上报
- 图像识别数据实际来源为盲杖端 OV7670 摄像头
