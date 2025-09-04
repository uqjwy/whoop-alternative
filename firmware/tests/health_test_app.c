#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "../modules/health_monitor/health_monitor.h"

LOG_MODULE_REGISTER(health_test_app, LOG_LEVEL_DBG);

// Test configuration
#define HEALTH_TEST_DURATION_DAYS 5
#define VITALS_UPDATE_INTERVAL_MS (6 * 60 * 60 * 1000)  // Every 6 hours
#define SLEEP_UPDATE_INTERVAL_MS (24 * 60 * 60 * 1000)  // Daily

// Global test data
static health_monitor_t g_health_monitor;
static int test_day = 0;
static bool test_running = false;

// Simulated health scenarios
typedef enum {
    SCENARIO_HEALTHY = 0,
    SCENARIO_GETTING_SICK,
    SCENARIO_SICK,
    SCENARIO_RECOVERING,
    SCENARIO_OVERTRAINED
} health_scenario_t;

static void generate_vitals_data(vitals_data_t *vitals, health_scenario_t scenario, int day)
{
    vitals->timestamp = k_uptime_get();
    
    // Base healthy values
    float base_hrv = 50.0f;
    float base_rhr = 60.0f;
    float base_temp = 36.5f;
    
    // Add some random variation
    float hrv_noise = ((rand() % 100) / 100.0f - 0.5f) * 5.0f;
    float rhr_noise = ((rand() % 100) / 100.0f - 0.5f) * 3.0f;
    float temp_noise = ((rand() % 100) / 100.0f - 0.5f) * 0.2f;
    
    switch (scenario) {
        case SCENARIO_HEALTHY:
            vitals->hrv = base_hrv + hrv_noise;
            vitals->resting_heart_rate = base_rhr + rhr_noise;
            vitals->temperature = base_temp + temp_noise;
            break;
            
        case SCENARIO_GETTING_SICK:
            // Gradual changes indicating illness onset
            vitals->hrv = base_hrv - (day * 5.0f) + hrv_noise;  // Decreasing HRV
            vitals->resting_heart_rate = base_rhr + (day * 2.0f) + rhr_noise;  // Increasing RHR
            vitals->temperature = base_temp + (day * 0.1f) + temp_noise;  // Slight temperature increase
            break;
            
        case SCENARIO_SICK:
            // Clear illness indicators
            vitals->hrv = base_hrv - 20.0f + hrv_noise;  // Low HRV
            vitals->resting_heart_rate = base_rhr + 15.0f + rhr_noise;  // High RHR
            vitals->temperature = base_temp + 1.5f + temp_noise;  // Fever
            break;
            
        case SCENARIO_RECOVERING:
            // Gradual improvement
            vitals->hrv = (base_hrv - 15.0f) + (day * 3.0f) + hrv_noise;  // Improving HRV
            vitals->resting_heart_rate = (base_rhr + 10.0f) - (day * 2.0f) + rhr_noise;  // Normalizing RHR
            vitals->temperature = (base_temp + 0.5f) - (day * 0.1f) + temp_noise;  // Normalizing temperature
            break;
            
        case SCENARIO_OVERTRAINED:
            // Overtraining syndrome indicators
            vitals->hrv = base_hrv - 15.0f + hrv_noise;  // Persistently low HRV
            vitals->resting_heart_rate = base_rhr + 8.0f + rhr_noise;  // Elevated RHR
            vitals->temperature = base_temp + temp_noise;  // Normal temperature
            break;
    }
    
    // Ensure realistic ranges
    if (vitals->hrv < 10.0f) vitals->hrv = 10.0f;
    if (vitals->hrv > 100.0f) vitals->hrv = 100.0f;
    if (vitals->resting_heart_rate < 40.0f) vitals->resting_heart_rate = 40.0f;
    if (vitals->resting_heart_rate > 100.0f) vitals->resting_heart_rate = 100.0f;
    if (vitals->temperature < 35.0f) vitals->temperature = 35.0f;
    if (vitals->temperature > 40.0f) vitals->temperature = 40.0f;
    
    // Add other vitals
    vitals->respiratory_rate = 16.0f + ((rand() % 100) / 100.0f - 0.5f) * 4.0f;
    vitals->oxygen_saturation = 98.0f + ((rand() % 100) / 100.0f - 0.5f) * 2.0f;
}

static void generate_sleep_data(sleep_data_t *sleep, health_scenario_t scenario)
{
    // Base sleep values (good sleep)
    sleep->total_sleep_minutes = 480;  // 8 hours
    sleep->deep_sleep_minutes = 96;    // 20% deep sleep
    sleep->rem_sleep_minutes = 96;     // 20% REM sleep
    sleep->light_sleep_minutes = 240;  // 50% light sleep
    sleep->awake_minutes = 48;         // 10% awake time
    sleep->efficiency = 90.0f;
    sleep->wake_episodes = 2;
    sleep->bedtime = k_uptime_get() - (12 * 60 * 60 * 1000);  // 12 hours ago
    sleep->wake_time = k_uptime_get() - (4 * 60 * 60 * 1000);  // 4 hours ago
    
    // Modify based on scenario
    switch (scenario) {
        case SCENARIO_HEALTHY:
            // Already set to good values
            break;
            
        case SCENARIO_GETTING_SICK:
        case SCENARIO_SICK:
            // Poor sleep quality when sick
            sleep->total_sleep_minutes = 420;  // 7 hours
            sleep->deep_sleep_minutes = 50;    // Reduced deep sleep
            sleep->rem_sleep_minutes = 60;     // Reduced REM
            sleep->awake_minutes = 90;         // More awake time
            sleep->efficiency = 75.0f;
            sleep->wake_episodes = 5;
            break;
            
        case SCENARIO_RECOVERING:
            // Improving sleep
            sleep->total_sleep_minutes = 510;  // 8.5 hours
            sleep->deep_sleep_minutes = 110;   // More deep sleep for recovery
            sleep->efficiency = 85.0f;
            sleep->wake_episodes = 3;
            break;
            
        case SCENARIO_OVERTRAINED:
            // Poor sleep from overtraining
            sleep->total_sleep_minutes = 360;  // 6 hours
            sleep->deep_sleep_minutes = 40;    // Very little deep sleep
            sleep->awake_minutes = 120;        // Lots of awake time
            sleep->efficiency = 70.0f;
            sleep->wake_episodes = 8;
            break;
    }
    
    // Calculate light sleep as remainder
    sleep->light_sleep_minutes = sleep->total_sleep_minutes - 
                                sleep->deep_sleep_minutes - 
                                sleep->rem_sleep_minutes - 
                                sleep->awake_minutes;
}

static void test_timer_handler(struct k_timer *timer)
{
    if (!test_running || test_day >= HEALTH_TEST_DURATION_DAYS) {
        test_running = false;
        return;
    }
    
    // Determine health scenario for this day
    health_scenario_t scenario = SCENARIO_HEALTHY;
    if (test_day == 0) scenario = SCENARIO_HEALTHY;
    else if (test_day == 1) scenario = SCENARIO_GETTING_SICK;
    else if (test_day == 2) scenario = SCENARIO_SICK;
    else if (test_day == 3) scenario = SCENARIO_RECOVERING;
    else if (test_day == 4) scenario = SCENARIO_OVERTRAINED;
    
    LOG_INF("=== Day %d - Scenario: %d ===", test_day + 1, scenario);
    
    // Generate and update vitals (multiple times per day)
    for (int i = 0; i < 4; i++) {  // 4 vitals readings per day
        vitals_data_t vitals;
        generate_vitals_data(&vitals, scenario, i);
        
        int ret = health_monitor_update_vitals(&g_health_monitor, &vitals);
        if (ret != 0) {
            LOG_ERR("Failed to update vitals: %d", ret);
            continue;
        }
        
        LOG_DBG("Vitals %d: HRV=%.1f, RHR=%.1f, Temp=%.1f°C", 
                i + 1, vitals.hrv, vitals.resting_heart_rate, vitals.temperature);
    }
    
    // Generate and update sleep data
    sleep_data_t sleep;
    generate_sleep_data(&sleep, scenario);
    
    int ret = health_monitor_update_sleep(&g_health_monitor, &sleep);
    if (ret != 0) {
        LOG_ERR("Failed to update sleep: %d", ret);
    } else {
        LOG_DBG("Sleep: %d min total, %d min deep, %.1f%% efficiency", 
                sleep.total_sleep_minutes, sleep.deep_sleep_minutes, sleep.efficiency);
    }
    
    // Get health status
    health_status_t status;
    ret = health_monitor_get_health_status(&g_health_monitor, &status);
    if (ret == 0) {
        LOG_INF("Health Status:");
        LOG_INF("  Overall Score: %.1f", status.overall_score);
        LOG_INF("  Illness Probability: %.1f%%", status.illness_probability * 100);
        LOG_INF("  Recovery State: %d", status.recovery_state);
        LOG_INF("  HRV Deviation: %.1f%%", status.hrv_deviation * 100);
        LOG_INF("  RHR Deviation: %.1f%%", status.rhr_deviation * 100);
        LOG_INF("  Sleep Score: %d", status.sleep_score);
        LOG_INF("  Has Fever: %s", status.has_fever ? "Yes" : "No");
    }
    
    // Check for health alerts
    health_alert_t alerts[10];
    int alert_count = health_monitor_check_alerts(&g_health_monitor, alerts, 10);
    if (alert_count > 0) {
        LOG_INF("Health Alerts (%d):", alert_count);
        for (int i = 0; i < alert_count; i++) {
            LOG_INF("  [%d] %s (Confidence: %.0f%%)", 
                    alerts[i].severity, alerts[i].message, alerts[i].confidence * 100);
        }
    } else {
        LOG_INF("No health alerts");
    }
    
    test_day++;
    
    if (test_day >= HEALTH_TEST_DURATION_DAYS) {
        LOG_INF("=== Health monitoring test completed ===");
        test_running = false;
    }
}

K_TIMER_DEFINE(test_timer, test_timer_handler, NULL);

static int run_health_test(void)
{
    LOG_INF("Starting health monitoring test...");
    
    // Initialize health monitor
    int ret = health_monitor_init(&g_health_monitor);
    if (ret != 0) {
        LOG_ERR("Failed to initialize health monitor: %d", ret);
        return ret;
    }
    
    // Reset test variables
    test_running = true;
    test_day = 0;
    
    LOG_INF("Test will simulate %d days of health data", HEALTH_TEST_DURATION_DAYS);
    LOG_INF("Scenarios: Healthy -> Getting Sick -> Sick -> Recovering -> Overtrained");
    
    // Start the test timer (accelerated - 1 second per day)
    k_timer_start(&test_timer, K_SECONDS(1), K_SECONDS(1));
    
    // Wait for test to complete
    while (test_running) {
        k_sleep(K_MSEC(100));
    }
    
    k_timer_stop(&test_timer);
    
    LOG_INF("Health monitoring test completed successfully");
    return 0;
}

int main(void)
{
    LOG_INF("Health Monitoring Test Application Started");
    LOG_INF("Testing illness prediction and health trend analysis");
    
    // Wait a moment for system to stabilize
    k_sleep(K_SECONDS(1));
    
    // Run the test
    int ret = run_health_test();
    if (ret != 0) {
        LOG_ERR("Health monitoring test failed with error: %d", ret);
        return ret;
    }
    
    LOG_INF("All tests completed. System idle.");
    
    // Keep the application running
    while (1) {
        k_sleep(K_SECONDS(10));
    }
    
    return 0;
}
