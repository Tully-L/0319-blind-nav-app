import 'package:flutter/material.dart';
import '../services/settings_service.dart';
import '../services/vision_service.dart';
import '../models/contact.dart';
import '../utils/app_theme.dart';

class SettingsPage extends StatefulWidget {
  final SettingsService settingsService;
  const SettingsPage({required this.settingsService});

  @override
  State<SettingsPage> createState() => _SettingsPageState();
}

class _SettingsPageState extends State<SettingsPage> {
  SettingsService get _settings => widget.settingsService;
  late List<EmergencyContact> _contacts;
  late String _sensitivity;
  late String _deviceId;
  late int _syncFrequency;

  @override
  void initState() {
    super.initState();
    _contacts = _settings.getContacts();
    _sensitivity = _settings.getSensitivity();
    _deviceId = _settings.getDeviceId();
    _syncFrequency = _settings.getSyncFrequency();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('设置')),
      body: ListView(
        children: [
          // 设备绑定
          _sectionHeader('设备绑定'),
          Card(
            child: ListTile(
              leading: Icon(Icons.devices, color: AppTheme.primary, size: 28),
              title: Text('设备 ID', style: TextStyle(fontSize: 16)),
              subtitle: Text(_deviceId.isEmpty ? '未绑定' : _deviceId,
                  style: TextStyle(fontSize: 14)),
              trailing: Icon(Icons.edit, color: AppTheme.textSecondary),
              onTap: _editDeviceId,
            ),
          ),
          // 紧急联系人
          _sectionHeader('紧急联系人'),
          ..._contacts.map((c) => Card(
                child: ListTile(
                  leading: CircleAvatar(
                    backgroundColor: AppTheme.primary.withOpacity(0.1),
                    child: Icon(Icons.person, color: AppTheme.primary),
                  ),
                  title: Text(c.name, style: TextStyle(fontSize: 16)),
                  subtitle: Text('${c.phone}  ${c.relation}',
                      style: TextStyle(fontSize: 14)),
                  trailing: Row(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      IconButton(
                        icon: Icon(Icons.edit, size: 20, color: AppTheme.textSecondary),
                        onPressed: () => _editContact(c),
                      ),
                      IconButton(
                        icon: Icon(Icons.delete, size: 20, color: AppTheme.danger),
                        onPressed: () => _deleteContact(c),
                      ),
                    ],
                  ),
                ),
              )),
          Padding(
            padding: EdgeInsets.symmetric(horizontal: 16, vertical: 8),
            child: OutlinedButton.icon(
              onPressed: _addContact,
              icon: Icon(Icons.add),
              label: Text('添加联系人', style: TextStyle(fontSize: 16)),
              style: OutlinedButton.styleFrom(
                padding: EdgeInsets.symmetric(vertical: 14),
              ),
            ),
          ),
          // 预警灵敏度
          _sectionHeader('预警灵敏度'),
          Card(
            child: Padding(
              padding: EdgeInsets.all(16),
              child: Row(
                children: ['低', '中', '高'].map((level) {
                  final selected = _sensitivity == level;
                  return Expanded(
                    child: Padding(
                      padding: EdgeInsets.symmetric(horizontal: 4),
                      child: ChoiceChip(
                        label: Text(level, style: TextStyle(fontSize: 16)),
                        selected: selected,
                        selectedColor: AppTheme.primary.withOpacity(0.2),
                        onSelected: (_) {
                          setState(() => _sensitivity = level);
                          _settings.setSensitivity(level);
                        },
                      ),
                    ),
                  );
                }).toList(),
              ),
            ),
          ),
          // 数据同步频率
          _sectionHeader('数据同步频率'),
          Card(
            child: Padding(
              padding: EdgeInsets.symmetric(horizontal: 16, vertical: 8),
              child: Row(
                children: [
                  Icon(Icons.sync, color: AppTheme.primary),
                  SizedBox(width: 12),
                  Expanded(
                    child: Text('每 $_syncFrequency 秒同步',
                        style: TextStyle(fontSize: 16)),
                  ),
                  DropdownButton<int>(
                    value: _syncFrequency,
                    items: [2, 5, 10, 30, 60].map((v) {
                      return DropdownMenuItem(
                        value: v,
                        child: Text('$v 秒'),
                      );
                    }).toList(),
                    onChanged: (v) {
                      if (v != null) {
                        setState(() => _syncFrequency = v);
                        _settings.setSyncFrequency(v);
                      }
                    },
                  ),
                ],
              ),
            ),
          ),
          // API Key
          _sectionHeader('视觉识别 API'),
          Card(
            child: ListTile(
              leading: Icon(Icons.key, color: AppTheme.accent, size: 28),
              title: Text('通义千问 API Key', style: TextStyle(fontSize: 16)),
              subtitle: Text(
                _settings.getApiKey().isEmpty ? '未配置（使用演示模式）' : '已配置',
                style: TextStyle(fontSize: 14),
              ),
              trailing: Icon(Icons.edit, color: AppTheme.textSecondary),
              onTap: _editApiKey,
            ),
          ),
          SizedBox(height: 24),
        ],
      ),
    );
  }

  Widget _sectionHeader(String title) {
    return Padding(
      padding: EdgeInsets.fromLTRB(20, 20, 16, 6),
      child: Text(title,
          style: TextStyle(
              fontSize: 15,
              fontWeight: FontWeight.w600,
              color: AppTheme.textSecondary)),
    );
  }

  Future<void> _editDeviceId() async {
    final controller = TextEditingController(text: _deviceId);
    final result = await showDialog<String>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text('绑定设备'),
        content: TextField(
          controller: controller,
          decoration: InputDecoration(
            labelText: '设备 ID',
            hintText: '输入盲杖设备编号',
            border: OutlineInputBorder(),
          ),
          style: TextStyle(fontSize: 18),
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(ctx), child: Text('取消')),
          ElevatedButton(
              onPressed: () => Navigator.pop(ctx, controller.text),
              child: Text('确定')),
        ],
      ),
    );
    if (result != null) {
      setState(() => _deviceId = result);
      _settings.setDeviceId(result);
    }
  }

  Future<void> _editContact(EmergencyContact contact) async {
    final nameCtrl = TextEditingController(text: contact.name);
    final phoneCtrl = TextEditingController(text: contact.phone);
    final relCtrl = TextEditingController(text: contact.relation);

    final confirmed = await showDialog<bool>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text('编辑联系人'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            TextField(controller: nameCtrl, decoration: InputDecoration(labelText: '姓名')),
            SizedBox(height: 8),
            TextField(controller: phoneCtrl, decoration: InputDecoration(labelText: '电话'),
                keyboardType: TextInputType.phone),
            SizedBox(height: 8),
            TextField(controller: relCtrl, decoration: InputDecoration(labelText: '关系')),
          ],
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(ctx, false), child: Text('取消')),
          ElevatedButton(onPressed: () => Navigator.pop(ctx, true), child: Text('保存')),
        ],
      ),
    );
    if (confirmed == true) {
      contact.name = nameCtrl.text;
      contact.phone = phoneCtrl.text;
      contact.relation = relCtrl.text;
      setState(() {});
      _settings.saveContacts(_contacts);
    }
  }

  void _deleteContact(EmergencyContact contact) {
    setState(() => _contacts.remove(contact));
    _settings.saveContacts(_contacts);
  }

  Future<void> _addContact() async {
    final nameCtrl = TextEditingController();
    final phoneCtrl = TextEditingController();
    final relCtrl = TextEditingController();

    final confirmed = await showDialog<bool>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text('添加联系人'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            TextField(controller: nameCtrl, decoration: InputDecoration(labelText: '姓名')),
            SizedBox(height: 8),
            TextField(controller: phoneCtrl, decoration: InputDecoration(labelText: '电话'),
                keyboardType: TextInputType.phone),
            SizedBox(height: 8),
            TextField(controller: relCtrl, decoration: InputDecoration(labelText: '关系')),
          ],
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(ctx, false), child: Text('取消')),
          ElevatedButton(onPressed: () => Navigator.pop(ctx, true), child: Text('添加')),
        ],
      ),
    );
    if (confirmed == true && nameCtrl.text.isNotEmpty) {
      setState(() {
        _contacts.add(EmergencyContact(
          id: DateTime.now().millisecondsSinceEpoch.toString(),
          name: nameCtrl.text,
          phone: phoneCtrl.text,
          relation: relCtrl.text,
        ));
      });
      _settings.saveContacts(_contacts);
    }
  }

  Future<void> _editApiKey() async {
    final controller = TextEditingController(text: _settings.getApiKey());
    final result = await showDialog<String>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text('配置 API Key'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Text('通义千问 VL API Key，留空则使用演示模式。',
                style: TextStyle(fontSize: 14, color: AppTheme.textSecondary)),
            SizedBox(height: 12),
            TextField(
              controller: controller,
              decoration: InputDecoration(
                labelText: 'API Key',
                border: OutlineInputBorder(),
              ),
              obscureText: true,
            ),
          ],
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(ctx), child: Text('取消')),
          ElevatedButton(
              onPressed: () => Navigator.pop(ctx, controller.text),
              child: Text('保存')),
        ],
      ),
    );
    if (result != null) {
      _settings.setApiKey(result);
      VisionService.setApiKey(result);
      setState(() {});
    }
  }
}
