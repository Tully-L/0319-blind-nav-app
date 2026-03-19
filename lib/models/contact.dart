class EmergencyContact {
  String id;
  String name;
  String phone;
  String relation;

  EmergencyContact({
    required this.id,
    required this.name,
    required this.phone,
    this.relation = '',
  });

  factory EmergencyContact.fromJson(Map<String, dynamic> json) {
    return EmergencyContact(
      id: json['id'] as String,
      name: json['name'] as String,
      phone: json['phone'] as String,
      relation: json['relation'] as String? ?? '',
    );
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'name': name,
        'phone': phone,
        'relation': relation,
      };
}
