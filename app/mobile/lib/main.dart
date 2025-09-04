import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:google_fonts/google_fonts.dart';
// import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'core/theme/app_theme.dart';
import 'core/routes/app_routes.dart';
// import 'core/services/notification_service.dart';
import 'core/services/permission_service.dart';

// final FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin =
//     FlutterLocalNotificationsPlugin();

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // Initialize notifications (temporarily disabled)
  // await NotificationService.initialize();
  
  // Request permissions
  await PermissionService.requestAllPermissions();
  
  runApp(const ProviderScope(child: WhoopAlternativeApp()));
}

class WhoopAlternativeApp extends ConsumerWidget {
  const WhoopAlternativeApp({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return MaterialApp.router(
      title: 'Whoop Alternative',
      debugShowCheckedModeBanner: false,
      theme: AppTheme.lightTheme,
      darkTheme: AppTheme.darkTheme,
      themeMode: ThemeMode.system,
      routerConfig: AppRoutes.router,
    );
  }
}
