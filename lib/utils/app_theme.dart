import 'package:flutter/material.dart';

class AppTheme {
  static const Color primary = Color(0xFF2196F3);
  static const Color primaryDark = Color(0xFF1565C0);
  static const Color accent = Color(0xFFFF9800);
  static const Color danger = Color(0xFFE53935);
  static const Color success = Color(0xFF43A047);
  static const Color background = Color(0xFFF5F5F5);
  static const Color cardBg = Colors.white;
  static const Color textPrimary = Color(0xFF212121);
  static const Color textSecondary = Color(0xFF757575);

  static ThemeData get theme => ThemeData(
        primarySwatch: Colors.blue,
        primaryColor: primary,
        scaffoldBackgroundColor: background,
        appBarTheme: AppBarTheme(
          backgroundColor: primary,
          foregroundColor: Colors.white,
          elevation: 0,
          centerTitle: true,
          titleTextStyle: TextStyle(
            fontSize: 20,
            fontWeight: FontWeight.w600,
            color: Colors.white,
          ),
        ),
        cardTheme: CardTheme(
          elevation: 2,
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
          margin: EdgeInsets.symmetric(horizontal: 16, vertical: 6),
        ),
        elevatedButtonTheme: ElevatedButtonThemeData(
          style: ElevatedButton.styleFrom(
            backgroundColor: primary,
            foregroundColor: Colors.white,
            padding: EdgeInsets.symmetric(horizontal: 24, vertical: 14),
            textStyle: TextStyle(fontSize: 18, fontWeight: FontWeight.w500),
            shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
          ),
        ),
        textTheme: TextTheme(
          headlineLarge: TextStyle(fontSize: 28, fontWeight: FontWeight.bold, color: textPrimary),
          headlineMedium: TextStyle(fontSize: 22, fontWeight: FontWeight.w600, color: textPrimary),
          bodyLarge: TextStyle(fontSize: 18, color: textPrimary),
          bodyMedium: TextStyle(fontSize: 16, color: textSecondary),
        ),
        bottomNavigationBarTheme: BottomNavigationBarThemeData(
          selectedItemColor: primary,
          unselectedItemColor: textSecondary,
          selectedLabelStyle: TextStyle(fontSize: 13, fontWeight: FontWeight.w500),
          unselectedLabelStyle: TextStyle(fontSize: 12),
          type: BottomNavigationBarType.fixed,
        ),
      );
}
