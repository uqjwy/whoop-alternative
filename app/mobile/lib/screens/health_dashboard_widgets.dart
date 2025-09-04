// Additional Health Dashboard Widgets
// This file contains the remaining widget methods for health_dashboard_screen.dart

  Widget _buildVitalsRow() {
    return Row(
      children: [
        Expanded(
          child: _buildVitalCard(
            'HRV',
            '${_currentHealth!.currentHrv.toInt()} ms',
            Icons.favorite,
            Colors.red,
            'Baseline: ${_currentHealth!.hrvBaseline.toInt()} ms',
          ),
        ),
        const SizedBox(width: 8),
        Expanded(
          child: _buildVitalCard(
            'RHR',
            '${_currentHealth!.currentRhr.toInt()} bpm',
            Icons.monitor_heart,
            Colors.orange,
            'Baseline: ${_currentHealth!.rhrBaseline.toInt()} bpm',
          ),
        ),
        const SizedBox(width: 8),
        Expanded(
          child: _buildVitalCard(
            'Temp',
            '${_currentHealth!.currentTemperature.toStringAsFixed(1)}°C',
            Icons.thermostat,
            _currentHealth!.currentTemperature > 37.5 ? Colors.red : Colors.blue,
            _currentHealth!.currentTemperature > 37.5 ? 'FEVER!' : 'Normal',
          ),
        ),
      ],
    );
  }

  Widget _buildVitalCard(String title, String value, IconData icon, Color color, String subtitle) {
    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Column(
          children: [
            Icon(icon, color: color, size: 24),
            const SizedBox(height: 8),
            Text(
              title,
              style: const TextStyle(color: Colors.grey, fontSize: 12),
            ),
            const SizedBox(height: 4),
            Text(
              value,
              style: const TextStyle(
                color: Colors.white,
                fontSize: 16,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 4),
            Text(
              subtitle,
              style: TextStyle(
                color: Colors.grey[600],
                fontSize: 10,
              ),
              textAlign: TextAlign.center,
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildRecoveryCard() {
    final recovery = _currentHealth!.recoveryState;
    final color = _getRecoveryColor(recovery);
    
    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Row(
          children: [
            Icon(
              _getRecoveryIcon(recovery),
              color: color,
              size: 32,
            ),
            const SizedBox(width: 16),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const Text(
                    'Recovery Status',
                    style: TextStyle(color: Colors.grey, fontSize: 14),
                  ),
                  const SizedBox(height: 4),
                  Text(
                    recovery.displayName,
                    style: TextStyle(
                      color: color,
                      fontSize: 20,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const SizedBox(height: 4),
                  Text(
                    _getRecoveryRecommendation(recovery),
                    style: const TextStyle(color: Colors.white, fontSize: 14),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildIllnessPredictionCard() {
    final probability = _currentHealth!.illnessProbability;
    final risk = probability * 100;
    final color = _getIllnessRiskColor(probability);
    
    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(
                  Icons.health_and_safety,
                  color: color,
                ),
                const SizedBox(width: 8),
                const Text(
                  'Illness Prediction',
                  style: TextStyle(color: Colors.white, fontSize: 16),
                ),
              ],
            ),
            const SizedBox(height: 12),
            LinearProgressIndicator(
              value: probability,
              backgroundColor: Colors.grey[800],
              valueColor: AlwaysStoppedAnimation<Color>(color),
            ),
            const SizedBox(height: 8),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Text(
                  '${risk.toInt()}% Risk',
                  style: TextStyle(
                    color: color,
                    fontSize: 16,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                Text(
                  _getIllnessRiskText(probability),
                  style: TextStyle(
                    color: color,
                    fontSize: 14,
                  ),
                ),
              ],
            ),
            if (probability > 0.5) ...[
              const SizedBox(height: 8),
              Container(
                padding: const EdgeInsets.all(8),
                decoration: BoxDecoration(
                  color: color.withOpacity(0.1),
                  borderRadius: BorderRadius.circular(8),
                ),
                child: Row(
                  children: [
                    Icon(Icons.warning, color: color, size: 16),
                    const SizedBox(width: 8),
                    Expanded(
                      child: Text(
                        'Consider rest and monitoring. Consult healthcare provider if symptoms develop.',
                        style: TextStyle(
                          color: color,
                          fontSize: 12,
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ],
        ),
      ),
    );
  }

  Widget _buildHealthTrendsChart() {
    if (_healthHistory.isEmpty) {
      return const SizedBox.shrink();
    }

    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Health Trends (7 Days)',
              style: TextStyle(color: Colors.white, fontSize: 16),
            ),
            const SizedBox(height: 16),
            SizedBox(
              height: 200,
              child: LineChart(
                LineChartData(
                  gridData: FlGridData(
                    show: true,
                    drawVerticalLine: true,
                    getDrawingVerticalLine: (value) => FlLine(
                      color: Colors.grey[800]!,
                      strokeWidth: 1,
                    ),
                    getDrawingHorizontalLine: (value) => FlLine(
                      color: Colors.grey[800]!,
                      strokeWidth: 1,
                    ),
                  ),
                  titlesData: FlTitlesData(
                    leftTitles: AxisTitles(
                      sideTitles: SideTitles(
                        showTitles: true,
                        reservedSize: 40,
                        getTitlesWidget: (value, meta) {
                          return Text(
                            '${value.toInt()}',
                            style: const TextStyle(color: Colors.grey, fontSize: 10),
                          );
                        },
                      ),
                    ),
                    bottomTitles: AxisTitles(
                      sideTitles: SideTitles(
                        showTitles: true,
                        getTitlesWidget: (value, meta) {
                          final days = ['M', 'T', 'W', 'T', 'F', 'S', 'S'];
                          final index = value.toInt() % 7;
                          return Text(
                            days[index],
                            style: const TextStyle(color: Colors.grey, fontSize: 10),
                          );
                        },
                      ),
                    ),
                    topTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
                    rightTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
                  ),
                  borderData: FlBorderData(show: false),
                  lineBarsData: [
                    LineChartBarData(
                      spots: _healthHistory.asMap().entries.map((entry) {
                        return FlSpot(entry.key.toDouble(), entry.value.healthScore);
                      }).toList(),
                      isCurved: true,
                      color: const Color(0xFF00D4AA),
                      barWidth: 3,
                      dotData: FlDotData(show: false),
                      belowBarData: BarAreaData(
                        show: true,
                        color: const Color(0xFF00D4AA).withOpacity(0.1),
                      ),
                    ),
                  ],
                  minY: 0,
                  maxY: 100,
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildHRVChart() {
    if (_healthHistory.isEmpty) {
      return const SizedBox.shrink();
    }

    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'HRV Trend (7 Days)',
              style: TextStyle(color: Colors.white, fontSize: 16),
            ),
            const SizedBox(height: 16),
            SizedBox(
              height: 150,
              child: LineChart(
                LineChartData(
                  gridData: FlGridData(
                    show: true,
                    getDrawingHorizontalLine: (value) => FlLine(
                      color: Colors.grey[800]!,
                      strokeWidth: 1,
                    ),
                  ),
                  titlesData: FlTitlesData(
                    leftTitles: AxisTitles(
                      sideTitles: SideTitles(
                        showTitles: true,
                        reservedSize: 40,
                        getTitlesWidget: (value, meta) {
                          return Text(
                            '${value.toInt()}',
                            style: const TextStyle(color: Colors.grey, fontSize: 10),
                          );
                        },
                      ),
                    ),
                    bottomTitles: AxisTitles(
                      sideTitles: SideTitles(showTitles: false),
                    ),
                    topTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
                    rightTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
                  ),
                  borderData: FlBorderData(show: false),
                  lineBarsData: [
                    // HRV trend line
                    LineChartBarData(
                      spots: _healthHistory.asMap().entries.map((entry) {
                        return FlSpot(entry.key.toDouble(), entry.value.hrv);
                      }).toList(),
                      isCurved: true,
                      color: Colors.red,
                      barWidth: 2,
                      dotData: FlDotData(show: true, size: 3),
                    ),
                    // Baseline line
                    LineChartBarData(
                      spots: _healthHistory.asMap().entries.map((entry) {
                        return FlSpot(entry.key.toDouble(), _currentHealth!.hrvBaseline);
                      }).toList(),
                      isCurved: false,
                      color: Colors.red.withOpacity(0.3),
                      barWidth: 1,
                      dotData: FlDotData(show: false),
                      dashArray: [5, 5],
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildSleepCard() {
    final sleepScore = _currentHealth!.sleepScore;
    final color = _getScoreColor(sleepScore.toDouble());
    
    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(Icons.bedtime, color: color),
                const SizedBox(width: 8),
                const Text(
                  'Sleep Quality',
                  style: TextStyle(color: Colors.white, fontSize: 16),
                ),
                const Spacer(),
                Text(
                  '$sleepScore/100',
                  style: TextStyle(
                    color: color,
                    fontSize: 16,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 12),
            LinearProgressIndicator(
              value: sleepScore / 100,
              backgroundColor: Colors.grey[800],
              valueColor: AlwaysStoppedAnimation<Color>(color),
            ),
            const SizedBox(height: 8),
            Text(
              _getSleepQualityText(sleepScore),
              style: TextStyle(
                color: color,
                fontSize: 14,
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildAlertsCard() {
    final activeAlerts = _currentHealth!.activeAlertsCount;
    
    if (activeAlerts == 0) {
      return Card(
        color: const Color(0xFF1A1A1A),
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Row(
            children: [
              Icon(Icons.check_circle, color: Colors.green),
              const SizedBox(width: 8),
              Text(
                'No Active Health Alerts',
                style: TextStyle(
                  color: Colors.green,
                  fontSize: 16,
                ),
              ),
            ],
          ),
        ),
      );
    }

    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(Icons.warning, color: Colors.orange),
                const SizedBox(width: 8),
                Text(
                  'Health Alerts ($activeAlerts)',
                  style: const TextStyle(color: Colors.white, fontSize: 16),
                ),
              ],
            ),
            const SizedBox(height: 8),
            if (_currentHealth!.illnessProbability > 0.5)
              _buildAlertItem('High illness risk detected', Colors.red),
            if (_currentHealth!.currentTemperature > 37.5)
              _buildAlertItem('Fever detected: ${_currentHealth!.currentTemperature.toStringAsFixed(1)}°C', Colors.red),
            if (_currentHealth!.sleepScore < 60)
              _buildAlertItem('Poor sleep quality', Colors.orange),
            if (_currentHealth!.recoveryState == RecoveryState.compromised)
              _buildAlertItem('Recovery compromised - rest recommended', Colors.orange),
          ],
        ),
      ),
    );
  }

  Widget _buildAlertItem(String message, Color color) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: Row(
        children: [
          Icon(Icons.fiber_manual_record, color: color, size: 8),
          const SizedBox(width: 8),
          Expanded(
            child: Text(
              message,
              style: const TextStyle(color: Colors.white, fontSize: 14),
            ),
          ),
        ],
      ),
    );
  }

  // Helper methods
  Color _getScoreColor(double score) {
    if (score >= 85) return const Color(0xFF00D4AA);
    if (score >= 70) return Colors.orange;
    return Colors.red;
  }

  Color _getRecoveryColor(RecoveryState recovery) {
    switch (recovery) {
      case RecoveryState.optimal:
        return const Color(0xFF00D4AA);
      case RecoveryState.adequate:
        return Colors.orange;
      case RecoveryState.compromised:
        return Colors.red;
    }
  }

  IconData _getRecoveryIcon(RecoveryState recovery) {
    switch (recovery) {
      case RecoveryState.optimal:
        return Icons.flash_on;
      case RecoveryState.adequate:
        return Icons.speed;
      case RecoveryState.compromised:
        return Icons.warning;
    }
  }

  Color _getIllnessRiskColor(double probability) {
    if (probability >= 0.7) return Colors.red;
    if (probability >= 0.5) return Colors.orange;
    if (probability >= 0.3) return Colors.yellow[700]!;
    return const Color(0xFF00D4AA);
  }

  String _getHealthStatusText(double score) {
    if (score >= 85) return 'Excellent Health';
    if (score >= 70) return 'Good Health';
    if (score >= 50) return 'Fair Health';
    return 'Poor Health';
  }

  String _getRecoveryRecommendation(RecoveryState recovery) {
    switch (recovery) {
      case RecoveryState.optimal:
        return 'Ready for intense training';
      case RecoveryState.adequate:
        return 'Moderate training recommended';
      case RecoveryState.compromised:
        return 'Rest or light activity only';
    }
  }

  String _getIllnessRiskText(double probability) {
    if (probability >= 0.7) return 'High Risk';
    if (probability >= 0.5) return 'Moderate Risk';
    if (probability >= 0.3) return 'Low Risk';
    return 'Minimal Risk';
  }

  String _getSleepQualityText(int score) {
    if (score >= 85) return 'Excellent sleep quality';
    if (score >= 70) return 'Good sleep quality';
    if (score >= 50) return 'Fair sleep quality';
    return 'Poor sleep quality';
  }

  void _showErrorSnackBar(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: Colors.red,
      ),
    );
  }
}
