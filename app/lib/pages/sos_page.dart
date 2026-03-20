import 'package:flutter/material.dart';
import 'package:flutter_map/flutter_map.dart';
import 'package:latlong2/latlong.dart';
import 'package:intl/intl.dart';
import '../services/mock_data_service.dart';
import '../models/sos_alert.dart';
import '../utils/app_theme.dart';

class SosPage extends StatefulWidget {
  @override
  State<SosPage> createState() => _SosPageState();
}

class _SosPageState extends State<SosPage> {
  final _mockService = MockDataService();
  late List<SosAlert> _alerts;

  @override
  void initState() {
    super.initState();
    _alerts = _mockService.mockAlerts;
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('SOS 告警')),
      body: Column(
        children: [
          // 一键 SOS 按钮
          Container(
            width: double.infinity,
            padding: EdgeInsets.all(20),
            color: Colors.white,
            child: ElevatedButton.icon(
              onPressed: _triggerSos,
              icon: Icon(Icons.warning_amber_rounded, size: 28),
              label: Text('模拟 SOS 告警', style: TextStyle(fontSize: 20)),
              style: ElevatedButton.styleFrom(
                backgroundColor: AppTheme.danger,
                foregroundColor: Colors.white,
                padding: EdgeInsets.symmetric(vertical: 18),
                shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(12)),
              ),
            ),
          ),
          Divider(height: 1),
          // 告警列表
          Expanded(
            child: _alerts.isEmpty
                ? Center(
                    child: Column(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        Icon(Icons.check_circle_outline,
                            size: 64, color: AppTheme.success),
                        SizedBox(height: 12),
                        Text('暂无告警', style: TextStyle(fontSize: 18, color: AppTheme.textSecondary)),
                      ],
                    ),
                  )
                : ListView.builder(
                    itemCount: _alerts.length,
                    padding: EdgeInsets.only(top: 8),
                    itemBuilder: (ctx, i) => _alertCard(_alerts[i]),
                  ),
          ),
        ],
      ),
    );
  }

  Widget _alertCard(SosAlert alert) {
    final statusColor = switch (alert.status) {
      AlertStatus.pending => AppTheme.danger,
      AlertStatus.processing => AppTheme.accent,
      AlertStatus.resolved => AppTheme.success,
    };

    return Card(
      child: InkWell(
        borderRadius: BorderRadius.circular(12),
        onTap: () => _showAlertOnMap(alert),
        child: Padding(
          padding: EdgeInsets.all(16),
          child: Row(
            children: [
              Container(
                width: 48,
                height: 48,
                decoration: BoxDecoration(
                  color: statusColor.withOpacity(0.1),
                  borderRadius: BorderRadius.circular(12),
                ),
                child: Icon(Icons.sos, color: statusColor, size: 28),
              ),
              SizedBox(width: 14),
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Row(
                      children: [
                        Expanded(
                          child: Text(alert.note ?? 'SOS 告警',
                              style: TextStyle(
                                  fontSize: 16, fontWeight: FontWeight.w500)),
                        ),
                        Container(
                          padding: EdgeInsets.symmetric(horizontal: 10, vertical: 3),
                          decoration: BoxDecoration(
                            color: statusColor.withOpacity(0.1),
                            borderRadius: BorderRadius.circular(20),
                          ),
                          child: Text(alert.statusText,
                              style: TextStyle(
                                  fontSize: 13,
                                  color: statusColor,
                                  fontWeight: FontWeight.w500)),
                        ),
                      ],
                    ),
                    SizedBox(height: 6),
                    Text(
                      DateFormat('MM-dd HH:mm').format(alert.timestamp),
                      style: TextStyle(fontSize: 14, color: AppTheme.textSecondary),
                    ),
                  ],
                ),
              ),
              SizedBox(width: 8),
              Icon(Icons.chevron_right, color: AppTheme.textSecondary),
            ],
          ),
        ),
      ),
    );
  }

  void _triggerSos() {
    setState(() {
      _alerts.insert(
        0,
        SosAlert(
          id: 'SOS-${DateTime.now().millisecondsSinceEpoch}',
          latitude: 39.9045 + (DateTime.now().second * 0.0001),
          longitude: 116.4078 + (DateTime.now().second * 0.0001),
          timestamp: DateTime.now(),
          status: AlertStatus.pending,
          note: '用户手动触发 SOS（模拟）',
        ),
      );
    });
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text('SOS 告警已触发！', style: TextStyle(fontSize: 16)),
        backgroundColor: AppTheme.danger,
        duration: Duration(seconds: 2),
      ),
    );
  }

  void _showAlertOnMap(SosAlert alert) {
    showModalBottomSheet(
      context: context,
      isScrollControlled: true,
      shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.vertical(top: Radius.circular(16))),
      builder: (ctx) => SizedBox(
        height: MediaQuery.of(context).size.height * 0.6,
        child: Column(
          children: [
            Padding(
              padding: EdgeInsets.all(16),
              child: Row(
                children: [
                  Icon(Icons.sos, color: AppTheme.danger, size: 24),
                  SizedBox(width: 8),
                  Expanded(
                    child: Text(alert.note ?? 'SOS 告警',
                        style: TextStyle(fontSize: 18, fontWeight: FontWeight.w600)),
                  ),
                  IconButton(
                    icon: Icon(Icons.close),
                    onPressed: () => Navigator.pop(ctx),
                  ),
                ],
              ),
            ),
            Divider(height: 1),
            Expanded(
              child: FlutterMap(
                options: MapOptions(
                  initialCenter: LatLng(alert.latitude, alert.longitude),
                  initialZoom: 17.0,
                ),
                children: [
                  TileLayer(
                    urlTemplate:
                        'https://tile.openstreetmap.org/{z}/{x}/{y}.png',
                    userAgentPackageName: 'com.linyu.blind_nav_app',
                  ),
                  MarkerLayer(
                    markers: [
                      Marker(
                        point: LatLng(alert.latitude, alert.longitude),
                        width: 48,
                        height: 48,
                        child: Icon(Icons.sos, color: AppTheme.danger, size: 40),
                      ),
                    ],
                  ),
                ],
              ),
            ),
            Container(
              padding: EdgeInsets.all(16),
              child: Text(
                '时间: ${DateFormat('yyyy-MM-dd HH:mm:ss').format(alert.timestamp)}\n'
                '位置: ${alert.latitude.toStringAsFixed(6)}, ${alert.longitude.toStringAsFixed(6)}\n'
                '状态: ${alert.statusText}',
                style: TextStyle(fontSize: 15, height: 1.6),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
