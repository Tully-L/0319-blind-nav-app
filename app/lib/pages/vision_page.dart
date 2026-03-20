import 'dart:io';
import 'package:flutter/material.dart';
import 'package:image_picker/image_picker.dart';
import 'package:flutter_tts/flutter_tts.dart';
import '../services/vision_service.dart';
import '../utils/app_theme.dart';

class VisionPage extends StatefulWidget {
  @override
  State<VisionPage> createState() => _VisionPageState();
}

class _VisionPageState extends State<VisionPage> {
  final _picker = ImagePicker();
  final _tts = FlutterTts();
  File? _imageFile;
  String? _result;
  bool _loading = false;
  bool _speaking = false;

  @override
  void initState() {
    super.initState();
    _initTts();
  }

  void _initTts() {
    _tts.setLanguage('zh-CN');
    _tts.setSpeechRate(0.45);
    _tts.setVolume(1.0);
    _tts.setCompletionHandler(() {
      if (mounted) setState(() => _speaking = false);
    });
  }

  @override
  void dispose() {
    _tts.stop();
    super.dispose();
  }

  Future<void> _takePhoto() async {
    final xFile = await _picker.pickImage(
      source: ImageSource.camera,
      maxWidth: 1280,
      imageQuality: 85,
    );
    if (xFile == null) return;

    setState(() {
      _imageFile = File(xFile.path);
      _result = null;
    });
    _recognize();
  }

  Future<void> _pickFromGallery() async {
    final xFile = await _picker.pickImage(
      source: ImageSource.gallery,
      maxWidth: 1280,
      imageQuality: 85,
    );
    if (xFile == null) return;

    setState(() {
      _imageFile = File(xFile.path);
      _result = null;
    });
    _recognize();
  }

  Future<void> _recognize() async {
    if (_imageFile == null) return;
    setState(() => _loading = true);

    final result = await VisionService.recognizeImage(_imageFile!);

    setState(() {
      _result = result;
      _loading = false;
    });
  }

  Future<void> _speak() async {
    if (_result == null) return;
    if (_speaking) {
      await _tts.stop();
      setState(() => _speaking = false);
    } else {
      setState(() => _speaking = true);
      // 去掉 emoji 后播报
      final text = _result!.replaceAll(RegExp(r'[^\u4e00-\u9fff\w\s，。！？、：；""''（）\-]'), '');
      await _tts.speak(text);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('视觉识别')),
      body: SingleChildScrollView(
        padding: EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            // 说明卡片
            Card(
              color: AppTheme.primary.withOpacity(0.05),
              child: Padding(
                padding: EdgeInsets.all(14),
                child: Row(
                  children: [
                    Icon(Icons.info_outline, color: AppTheme.primary, size: 22),
                    SizedBox(width: 10),
                    Expanded(
                      child: Text(
                        '实际系统中，图像来源为盲杖端 OV7670 摄像头，通过 GPRS 上传至云端识别。'
                        '此处使用手机摄像头演示。',
                        style: TextStyle(fontSize: 14, color: AppTheme.textSecondary),
                      ),
                    ),
                  ],
                ),
              ),
            ),
            SizedBox(height: 16),
            // 拍照/选图按钮
            Row(
              children: [
                Expanded(
                  child: ElevatedButton.icon(
                    onPressed: _loading ? null : _takePhoto,
                    icon: Icon(Icons.camera_alt, size: 24),
                    label: Text('拍照识别'),
                    style: ElevatedButton.styleFrom(
                      padding: EdgeInsets.symmetric(vertical: 16),
                    ),
                  ),
                ),
                SizedBox(width: 12),
                Expanded(
                  child: ElevatedButton.icon(
                    onPressed: _loading ? null : _pickFromGallery,
                    icon: Icon(Icons.photo_library, size: 24),
                    label: Text('选择图片'),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: AppTheme.accent,
                      padding: EdgeInsets.symmetric(vertical: 16),
                    ),
                  ),
                ),
              ],
            ),
            SizedBox(height: 20),
            // 图片预览
            if (_imageFile != null)
              ClipRRect(
                borderRadius: BorderRadius.circular(12),
                child: Image.file(_imageFile!,
                    height: 220, width: double.infinity, fit: BoxFit.cover),
              ),
            if (_imageFile != null) SizedBox(height: 16),
            // 加载中
            if (_loading)
              Center(
                child: Column(
                  children: [
                    CircularProgressIndicator(),
                    SizedBox(height: 12),
                    Text('正在识别中...', style: TextStyle(fontSize: 16)),
                  ],
                ),
              ),
            // 识别结果
            if (_result != null && !_loading) ...[
              Card(
                child: Padding(
                  padding: EdgeInsets.all(16),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Row(
                        children: [
                          Icon(Icons.visibility, color: AppTheme.primary),
                          SizedBox(width: 8),
                          Text('识别结果',
                              style: TextStyle(
                                  fontSize: 18, fontWeight: FontWeight.w600)),
                          Spacer(),
                          IconButton(
                            onPressed: _speak,
                            icon: Icon(
                              _speaking ? Icons.stop : Icons.volume_up,
                              color: AppTheme.primary,
                            ),
                            tooltip: _speaking ? '停止播报' : '语音播报',
                          ),
                        ],
                      ),
                      Divider(),
                      SelectableText(
                        _result!,
                        style: TextStyle(fontSize: 16, height: 1.6),
                      ),
                    ],
                  ),
                ),
              ),
            ],
          ],
        ),
      ),
    );
  }
}
