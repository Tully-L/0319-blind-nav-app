import 'package:flutter/material.dart';
import 'package:flutter_map/flutter_map.dart';
import 'package:latlong2/latlong.dart';
import 'package:intl/intl.dart';
import '../services/mock_data_service.dart';
import '../models/location_point.dart';
import '../utils/app_theme.dart';

class TrackPage extends StatefulWidget {
  @override
  State<TrackPage> createState() => _TrackPageState();
}

class _TrackPageState extends State<TrackPage> {
  final _mockService = MockDataService();
  late DateTime _selectedDate;
  List<LocationPoint> _trackPoints = [];

  @override
  void initState() {
    super.initState();
    _selectedDate = DateTime.now();
    _loadTrack();
  }

  void _loadTrack() {
    setState(() {
      _trackPoints = _mockService.getTrackForDate(_selectedDate);
    });
  }

  @override
  Widget build(BuildContext context) {
    final polylinePoints =
        _trackPoints.map((p) => LatLng(p.latitude, p.longitude)).toList();
    final center = polylinePoints.isNotEmpty
        ? polylinePoints[polylinePoints.length ~/ 2]
        : LatLng(39.9042, 116.4074);

    return Scaffold(
      appBar: AppBar(
        title: Text('历史轨迹'),
        actions: [
          IconButton(
            icon: Icon(Icons.calendar_today),
            onPressed: _pickDate,
            tooltip: '选择日期',
          ),
        ],
      ),
      body: Column(
        children: [
          // 日期栏
          Container(
            padding: EdgeInsets.symmetric(horizontal: 16, vertical: 12),
            color: Colors.white,
            child: Row(
              children: [
                Icon(Icons.date_range, color: AppTheme.primary, size: 22),
                SizedBox(width: 8),
                Text(
                  DateFormat('yyyy年MM月dd日').format(_selectedDate),
                  style: TextStyle(fontSize: 18, fontWeight: FontWeight.w500),
                ),
                Spacer(),
                Text(
                  '${_trackPoints.length} 个轨迹点',
                  style: TextStyle(fontSize: 14, color: AppTheme.textSecondary),
                ),
              ],
            ),
          ),
          Divider(height: 1),
          // 地图
          Expanded(
            child: FlutterMap(
              options: MapOptions(
                initialCenter: center,
                initialZoom: 15.0,
              ),
              children: [
                TileLayer(
                  urlTemplate: 'https://tile.openstreetmap.org/{z}/{x}/{y}.png',
                  userAgentPackageName: 'com.linyu.blind_nav_app',
                ),
                if (polylinePoints.length > 1)
                  PolylineLayer(
                    polylines: [
                      Polyline(
                        points: polylinePoints,
                        color: AppTheme.primary,
                        strokeWidth: 4.0,
                      ),
                    ],
                  ),
                if (polylinePoints.isNotEmpty)
                  MarkerLayer(
                    markers: [
                      // 起点
                      Marker(
                        point: polylinePoints.first,
                        width: 40,
                        height: 40,
                        child: Icon(Icons.play_circle_filled,
                            color: AppTheme.success, size: 32),
                      ),
                      // 终点
                      Marker(
                        point: polylinePoints.last,
                        width: 40,
                        height: 40,
                        child: Icon(Icons.stop_circle,
                            color: AppTheme.danger, size: 32),
                      ),
                      // 时间标记点（每小时一个）
                      ..._hourlyMarkers(),
                    ],
                  ),
              ],
            ),
          ),
          // 轨迹信息
          Container(
            padding: EdgeInsets.all(16),
            color: Colors.white,
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                _infoItem(Icons.schedule, '开始',
                    _trackPoints.isNotEmpty ? _formatTime(_trackPoints.first.timestamp) : '-'),
                _infoItem(Icons.schedule, '结束',
                    _trackPoints.isNotEmpty ? _formatTime(_trackPoints.last.timestamp) : '-'),
                _infoItem(Icons.straighten, '总距离',
                    '${_calcDistance().toStringAsFixed(1)} km'),
              ],
            ),
          ),
        ],
      ),
    );
  }

  List<Marker> _hourlyMarkers() {
    final markers = <Marker>[];
    int lastHour = -1;
    for (final p in _trackPoints) {
      if (p.timestamp.hour != lastHour && p.timestamp.minute < 5) {
        lastHour = p.timestamp.hour;
        markers.add(Marker(
          point: LatLng(p.latitude, p.longitude),
          width: 60,
          height: 24,
          child: Container(
            padding: EdgeInsets.symmetric(horizontal: 6, vertical: 2),
            decoration: BoxDecoration(
              color: AppTheme.primary,
              borderRadius: BorderRadius.circular(12),
            ),
            child: Text(
              '${p.timestamp.hour}:00',
              style: TextStyle(color: Colors.white, fontSize: 11),
              textAlign: TextAlign.center,
            ),
          ),
        ));
      }
    }
    return markers;
  }

  double _calcDistance() {
    if (_trackPoints.length < 2) return 0;
    const distance = Distance();
    double total = 0;
    for (var i = 1; i < _trackPoints.length; i++) {
      total += distance.as(
        LengthUnit.Meter,
        LatLng(_trackPoints[i - 1].latitude, _trackPoints[i - 1].longitude),
        LatLng(_trackPoints[i].latitude, _trackPoints[i].longitude),
      );
    }
    return total / 1000;
  }

  Widget _infoItem(IconData icon, String label, String value) {
    return Column(
      children: [
        Icon(icon, color: AppTheme.primary, size: 22),
        SizedBox(height: 4),
        Text(label, style: TextStyle(fontSize: 13, color: AppTheme.textSecondary)),
        SizedBox(height: 2),
        Text(value, style: TextStyle(fontSize: 16, fontWeight: FontWeight.w600)),
      ],
    );
  }

  Future<void> _pickDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: _selectedDate,
      firstDate: DateTime.now().subtract(Duration(days: 30)),
      lastDate: DateTime.now(),
      locale: Locale('zh', 'CN'),
    );
    if (picked != null) {
      _selectedDate = picked;
      _loadTrack();
    }
  }

  String _formatTime(DateTime dt) =>
      '${dt.hour.toString().padLeft(2, '0')}:${dt.minute.toString().padLeft(2, '0')}';
}
