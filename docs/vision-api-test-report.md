# 视觉识别 API 测试报告

## 测试概述

- **测试日期**: 2026-03-20
- **API 服务**: 阿里云 DashScope（百炼平台）
- **模型**: qwen-vl-plus（通义千问视觉语言模型）
- **API 端点**: `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions`
- **认证方式**: Bearer Token
- **图片传输方式**: Base64 编码

## 系统 Prompt

```
你是一个盲人辅助系统的视觉识别模块。请分析这张图片，识别以下内容：
1. 是否有红绿灯？如果有，当前是什么颜色？
2. 前方是否有障碍物？如果有，是什么障碍物，大约多远？
3. 是否有盲道？盲道方向如何？
4. 周围环境描述（路况、人流等）。
请用简洁的中文回答，适合语音播报。
```

## API 调用方式

### 请求格式（OpenAI 兼容）

```bash
curl -s "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <API_KEY>" \
  -d '{
    "model": "qwen-vl-plus",
    "messages": [{
      "role": "user",
      "content": [
        {
          "type": "image_url",
          "image_url": {
            "url": "data:image/jpeg;base64,<BASE64_IMAGE>"
          }
        },
        {
          "type": "text",
          "text": "<PROMPT>"
        }
      ]
    }],
    "max_tokens": 1024
  }'
```

### 响应格式

```json
{
  "choices": [{
    "message": {
      "content": "识别结果文本"
    }
  }]
}
```

### Flutter 代码调用路径

```
vision_page.dart → _recognize() → VisionService.recognizeImage(File)
  1. 读取图片文件为字节
  2. Base64 编码
  3. 构造 OpenAI 兼容格式请求
  4. POST 到 DashScope API
  5. 解析 choices[0].message.content
  6. 返回识别结果文本，支持 TTS 语音播报
```

---

## 测试用例

### 测试 1：沙滩场景（无障碍物、无红绿灯）

**测试图片**: [test_beach.jpg](test-images/test_beach.jpg)

![沙滩场景](test-images/test_beach.jpg)

**场景描述**: 夜晚城市河景，有桥梁和高楼大厦。

**识别结果**:
> 1. 图片中没有红绿灯。
> 2. 前方没有明显的障碍物。
> 3. 图片中没有盲道。
> 4. 周围环境为夜晚的城市景观，有桥梁和高楼大厦，水面有船只行驶，没有明显的人流。

**评估**: 正确识别出无红绿灯、无障碍物、无盲道，环境描述准确。

---

### 测试 2：阿里云示例图（人物 + 动物）

**测试来源**: DashScope 官方示例图（dog_and_girl.jpeg）

**场景描述**: 海边沙滩上，一位女性和一只拉布拉多犬互动。

**识别结果**:
> 这是一张在海滩上的照片。画面中有一位女性和一只狗。女性坐在沙滩上，穿着格子衬衫和黑色裤子，面带微笑，正在与狗互动。她的右手伸向狗的前爪，似乎在进行握手动作。狗是一只黄色的拉布拉多犬，戴着彩色的牵引绳，坐在女性的对面，前爪搭在女性的手上。背景是海浪拍打沙滩的景象，天空明亮，阳光照射在沙滩上，形成温暖的光影效果。沙滩上有许多脚印，显示出有人在此活动过。整体氛围温馨和谐。

**评估**: 环境描述非常详细，人物、动物、动作、背景均识别准确。

---

### 测试 3：夜间城市路口（有红绿灯、有盲道）

**测试图片**: [test_street_night.png](test-images/test_street_night.png)

![夜间城市路口](test-images/test_street_night.png)

**场景描述**: 夜间城市十字路口，有红绿灯显示红灯，有发光斑马线，地面有盲道，远处可见车辆和高楼。

**识别结果**:
> 1. 有红绿灯，当前显示为红色。
> 2. 前方没有明显的障碍物。
> 3. 有盲道，盲道方向与人行横道平行。
> 4. 周围环境为夜间城市街道，路况良好，远处有车辆和行人，但距离较远。

**评估**: 红绿灯颜色（红色）识别正确，盲道识别正确且给出方向，环境描述准确。该场景下系统应提示用户"红灯，请等待"。

---

## 测试结论

| 项目 | 结果 |
|------|------|
| API 连通性 | 正常 |
| Base64 图片传输 | 正常 |
| 红绿灯识别 | 正确（红色） |
| 障碍物检测 | 正确（无障碍物场景） |
| 盲道识别 | 正确（识别存在 + 方向） |
| 环境描述 | 详细准确 |
| 响应速度 | 2-5 秒 |
| 免费额度 | 新用户赠送，足够开发测试 |

### 备选 API 方案

| 方案 | 端点 | 模型 | 状态 |
|------|------|------|------|
| 阿里云 DashScope | dashscope.aliyuncs.com | qwen-vl-plus | **当前使用** |
| 硅基流动 SiliconFlow | api.siliconflow.cn | Qwen2.5-VL-72B | 需充值（最低 1 元） |
| Google Gemini | generativelanguage.googleapis.com | gemini-2.0-flash | 需代理，免费额度激活中 |

三个方案均采用 OpenAI 兼容格式，切换只需修改 `_apiUrl`、`_apiKey` 和 `model` 三个参数。
