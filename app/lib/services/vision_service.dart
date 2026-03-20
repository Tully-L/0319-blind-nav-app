import 'dart:convert';
import 'dart:io';
import 'package:http/http.dart' as http;

class VisionService {
  // 阿里云 DashScope API（OpenAI 兼容格式）
  static const String _apiUrl =
      'https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions';
  static String? _apiKey = 'sk-2b62e8fe7b4341339298a8b3867b9e6a';

  static void setApiKey(String key) => _apiKey = key;

  /// 调用视觉大模型识别图片
  /// 返回识别结果文本
  static Future<String> recognizeImage(File imageFile) async {
    // 如果没有配置 API Key，返回 mock 结果
    if (_apiKey == null || _apiKey!.isEmpty) {
      return _mockRecognition();
    }

    try {
      final bytes = await imageFile.readAsBytes();
      final base64Image = base64Encode(bytes);

      final response = await http.post(
        Uri.parse(_apiUrl),
        headers: {
          'Content-Type': 'application/json',
          'Authorization': 'Bearer $_apiKey',
        },
        body: jsonEncode({
          'model': 'qwen-vl-plus',
          'messages': [
            {
              'role': 'user',
              'content': [
                {
                  'type': 'image_url',
                  'image_url': {
                    'url': 'data:image/jpeg;base64,$base64Image',
                  },
                },
                {
                  'type': 'text',
                  'text': '你是一个盲人辅助系统的视觉识别模块。请分析这张图片，识别以下内容：'
                      '1. 是否有红绿灯？如果有，当前是什么颜色？'
                      '2. 前方是否有障碍物？如果有，是什么障碍物，大约多远？'
                      '3. 是否有盲道？盲道方向如何？'
                      '4. 周围环境描述（路况、人流等）。'
                      '请用简洁的中文回答，适合语音播报。',
                },
              ],
            },
          ],
          'max_tokens': 1024,
        }),
      );

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        return data['choices']?[0]?['message']?['content'] ??
            '识别完成，但未获取到结果文本。';
      } else {
        return '识别请求失败（${response.statusCode}），请检查网络连接和 API 配置。\n${response.body}';
      }
    } catch (e) {
      return '识别过程出错：$e';
    }
  }

  /// Mock 识别结果（演示用）
  static String _mockRecognition() {
    return '【视觉识别结果 - 演示模式】\n\n'
        '🚦 红绿灯：检测到前方路口有红绿灯，当前为绿灯，可以通行。\n\n'
        '⚠️ 障碍物：前方约3米处有一个垃圾桶，位于右侧，请靠左行走。\n\n'
        '🔲 盲道：脚下有盲道，方向为正前方，沿盲道直行约50米到达路口。\n\n'
        '🏙️ 环境：当前位于人行道上，左侧为商铺，右侧为马路。'
        '行人较少，路面平整，适合行走。\n\n'
        '💡 建议：沿盲道直行，注意右侧垃圾桶，前方路口绿灯可通行。';
  }
}
