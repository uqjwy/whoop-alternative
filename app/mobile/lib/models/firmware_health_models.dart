// Extended Health Models for Firmware Integration
// Bridges firmware health_monitor.h with Flutter app

import 'health_models.dart';

/// Recovery states matching firmware enum
enum RecoveryState {
  optimal,
  adequate, 
  compromised;

  String get displayName {
    switch (this) {
      case RecoveryState.optimal:
        return 'Optimal';
      case RecoveryState.adequate:
        return 'Adequate';
      case RecoveryState.compromised:
        return 'Compromised';
    }
  }
}

/// Comprehensive health status from firmware health monitor
class FirmwareHealthStatus {
  final double overallScore;           // Overall health score (0-100)
  final double illnessProbability;     // Probability of illness (0-1)
  final RecoveryState recoveryState;   // Current recovery state
  final int activeAlertsCount;         // Number of active health alerts
  final bool trendImproving;           // Health trend direction
  final double stressLevel;            // Stress level (0-100)
  
  // Current vitals
  final double currentHrv;
  final double currentRhr;
  final double currentTemperature;
  
  // Baselines
  final double hrvBaseline;
  final double rhrBaseline;
  final double temperatureBaseline;
  
  // Sleep score
  final int sleepScore;
  
  final DateTime lastUpdate;

  const FirmwareHealthStatus({
    required this.overallScore,
    required this.illnessProbability,
    required this.recoveryState,
    required this.activeAlertsCount,
    required this.trendImproving,
    required this.stressLevel,
    required this.currentHrv,
    required this.currentRhr,
    required this.currentTemperature,
    required this.hrvBaseline,
    required this.rhrBaseline,
    required this.temperatureBaseline,
    required this.sleepScore,
    required this.lastUpdate,
  });

  factory FirmwareHealthStatus.fromJson(Map<String, dynamic> json) {
    return FirmwareHealthStatus(
      overallScore: (json['overall_score'] as num).toDouble(),
      illnessProbability: (json['illness_probability'] as num).toDouble(),
      recoveryState: RecoveryState.values[json['recovery_state']],
      activeAlertsCount: json['active_alerts_count'],
      trendImproving: json['trend_improving'],
      stressLevel: (json['stress_level'] as num).toDouble(),
      currentHrv: (json['current_hrv'] as num).toDouble(),
      currentRhr: (json['current_rhr'] as num).toDouble(),
      currentTemperature: (json['current_temperature'] as num).toDouble(),
      hrvBaseline: (json['hrv_baseline'] as num).toDouble(),
      rhrBaseline: (json['rhr_baseline'] as num).toDouble(),
      temperatureBaseline: (json['temperature_baseline'] as num).toDouble(),
      sleepScore: json['sleep_score'],
      lastUpdate: DateTime.fromMillisecondsSinceEpoch(json['last_update']),
    );
  }

  /// Create mock data for testing
  factory FirmwareHealthStatus.mockHealthy() {
    return FirmwareHealthStatus(
      overallScore: 87.5,
      illnessProbability: 0.05,
      recoveryState: RecoveryState.optimal,
      activeAlertsCount: 0,
      trendImproving: true,
      stressLevel: 15.0,
      currentHrv: 52.3,
      currentRhr: 58.0,
      currentTemperature: 36.6,
      hrvBaseline: 50.1,
      rhrBaseline: 60.2,
      temperatureBaseline: 36.5,
      sleepScore: 89,
      lastUpdate: DateTime.now(),
    );
  }

  factory FirmwareHealthStatus.mockGettingSick() {
    return FirmwareHealthStatus(
      overallScore: 72.1,
      illnessProbability: 0.65,
      recoveryState: RecoveryState.adequate,
      activeAlertsCount: 2,
      trendImproving: false,
      stressLevel: 35.0,
      currentHrv: 38.7,
      currentRhr: 68.0,
      currentTemperature: 36.9,
      hrvBaseline: 50.1,
      rhrBaseline: 60.2,
      temperatureBaseline: 36.5,
      sleepScore: 65,
      lastUpdate: DateTime.now(),
    );
  }

  factory FirmwareHealthStatus.mockSick() {
    return FirmwareHealthStatus(
      overallScore: 45.3,
      illnessProbability: 0.92,
      recoveryState: RecoveryState.compromised,
      activeAlertsCount: 4,
      trendImproving: false,
      stressLevel: 75.0,
      currentHrv: 28.2,
      currentRhr: 78.0,
      currentTemperature: 38.1,
      hrvBaseline: 50.1,
      rhrBaseline: 60.2,
      temperatureBaseline: 36.5,
      sleepScore: 42,
      lastUpdate: DateTime.now(),
    );
  }
}

/// Health data point for trends and charts
class HealthDataPoint {
  final double healthScore;
  final double hrv;
  final double rhr;
  final double temperature;
  final double illnessProbability;
  final DateTime timestamp;

  const HealthDataPoint({
    required this.healthScore,
    required this.hrv,
    required this.rhr,
    required this.temperature,
    required this.illnessProbability,
    required this.timestamp,
  });

  factory HealthDataPoint.fromFirmwareHealthStatus(FirmwareHealthStatus status) {
    return HealthDataPoint(
      healthScore: status.overallScore,
      hrv: status.currentHrv,
      rhr: status.currentRhr,
      temperature: status.currentTemperature,
      illnessProbability: status.illnessProbability,
      timestamp: status.lastUpdate,
    );
  }

  /// Generate mock data for 7-day trend
  static List<HealthDataPoint> generateMockWeekData() {
    final List<HealthDataPoint> data = [];
    final now = DateTime.now();
    
    // Simulate gradual decline over 7 days (getting sick scenario)
    for (int i = 6; i >= 0; i--) {
      final day = now.subtract(Duration(days: i));
      final decline = (6 - i) * 0.15; // Gradual decline factor
      
      data.add(HealthDataPoint(
        healthScore: 90.0 - (decline * 30), // 90 down to 63
        hrv: 52.0 - (decline * 15), // 52 down to 30
        rhr: 58.0 + (decline * 8), // 58 up to 70
        temperature: 36.5 + (decline * 0.8), // 36.5 up to 37.1
        illnessProbability: decline * 0.6, // 0 up to 0.54
        timestamp: day,
      ));
    }
    
    return data;
  }

  /// Generate mock recovery data
  static List<HealthDataPoint> generateMockRecoveryData() {
    final List<HealthDataPoint> data = [];
    final now = DateTime.now();
    
    // Simulate recovery over 7 days
    for (int i = 6; i >= 0; i--) {
      final day = now.subtract(Duration(days: i));
      final recovery = (6 - i) * 0.15; // Gradual improvement factor
      
      data.add(HealthDataPoint(
        healthScore: 60.0 + (recovery * 30), // 60 up to 87
        hrv: 30.0 + (recovery * 20), // 30 up to 50
        rhr: 75.0 - (recovery * 15), // 75 down to 62
        temperature: 37.2 - (recovery * 0.7), // 37.2 down to 36.5
        illnessProbability: 0.8 - (recovery * 0.8), // 0.8 down to 0.08
        timestamp: day,
      ));
    }
    
    return data;
  }
}
