enum AlertStatus { pending, processing, resolved }

class SosAlert {
  final String id;
  final double latitude;
  final double longitude;
  final DateTime timestamp;
  final AlertStatus status;
  final String? note;

  SosAlert({
    required this.id,
    required this.latitude,
    required this.longitude,
    required this.timestamp,
    this.status = AlertStatus.pending,
    this.note,
  });

  String get statusText {
    switch (status) {
      case AlertStatus.pending:
        return '待处理';
      case AlertStatus.processing:
        return '处理中';
      case AlertStatus.resolved:
        return '已解决';
    }
  }
}
