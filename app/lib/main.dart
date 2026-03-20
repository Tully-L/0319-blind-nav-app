import 'package:flutter/material.dart';
import 'pages/home_page.dart';
import 'pages/track_page.dart';
import 'pages/sos_page.dart';
import 'pages/vision_page.dart';
import 'pages/settings_page.dart';
import 'pages/about_page.dart';
import 'services/settings_service.dart';
import 'services/vision_service.dart';
import 'utils/app_theme.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  final settings = SettingsService();
  await settings.init();

  // 如果已配置 API Key，加载它
  final apiKey = settings.getApiKey();
  if (apiKey.isNotEmpty) {
    VisionService.setApiKey(apiKey);
  }

  runApp(BlindNavApp(settingsService: settings));
}

class BlindNavApp extends StatelessWidget {
  final SettingsService settingsService;
  const BlindNavApp({required this.settingsService});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: '智能盲人辅助导航',
      theme: AppTheme.theme,
      debugShowCheckedModeBanner: false,
      home: MainScreen(settingsService: settingsService),
      localizationsDelegates: [
        DefaultMaterialLocalizations.delegate,
        DefaultWidgetsLocalizations.delegate,
      ],
    );
  }
}

class MainScreen extends StatefulWidget {
  final SettingsService settingsService;
  const MainScreen({required this.settingsService});

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  int _currentIndex = 0;
  late final List<Widget> _pages;

  @override
  void initState() {
    super.initState();
    _pages = [
      HomePage(),
      TrackPage(),
      SosPage(),
      VisionPage(),
      SettingsPage(settingsService: widget.settingsService),
    ];
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: IndexedStack(
        index: _currentIndex,
        children: _pages,
      ),
      bottomNavigationBar: BottomNavigationBar(
        currentIndex: _currentIndex,
        onTap: (i) => setState(() => _currentIndex = i),
        items: [
          BottomNavigationBarItem(
            icon: Icon(Icons.map),
            activeIcon: Icon(Icons.map, size: 28),
            label: '首页',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.timeline),
            activeIcon: Icon(Icons.timeline, size: 28),
            label: '轨迹',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.sos),
            activeIcon: Icon(Icons.sos, size: 28),
            label: 'SOS',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.visibility),
            activeIcon: Icon(Icons.visibility, size: 28),
            label: '识别',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.settings),
            activeIcon: Icon(Icons.settings, size: 28),
            label: '设置',
          ),
        ],
      ),
      // 关于页通过设置页或导航栏长按进入
      floatingActionButton: _currentIndex == 4
          ? FloatingActionButton.small(
              onPressed: () => Navigator.push(
                  context, MaterialPageRoute(builder: (_) => AboutPage())),
              backgroundColor: AppTheme.primary,
              child: Icon(Icons.info_outline, color: Colors.white),
              tooltip: '关于',
            )
          : null,
    );
  }
}
