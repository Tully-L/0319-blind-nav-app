class LocationPoint {
  final double latitude;
  final double longitude;
  final DateTime timestamp;
  final double accuracy;

  LocationPoint({
    required this.latitude,
    required this.longitude,
    required this.timestamp,
    this.accuracy = 10.0,
  });

  factory LocationPoint.fromJson(Map<String, dynamic> json) {
    return LocationPoint(
      latitude: json['latitude'] as double,
      longitude: json['longitude'] as double,
      timestamp: DateTime.parse(json['timestamp'] as String),
      accuracy: (json['accuracy'] as num?)?.toDouble() ?? 10.0,
    );
  }

  Map<String, dynamic> toJson() => {
        'latitude': latitude,
        'longitude': longitude,
        'timestamp': timestamp.toIso8601String(),
        'accuracy': accuracy,
      };
}
