import 'dart:async';
import 'dart:math';
import '../models/location_point.dart';
import '../models/sos_alert.dart';

class MockDataService {
  static final MockDataService _instance = MockDataService._();
  factory MockDataService() => _instance;
  MockDataService._();

  final _random = Random(42);

  // 模拟盲杖当前位置（北京天安门附近）
  static const _baseLat = 39.9042;
  static const _baseLng = 116.4074;

  LocationPoint _currentLocation = LocationPoint(
    latitude: _baseLat,
    longitude: _baseLng,
    timestamp: DateTime.now(),
    accuracy: 8.0,
  );

  LocationPoint get currentLocation => _currentLocation;

  /// 模拟位置更新流（每2秒更新一次）
  Stream<LocationPoint> get locationStream {
    return Stream.periodic(Duration(seconds: 2), (_) {
      _currentLocation = LocationPoint(
        latitude: _currentLocation.latitude + (_random.nextDouble() - 0.5) * 0.0005,
        longitude: _currentLocation.longitude + (_random.nextDouble() - 0.5) * 0.0005,
        timestamp: DateTime.now(),
        accuracy: 5.0 + _random.nextDouble() * 10,
      );
      return _currentLocation;
    });
  }

  /// 生成某天的历史轨迹
  List<LocationPoint> getTrackForDate(DateTime date) {
    final points = <LocationPoint>[];
    var lat = _baseLat;
    var lng = _baseLng;
    final rng = Random(date.day * 100 + date.month);

    // 模拟一天中 8:00 - 18:00 的轨迹，每5分钟一个点
    for (var hour = 8; hour < 18; hour++) {
      for (var min = 0; min < 60; min += 5) {
        lat += (rng.nextDouble() - 0.48) * 0.0003;
        lng += (rng.nextDouble() - 0.48) * 0.0003;
        points.add(LocationPoint(
          latitude: lat,
          longitude: lng,
          timestamp: DateTime(date.year, date.month, date.day, hour, min),
          accuracy: 5.0 + rng.nextDouble() * 8,
        ));
      }
    }
    return points;
  }

  /// 模拟 SOS 告警列表
  List<SosAlert> get mockAlerts => [
        SosAlert(
          id: 'SOS-001',
          latitude: 39.9055,
          longitude: 116.4082,
          timestamp: DateTime.now().subtract(Duration(minutes: 15)),
          status: AlertStatus.pending,
          note: '检测到摔倒，自动触发 SOS',
        ),
        SosAlert(
          id: 'SOS-002',
          latitude: 39.9038,
          longitude: 116.4065,
          timestamp: DateTime.now().subtract(Duration(hours: 2)),
          status: AlertStatus.processing,
          note: '用户手动触发 SOS',
        ),
        SosAlert(
          id: 'SOS-003',
          latitude: 39.9061,
          longitude: 116.4091,
          timestamp: DateTime.now().subtract(Duration(hours: 5)),
          status: AlertStatus.resolved,
          note: '偏离预设路线告警',
        ),
        SosAlert(
          id: 'SOS-004',
          latitude: 39.9029,
          longitude: 116.4050,
          timestamp: DateTime.now().subtract(Duration(days: 1)),
          status: AlertStatus.resolved,
          note: '长时间静止告警',
        ),
      ];

  /// 可筛选的日期列表（最近7天）
  List<DateTime> get availableDates {
    final now = DateTime.now();
    return List.generate(7, (i) => DateTime(now.year, now.month, now.day - i));
  }
}
