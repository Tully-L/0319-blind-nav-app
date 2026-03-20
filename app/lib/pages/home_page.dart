import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter_map/flutter_map.dart';
import 'package:latlong2/latlong.dart';
import '../services/mock_data_service.dart';
import '../models/location_point.dart';
import '../utils/app_theme.dart';

class HomePage extends StatefulWidget {
  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  final _mockService = MockDataService();
  final _mapController = MapController();
  LocationPoint? _currentPoint;
  StreamSubscription? _locationSub;
  bool _tracking = true;

  @override
  void initState() {
    super.initState();
    _currentPoint = _mockService.currentLocation;
    _locationSub = _mockService.locationStream.listen((point) {
      setState(() => _currentPoint = point);
      if (_tracking) {
        _mapController.move(
          LatLng(point.latitude, point.longitude),
          _mapController.camera.zoom,
        );
      }
    });
  }

  @override
  void dispose() {
    _locationSub?.cancel();
    _mapController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final pos = _currentPoint;
    final center = pos != null
        ? LatLng(pos.latitude, pos.longitude)
        : LatLng(39.9042, 116.4074);

    return Scaffold(
      appBar: AppBar(title: Text('实时位置')),
      body: Stack(
        children: [
          FlutterMap(
            mapController: _mapController,
            options: MapOptions(
              initialCenter: center,
              initialZoom: 16.0,
              onPositionChanged: (pos, hasGesture) {
                if (hasGesture) _tracking = false;
              },
            ),
            children: [
              TileLayer(
                urlTemplate: 'https://tile.openstreetmap.org/{z}/{x}/{y}.png',
                userAgentPackageName: 'com.linyu.blind_nav_app',
              ),
              if (pos != null)
                CircleLayer(
                  circles: [
                    CircleMarker(
                      point: LatLng(pos.latitude, pos.longitude),
                      radius: pos.accuracy,
                      useRadiusInMeter: true,
                      color: AppTheme.primary.withOpacity(0.15),
                      borderColor: AppTheme.primary.withOpacity(0.4),
                      borderStrokeWidth: 1.5,
                    ),
                  ],
                ),
              if (pos != null)
                MarkerLayer(
                  markers: [
                    Marker(
                      point: LatLng(pos.latitude, pos.longitude),
                      width: 40,
                      height: 40,
                      child: Icon(Icons.accessible_forward,
                          color: AppTheme.primary, size: 36),
                    ),
                  ],
                ),
            ],
          ),
          // 信息面板
          Positioned(
            left: 16,
            right: 16,
            bottom: 16,
            child: Card(
              child: Padding(
                padding: EdgeInsets.all(16),
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Row(
                      children: [
                        Icon(Icons.circle, color: AppTheme.success, size: 12),
                        SizedBox(width: 8),
                        Text('设备在线',
                            style: TextStyle(
                                fontSize: 16,
                                fontWeight: FontWeight.w600,
                                color: AppTheme.success)),
                        Spacer(),
                        Text(
                          '精度: ${pos?.accuracy.toStringAsFixed(1) ?? '-'}m',
                          style: TextStyle(fontSize: 14, color: AppTheme.textSecondary),
                        ),
                      ],
                    ),
                    SizedBox(height: 8),
                    Text(
                      '纬度: ${pos?.latitude.toStringAsFixed(6) ?? '-'}  '
                      '经度: ${pos?.longitude.toStringAsFixed(6) ?? '-'}',
                      style: TextStyle(fontSize: 14, color: AppTheme.textSecondary),
                    ),
                    SizedBox(height: 4),
                    Text(
                      '更新时间: ${_formatTime(pos?.timestamp)}',
                      style: TextStyle(fontSize: 14, color: AppTheme.textSecondary),
                    ),
                  ],
                ),
              ),
            ),
          ),
          // 回到当前位置按钮
          if (!_tracking)
            Positioned(
              right: 16,
              bottom: 140,
              child: FloatingActionButton.small(
                onPressed: () {
                  _tracking = true;
                  if (pos != null) {
                    _mapController.move(
                      LatLng(pos.latitude, pos.longitude),
                      16.0,
                    );
                  }
                },
                backgroundColor: Colors.white,
                child: Icon(Icons.my_location, color: AppTheme.primary),
              ),
            ),
        ],
      ),
    );
  }

  String _formatTime(DateTime? dt) {
    if (dt == null) return '-';
    return '${dt.hour.toString().padLeft(2, '0')}:'
        '${dt.minute.toString().padLeft(2, '0')}:'
        '${dt.second.toString().padLeft(2, '0')}';
  }
}
