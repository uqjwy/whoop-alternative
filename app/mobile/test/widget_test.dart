// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'package:whoop_alternative_app/main.dart';

void main() {
  testWidgets('Whoop Alternative App smoke test', (WidgetTester tester) async {
    // Build our app and trigger a frame.
    await tester.pumpWidget(const ProviderScope(child: WhoopAlternativeApp()));

    // Verify that the app loads without errors
    await tester.pumpAndSettle();
    
    // Basic smoke test - app should load without exceptions
    expect(tester.takeException(), isNull);
    
    // Verify MaterialApp is present
    expect(find.byType(MaterialApp), findsOneWidget);
  });
}
