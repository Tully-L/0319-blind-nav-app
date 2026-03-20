import 'package:flutter/material.dart';
import '../utils/app_theme.dart';

class AboutPage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('关于')),
      body: SingleChildScrollView(
        padding: EdgeInsets.all(24),
        child: Column(
          children: [
            SizedBox(height: 20),
            // Logo
            Container(
              width: 100,
              height: 100,
              decoration: BoxDecoration(
                color: AppTheme.primary,
                borderRadius: BorderRadius.circular(24),
                boxShadow: [
                  BoxShadow(
                    color: AppTheme.primary.withOpacity(0.3),
                    blurRadius: 20,
                    offset: Offset(0, 8),
                  ),
                ],
              ),
              child: Icon(Icons.accessible_forward,
                  size: 56, color: Colors.white),
            ),
            SizedBox(height: 24),
            Text('智能盲人辅助导航系统',
                style: TextStyle(
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                    color: AppTheme.textPrimary)),
            SizedBox(height: 8),
            Text('v1.0.0',
                style: TextStyle(fontSize: 16, color: AppTheme.textSecondary)),
            SizedBox(height: 32),
            // 信息卡片
            Card(
              child: Padding(
                padding: EdgeInsets.all(20),
                child: Column(
                  children: [
                    _infoRow(Icons.person, '开发者', '林雨'),
                    Divider(height: 24),
                    _infoRow(Icons.calendar_today, '开发日期', '2026年'),
                    Divider(height: 24),
                    _infoRow(Icons.school, '学校', '/* 学校名称 */'),
                    Divider(height: 24),
                    _infoRow(Icons.book, '专业', '/* 专业名称 */'),
                    Divider(height: 24),
                    _infoRow(Icons.assignment, '项目',
                        '基于STM32的智能盲人辅助导航系统\n—— 导航与远程管理'),
                  ],
                ),
              ),
            ),
            SizedBox(height: 16),
            // 系统架构
            Card(
              child: Padding(
                padding: EdgeInsets.all(20),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text('系统架构',
                        style: TextStyle(
                            fontSize: 18,
                            fontWeight: FontWeight.w600,
                            color: AppTheme.textPrimary)),
                    SizedBox(height: 12),
                    _archItem(Icons.bluetooth, '盲杖端',
                        'STM32F103C8T6 + 超声波/红外/GPS/摄像头'),
                    _archItem(Icons.cloud, '云端服务',
                        '数据接收 + 大模型视觉识别'),
                    _archItem(Icons.phone_android, 'App 端',
                        '实时地图 / 轨迹回放 / SOS 告警 / 视觉识别'),
                  ],
                ),
              ),
            ),
            SizedBox(height: 16),
            // 技术栈
            Card(
              child: Padding(
                padding: EdgeInsets.all(20),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text('技术栈',
                        style: TextStyle(
                            fontSize: 18,
                            fontWeight: FontWeight.w600,
                            color: AppTheme.textPrimary)),
                    SizedBox(height: 12),
                    Wrap(
                      spacing: 8,
                      runSpacing: 8,
                      children: [
                        _techChip('Flutter'),
                        _techChip('Dart'),
                        _techChip('OpenStreetMap'),
                        _techChip('通义千问 VL'),
                        _techChip('Flutter TTS'),
                        _techChip('STM32'),
                        _techChip('GPRS'),
                      ],
                    ),
                  ],
                ),
              ),
            ),
            SizedBox(height: 32),
            Text('毕业设计作品',
                style: TextStyle(fontSize: 14, color: AppTheme.textSecondary)),
            SizedBox(height: 8),
          ],
        ),
      ),
    );
  }

  Widget _infoRow(IconData icon, String label, String value) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Icon(icon, color: AppTheme.primary, size: 22),
        SizedBox(width: 12),
        SizedBox(
          width: 70,
          child: Text(label,
              style: TextStyle(fontSize: 15, color: AppTheme.textSecondary)),
        ),
        Expanded(
          child: Text(value,
              style: TextStyle(fontSize: 16, fontWeight: FontWeight.w500)),
        ),
      ],
    );
  }

  Widget _archItem(IconData icon, String title, String desc) {
    return Padding(
      padding: EdgeInsets.only(bottom: 10),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Icon(icon, color: AppTheme.accent, size: 20),
          SizedBox(width: 10),
          Expanded(
            child: RichText(
              text: TextSpan(
                style: TextStyle(fontSize: 15, color: AppTheme.textPrimary),
                children: [
                  TextSpan(
                      text: '$title: ',
                      style: TextStyle(fontWeight: FontWeight.w600)),
                  TextSpan(text: desc),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _techChip(String label) {
    return Chip(
      label: Text(label, style: TextStyle(fontSize: 13)),
      backgroundColor: AppTheme.primary.withOpacity(0.08),
      padding: EdgeInsets.symmetric(horizontal: 4),
    );
  }
}
