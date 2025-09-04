import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import '../../screens/home_screen.dart';
import '../../screens/device_connection_screen.dart';
import '../../screens/health_dashboard_screen_simple.dart';
import '../../screens/sleep_analysis_screen.dart';
import '../../screens/settings_screen.dart';

class AppRoutes {
  static final GoRouter router = GoRouter(
    initialLocation: '/',
    routes: [
      GoRoute(
        path: '/',
        name: 'home',
        builder: (context, state) => const HomeScreen(),
      ),
      GoRoute(
        path: '/device',
        name: 'device',
        builder: (context, state) => const DeviceConnectionScreen(),
      ),
      GoRoute(
        path: '/health',
        name: 'health',
        builder: (context, state) => const HealthDashboardScreen(),
      ),
      GoRoute(
        path: '/sleep',
        name: 'sleep',
        builder: (context, state) => const SleepAnalysisScreen(),
      ),
      GoRoute(
        path: '/settings',
        name: 'settings',
        builder: (context, state) => const SettingsScreen(),
      ),
    ],
  );
}
