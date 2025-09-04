enum HealthAlertType {
  illnessPrediction,
  lowHrv,
  highRestingHr,
  lowO2Saturation,
  abnormalTemperature,
  sleepDisorder,
  overtraining,
  recovery,
}

enum AlertSeverity {
  low,
  medium,
  high,
  critical,
}

class HealthAlert {
  final HealthAlertType type;
  final String message;
  final AlertSeverity severity;
  final DateTime timestamp;
  final Map<String, dynamic>? metadata;
  final bool acknowledged;

  HealthAlert({
    required this.type,
    required this.message,
    required this.severity,
    required this.timestamp,
    this.metadata,
    this.acknowledged = false,
  });

  HealthAlert copyWith({
    HealthAlertType? type,
    String? message,
    AlertSeverity? severity,
    DateTime? timestamp,
    Map<String, dynamic>? metadata,
    bool? acknowledged,
  }) {
    return HealthAlert(
      type: type ?? this.type,
      message: message ?? this.message,
      severity: severity ?? this.severity,
      timestamp: timestamp ?? this.timestamp,
      metadata: metadata ?? this.metadata,
      acknowledged: acknowledged ?? this.acknowledged,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'type': type.toString(),
      'message': message,
      'severity': severity.toString(),
      'timestamp': timestamp.toIso8601String(),
      'metadata': metadata,
      'acknowledged': acknowledged,
    };
  }

  factory HealthAlert.fromJson(Map<String, dynamic> json) {
    return HealthAlert(
      type: HealthAlertType.values.firstWhere(
        (e) => e.toString() == json['type'],
      ),
      message: json['message'],
      severity: AlertSeverity.values.firstWhere(
        (e) => e.toString() == json['severity'],
      ),
      timestamp: DateTime.parse(json['timestamp']),
      metadata: json['metadata'],
      acknowledged: json['acknowledged'] ?? false,
    );
  }
}

class HeartRateData {
  final int heartRate;
  final DateTime timestamp;
  final List<int>? rrIntervals; // RR intervals in milliseconds
  final double quality; // Signal quality percentage (0-100)
  final Map<String, dynamic>? metadata;

  HeartRateData({
    required this.heartRate,
    required this.timestamp,
    this.rrIntervals,
    this.quality = 0.0,
    this.metadata,
  });

  Map<String, dynamic> toJson() {
    return {
      'heartRate': heartRate,
      'timestamp': timestamp.toIso8601String(),
      'rrIntervals': rrIntervals,
      'quality': quality,
      'metadata': metadata,
    };
  }

  factory HeartRateData.fromJson(Map<String, dynamic> json) {
    return HeartRateData(
      heartRate: json['heartRate'],
      timestamp: DateTime.parse(json['timestamp']),
      rrIntervals: json['rrIntervals']?.cast<int>(),
      quality: json['quality']?.toDouble() ?? 0.0,
      metadata: json['metadata'],
    );
  }
}

class VitalsData {
  final double? hrv; // Heart Rate Variability (RMSSD)
  final double? sdnn; // Standard deviation of NN intervals
  final double? temperature; // Body temperature in Celsius
  final double? respiratoryRate; // Breaths per minute
  final double? oxygenSaturation; // SpO2 percentage
  final DateTime timestamp;
  final Map<String, dynamic>? metadata;

  VitalsData({
    this.hrv,
    this.sdnn,
    this.temperature,
    this.respiratoryRate,
    this.oxygenSaturation,
    required this.timestamp,
    this.metadata,
  });

  Map<String, dynamic> toJson() {
    return {
      'hrv': hrv,
      'sdnn': sdnn,
      'temperature': temperature,
      'respiratoryRate': respiratoryRate,
      'oxygenSaturation': oxygenSaturation,
      'timestamp': timestamp.toIso8601String(),
      'metadata': metadata,
    };
  }

  factory VitalsData.fromJson(Map<String, dynamic> json) {
    return VitalsData(
      hrv: json['hrv']?.toDouble(),
      sdnn: json['sdnn']?.toDouble(),
      temperature: json['temperature']?.toDouble(),
      respiratoryRate: json['respiratoryRate']?.toDouble(),
      oxygenSaturation: json['oxygenSaturation']?.toDouble(),
      timestamp: DateTime.parse(json['timestamp']),
      metadata: json['metadata'],
    );
  }
}

class SleepData {
  final DateTime sleepStart;
  final DateTime sleepEnd;
  final Duration totalSleep;
  final Duration deepSleep;
  final Duration lightSleep;
  final Duration remSleep;
  final Duration awakeTime;
  final double sleepEfficiency; // Percentage
  final int sleepScore; // 0-100
  final List<SleepStage> sleepStages;
  final Map<String, dynamic>? metadata;

  SleepData({
    required this.sleepStart,
    required this.sleepEnd,
    required this.totalSleep,
    required this.deepSleep,
    required this.lightSleep,
    required this.remSleep,
    required this.awakeTime,
    required this.sleepEfficiency,
    required this.sleepScore,
    required this.sleepStages,
    this.metadata,
  });

  Map<String, dynamic> toJson() {
    return {
      'sleepStart': sleepStart.toIso8601String(),
      'sleepEnd': sleepEnd.toIso8601String(),
      'totalSleep': totalSleep.inMinutes,
      'deepSleep': deepSleep.inMinutes,
      'lightSleep': lightSleep.inMinutes,
      'remSleep': remSleep.inMinutes,
      'awakeTime': awakeTime.inMinutes,
      'sleepEfficiency': sleepEfficiency,
      'sleepScore': sleepScore,
      'sleepStages': sleepStages.map((stage) => stage.toJson()).toList(),
      'metadata': metadata,
    };
  }

  factory SleepData.fromJson(Map<String, dynamic> json) {
    return SleepData(
      sleepStart: DateTime.parse(json['sleepStart']),
      sleepEnd: DateTime.parse(json['sleepEnd']),
      totalSleep: Duration(minutes: json['totalSleep']),
      deepSleep: Duration(minutes: json['deepSleep']),
      lightSleep: Duration(minutes: json['lightSleep']),
      remSleep: Duration(minutes: json['remSleep']),
      awakeTime: Duration(minutes: json['awakeTime']),
      sleepEfficiency: json['sleepEfficiency'].toDouble(),
      sleepScore: json['sleepScore'],
      sleepStages: (json['sleepStages'] as List)
          .map((stage) => SleepStage.fromJson(stage))
          .toList(),
      metadata: json['metadata'],
    );
  }
}

enum SleepStageType {
  awake,
  light,
  deep,
  rem,
}

class SleepStage {
  final SleepStageType type;
  final DateTime startTime;
  final Duration duration;

  SleepStage({
    required this.type,
    required this.startTime,
    required this.duration,
  });

  Map<String, dynamic> toJson() {
    return {
      'type': type.toString(),
      'startTime': startTime.toIso8601String(),
      'duration': duration.inMinutes,
    };
  }

  factory SleepStage.fromJson(Map<String, dynamic> json) {
    return SleepStage(
      type: SleepStageType.values.firstWhere(
        (e) => e.toString() == json['type'],
      ),
      startTime: DateTime.parse(json['startTime']),
      duration: Duration(minutes: json['duration']),
    );
  }
}

class ActivityData {
  final String activityType; // walking, running, cycling, etc.
  final DateTime startTime;
  final DateTime endTime;
  final Duration duration;
  final double? distance; // in meters
  final int? steps;
  final double? calories;
  final double? averageHeartRate;
  final double? maxHeartRate;
  final int strain; // 0-21 scale like Whoop
  final Map<String, dynamic>? metadata;

  ActivityData({
    required this.activityType,
    required this.startTime,
    required this.endTime,
    required this.duration,
    this.distance,
    this.steps,
    this.calories,
    this.averageHeartRate,
    this.maxHeartRate,
    required this.strain,
    this.metadata,
  });

  Map<String, dynamic> toJson() {
    return {
      'activityType': activityType,
      'startTime': startTime.toIso8601String(),
      'endTime': endTime.toIso8601String(),
      'duration': duration.inMinutes,
      'distance': distance,
      'steps': steps,
      'calories': calories,
      'averageHeartRate': averageHeartRate,
      'maxHeartRate': maxHeartRate,
      'strain': strain,
      'metadata': metadata,
    };
  }

  factory ActivityData.fromJson(Map<String, dynamic> json) {
    return ActivityData(
      activityType: json['activityType'],
      startTime: DateTime.parse(json['startTime']),
      endTime: DateTime.parse(json['endTime']),
      duration: Duration(minutes: json['duration']),
      distance: json['distance']?.toDouble(),
      steps: json['steps'],
      calories: json['calories']?.toDouble(),
      averageHeartRate: json['averageHeartRate']?.toDouble(),
      maxHeartRate: json['maxHeartRate']?.toDouble(),
      strain: json['strain'],
      metadata: json['metadata'],
    );
  }
}

class RecoveryData {
  final DateTime date;
  final double recoveryScore; // 0-100 like Whoop
  final double restingHeartRate;
  final double hrvScore;
  final double sleepPerformance;
  final String recoveryState; // optimal, adequate, compromised
  final Map<String, dynamic>? metadata;

  RecoveryData({
    required this.date,
    required this.recoveryScore,
    required this.restingHeartRate,
    required this.hrvScore,
    required this.sleepPerformance,
    required this.recoveryState,
    this.metadata,
  });

  Map<String, dynamic> toJson() {
    return {
      'date': date.toIso8601String(),
      'recoveryScore': recoveryScore,
      'restingHeartRate': restingHeartRate,
      'hrvScore': hrvScore,
      'sleepPerformance': sleepPerformance,
      'recoveryState': recoveryState,
      'metadata': metadata,
    };
  }

  factory RecoveryData.fromJson(Map<String, dynamic> json) {
    return RecoveryData(
      date: DateTime.parse(json['date']),
      recoveryScore: json['recoveryScore'].toDouble(),
      restingHeartRate: json['restingHeartRate'].toDouble(),
      hrvScore: json['hrvScore'].toDouble(),
      sleepPerformance: json['sleepPerformance'].toDouble(),
      recoveryState: json['recoveryState'],
      metadata: json['metadata'],
    );
  }
}

// Simple health data point for dashboard
class HealthDataPoint {
  final DateTime timestamp;
  final double healthScore;

  HealthDataPoint({
    required this.timestamp,
    required this.healthScore,
  });

  static List<HealthDataPoint> generateMockWeekData() {
    final now = DateTime.now();
    return List.generate(7, (index) {
      return HealthDataPoint(
        timestamp: now.subtract(Duration(days: 6 - index)),
        healthScore: 80 + (index * 2) + (index % 4) * 3,
      );
    });
  }

  static List<HealthDataPoint> generateMockRecoveryData() {
    final now = DateTime.now();
    return List.generate(7, (index) {
      // Simulate recovery from illness
      final recoveryProgress = index / 6.0;
      return HealthDataPoint(
        timestamp: now.subtract(Duration(days: 6 - index)),
        healthScore: 40 + (recoveryProgress * 40),
      );
    });
  }
}
