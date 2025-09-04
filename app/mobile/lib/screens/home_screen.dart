import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';
import 'package:fl_chart/fl_chart.dart';
import '../models/health_models.dart';

class HomeScreen extends ConsumerWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Scaffold(
      backgroundColor: const Color(0xFF0A0A0A),
      body: SafeArea(
        child: CustomScrollView(
          slivers: [
            SliverAppBar(
              backgroundColor: Colors.transparent,
              elevation: 0,
              floating: true,
              flexibleSpace: FlexibleSpaceBar(
                title: Text(
                  'Whoop Alternative',
                  style: TextStyle(
                    color: Colors.white,
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
              actions: [
                IconButton(
                  icon: const Icon(Icons.settings, color: Colors.white),
                  onPressed: () => context.push('/settings'),
                ),
              ],
            ),
            SliverPadding(
              padding: const EdgeInsets.all(16),
              sliver: SliverList(
                delegate: SliverChildListDelegate([
                  // Connection Status Card
                  _buildConnectionStatusCard(context),
                  const SizedBox(height: 16),
                  
                  // Quick Stats Row
                  Row(
                    children: [
                      Expanded(child: _buildQuickStatCard('Heart Rate', '72', 'bpm', Colors.red)),
                      const SizedBox(width: 12),
                      Expanded(child: _buildQuickStatCard('HRV', '45', 'ms', Colors.green)),
                    ],
                  ),
                  const SizedBox(height: 12),
                  Row(
                    children: [
                      Expanded(child: _buildQuickStatCard('Recovery', '85', '%', Colors.blue)),
                      const SizedBox(width: 12),
                      Expanded(child: _buildQuickStatCard('Sleep Score', '92', '/100', Colors.purple)),
                    ],
                  ),
                  const SizedBox(height: 24),
                  
                  // Heart Rate Chart
                  _buildHeartRateChart(),
                  const SizedBox(height: 24),
                  
                  // Today's Activity
                  _buildActivitySummary(),
                  const SizedBox(height: 24),
                  
                  // Health Alerts
                  _buildHealthAlerts(),
                  const SizedBox(height: 24),
                  
                  // Navigation Cards
                  _buildNavigationGrid(context),
                ]),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildConnectionStatusCard(BuildContext context) {
    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Row(
          children: [
            Container(
              width: 12,
              height: 12,
              decoration: BoxDecoration(
                color: Colors.grey, // Would be green when connected
                shape: BoxShape.circle,
              ),
            ),
            const SizedBox(width: 12),
            const Expanded(
              child: Text(
                'No device connected',
                style: TextStyle(color: Colors.white, fontSize: 16),
              ),
            ),
            ElevatedButton(
              onPressed: () => context.go('/device'),
              style: ElevatedButton.styleFrom(
                backgroundColor: const Color(0xFFFF0040),
                foregroundColor: Colors.white,
              ),
              child: const Text('Connect'),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildQuickStatCard(String title, String value, String unit, Color color) {
    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              title,
              style: const TextStyle(color: Colors.grey, fontSize: 12),
            ),
            const SizedBox(height: 8),
            Row(
              crossAxisAlignment: CrossAxisAlignment.end,
              children: [
                Text(
                  value,
                  style: TextStyle(
                    color: color,
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                const SizedBox(width: 4),
                Text(
                  unit,
                  style: const TextStyle(color: Colors.grey, fontSize: 12),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildHeartRateChart() {
    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Heart Rate (24h)',
              style: TextStyle(color: Colors.white, fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 16),
            SizedBox(
              height: 200,
              child: LineChart(
                LineChartData(
                  gridData: FlGridData(show: false),
                  titlesData: FlTitlesData(show: false),
                  borderData: FlBorderData(show: false),
                  lineBarsData: [
                    LineChartBarData(
                      spots: _generateMockHeartRateData(),
                      isCurved: true,
                      color: Colors.red,
                      barWidth: 2,
                      dotData: FlDotData(show: false),
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

  Widget _buildActivitySummary() {
    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Today\'s Activity',
              style: TextStyle(color: Colors.white, fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 16),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                _buildActivityMetric('Steps', '8,432', Icons.directions_walk),
                _buildActivityMetric('Calories', '542', Icons.local_fire_department),
                _buildActivityMetric('Distance', '6.2 km', Icons.straighten),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildActivityMetric(String label, String value, IconData icon) {
    return Column(
      children: [
        Icon(icon, color: const Color(0xFF00D4AA), size: 28),
        const SizedBox(height: 8),
        Text(
          value,
          style: const TextStyle(color: Colors.white, fontSize: 16, fontWeight: FontWeight.bold),
        ),
        Text(
          label,
          style: const TextStyle(color: Colors.grey, fontSize: 12),
        ),
      ],
    );
  }

  Widget _buildHealthAlerts() {
    return Card(
      color: const Color(0xFF1A1A1A),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Health Insights',
              style: TextStyle(color: Colors.white, fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 16),
            Container(
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: Colors.green.withOpacity(0.1),
                borderRadius: BorderRadius.circular(8),
                border: Border.all(color: Colors.green.withOpacity(0.3)),
              ),
              child: Row(
                children: [
                  Icon(Icons.check_circle, color: Colors.green, size: 20),
                  const SizedBox(width: 12),
                  const Expanded(
                    child: Text(
                      'Your recovery is optimal. Great job maintaining your health!',
                      style: TextStyle(color: Colors.white, fontSize: 14),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildNavigationGrid(BuildContext context) {
    return GridView.count(
      shrinkWrap: true,
      physics: const NeverScrollableScrollPhysics(),
      crossAxisCount: 2,
      crossAxisSpacing: 12,
      mainAxisSpacing: 12,
      children: [
        _buildNavigationCard(
          'Health Dashboard',
          Icons.favorite,
          Colors.red,
          () => context.push('/health'),
        ),
        _buildNavigationCard(
          'Sleep Analysis',
          Icons.bedtime,
          Colors.blue,
          () => context.push('/sleep'),
        ),
        _buildNavigationCard(
          'Device Settings',
          Icons.bluetooth,
          Colors.green,
          () => context.push('/device'),
        ),
        _buildNavigationCard(
          'App Settings',
          Icons.settings,
          Colors.orange,
          () => context.push('/settings'),
        ),
      ],
    );
  }

  Widget _buildNavigationCard(String title, IconData icon, Color color, VoidCallback onTap) {
    return Card(
      color: const Color(0xFF1A1A1A),
      child: InkWell(
        onTap: onTap,
        borderRadius: BorderRadius.circular(12),
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(icon, color: color, size: 32),
              const SizedBox(height: 8),
              Text(
                title,
                textAlign: TextAlign.center,
                style: const TextStyle(color: Colors.white, fontSize: 14),
              ),
            ],
          ),
        ),
      ),
    );
  }

  List<FlSpot> _generateMockHeartRateData() {
    // Generate 24 hours of mock heart rate data
    return List.generate(24, (index) {
      double baseRate = 60.0;
      // Simulate daily rhythm
      double timeOfDay = index / 24.0;
      double circadianVariation = 10 * (0.5 - (timeOfDay - 0.5).abs());
      double noise = (DateTime.now().millisecond % 10 - 5) * 2.0;
      
      return FlSpot(index.toDouble(), baseRate + circadianVariation + noise);
    });
  }
}
