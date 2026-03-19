import 'dart:convert';
import 'package:shared_preferences/shared_preferences.dart';
import '../models/contact.dart';

class SettingsService {
  static const _keyContacts = 'emergency_contacts';
  static const _keySensitivity = 'alert_sensitivity';
  static const _keyDeviceId = 'device_id';
  static const _keySyncFrequency = 'sync_frequency';
  static const _keyApiKey = 'vision_api_key';

  late SharedPreferences _prefs;

  Future<void> init() async {
    _prefs = await SharedPreferences.getInstance();
  }

  // 紧急联系人
  List<EmergencyContact> getContacts() {
    final json = _prefs.getString(_keyContacts);
    if (json == null) return _defaultContacts;
    final list = jsonDecode(json) as List;
    return list.map((e) => EmergencyContact.fromJson(e)).toList();
  }

  Future<void> saveContacts(List<EmergencyContact> contacts) async {
    await _prefs.setString(
        _keyContacts, jsonEncode(contacts.map((e) => e.toJson()).toList()));
  }

  static List<EmergencyContact> get _defaultContacts => [
        EmergencyContact(id: '1', name: '张三（父亲）', phone: '13800138000', relation: '父亲'),
        EmergencyContact(id: '2', name: '李四（母亲）', phone: '13900139000', relation: '母亲'),
      ];

  // 预警灵敏度
  String getSensitivity() => _prefs.getString(_keySensitivity) ?? '中';
  Future<void> setSensitivity(String v) => _prefs.setString(_keySensitivity, v);

  // 设备 ID
  String getDeviceId() => _prefs.getString(_keyDeviceId) ?? '';
  Future<void> setDeviceId(String v) => _prefs.setString(_keyDeviceId, v);

  // 同步频率
  int getSyncFrequency() => _prefs.getInt(_keySyncFrequency) ?? 5;
  Future<void> setSyncFrequency(int seconds) =>
      _prefs.setInt(_keySyncFrequency, seconds);

  // 视觉识别 API Key
  String getApiKey() => _prefs.getString(_keyApiKey) ?? '';
  Future<void> setApiKey(String v) => _prefs.setString(_keyApiKey, v);
}
