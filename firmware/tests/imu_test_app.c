#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "../drivers/imu/bma400.h"
#include "../modules/imu_algorithms/imu_algorithms.h"

LOG_MODULE_REGISTER(imu_test_app, LOG_LEVEL_DBG);

// Test configuration
#define IMU_TEST_DURATION_SEC 30
#define IMU_SAMPLE_RATE_HZ 50
#define IMU_SAMPLE_INTERVAL_MS (1000 / IMU_SAMPLE_RATE_HZ)

// Global test data
static imu_processor_t g_imu_processor;
static bool test_running = false;
static uint32_t test_samples_processed = 0;

// Simulated IMU data patterns for different activities
static void generate_walking_pattern(imu_data_t *data, uint32_t sample_index)
{
    float time = (float)sample_index / IMU_SAMPLE_RATE_HZ;
    float step_freq = 1.8f; // Steps per second
    
    // Simulate walking acceleration pattern
    data->accel_x = 0.5f * sinf(2.0f * M_PI * step_freq * time) + 0.2f * ((rand() % 100) / 100.0f - 0.5f);
    data->accel_y = 0.3f * cosf(2.0f * M_PI * step_freq * time) + 0.1f * ((rand() % 100) / 100.0f - 0.5f);
    data->accel_z = 9.81f + 0.8f * sinf(2.0f * M_PI * step_freq * time) + 0.1f * ((rand() % 100) / 100.0f - 0.5f);
    
    // Add some gyroscope data
    data->gyro_x = 0.1f * sinf(2.0f * M_PI * step_freq * time);
    data->gyro_y = 0.05f * cosf(2.0f * M_PI * step_freq * time);
    data->gyro_z = 0.02f * sinf(4.0f * M_PI * step_freq * time);
}

static void generate_running_pattern(imu_data_t *data, uint32_t sample_index)
{
    float time = (float)sample_index / IMU_SAMPLE_RATE_HZ;
    float step_freq = 2.5f; // Faster steps for running
    
    // Simulate running acceleration pattern (higher amplitude)
    data->accel_x = 1.2f * sinf(2.0f * M_PI * step_freq * time) + 0.3f * ((rand() % 100) / 100.0f - 0.5f);
    data->accel_y = 0.8f * cosf(2.0f * M_PI * step_freq * time) + 0.2f * ((rand() % 100) / 100.0f - 0.5f);
    data->accel_z = 9.81f + 2.0f * sinf(2.0f * M_PI * step_freq * time) + 0.2f * ((rand() % 100) / 100.0f - 0.5f);
    
    // More pronounced gyroscope movement
    data->gyro_x = 0.3f * sinf(2.0f * M_PI * step_freq * time);
    data->gyro_y = 0.2f * cosf(2.0f * M_PI * step_freq * time);
    data->gyro_z = 0.1f * sinf(4.0f * M_PI * step_freq * time);
}

static void generate_resting_pattern(imu_data_t *data, uint32_t sample_index)
{
    // Simulate minimal movement at rest
    data->accel_x = 0.05f * ((rand() % 100) / 100.0f - 0.5f);
    data->accel_y = 0.05f * ((rand() % 100) / 100.0f - 0.5f);
    data->accel_z = 9.81f + 0.05f * ((rand() % 100) / 100.0f - 0.5f);
    
    data->gyro_x = 0.01f * ((rand() % 100) / 100.0f - 0.5f);
    data->gyro_y = 0.01f * ((rand() % 100) / 100.0f - 0.5f);
    data->gyro_z = 0.01f * ((rand() % 100) / 100.0f - 0.5f);
}

static void simulate_imu_data(imu_data_t *data, uint32_t sample_index)
{
    data->timestamp = k_uptime_get();
    
    // Switch between different activity patterns during test
    uint32_t pattern_duration = IMU_SAMPLE_RATE_HZ * 10; // 10 seconds per pattern
    uint32_t pattern_index = (sample_index / pattern_duration) % 3;
    
    switch (pattern_index) {
        case 0:
            generate_resting_pattern(data, sample_index % pattern_duration);
            break;
        case 1:
            generate_walking_pattern(data, sample_index % pattern_duration);
            break;
        case 2:
            generate_running_pattern(data, sample_index % pattern_duration);
            break;
    }
}

static void test_timer_handler(struct k_timer *timer)
{
    if (!test_running) {
        return;
    }
    
    // Generate simulated IMU data
    imu_data_t imu_data;
    simulate_imu_data(&imu_data, test_samples_processed);
    
    // Process the data through our algorithms
    int ret = imu_algorithms_process(&g_imu_processor, &imu_data);
    if (ret != 0) {
        LOG_ERR("Failed to process IMU data: %d", ret);
        return;
    }
    
    test_samples_processed++;
    
    // Log activity info every 2 seconds
    if (test_samples_processed % (IMU_SAMPLE_RATE_HZ * 2) == 0) {
        activity_info_t activity_info;
        ret = imu_algorithms_get_activity_info(&g_imu_processor, &activity_info);
        if (ret == 0) {
            LOG_INF("Activity: %d, Steps: %u, Magnitude: %.2f, Pitch: %.1f°, Roll: %.1f°", 
                    activity_info.current_activity,
                    activity_info.daily_steps,
                    activity_info.current_magnitude,
                    activity_info.orientation.pitch,
                    activity_info.orientation.roll);
        }
    }
    
    // Stop test after specified duration
    if (test_samples_processed >= (IMU_TEST_DURATION_SEC * IMU_SAMPLE_RATE_HZ)) {
        test_running = false;
        LOG_INF("IMU test completed. Total samples: %u", test_samples_processed);
        
        // Final statistics
        activity_info_t final_info;
        if (imu_algorithms_get_activity_info(&g_imu_processor, &final_info) == 0) {
            LOG_INF("Final results:");
            LOG_INF("  Total steps detected: %u", final_info.daily_steps);
            LOG_INF("  Final activity: %d", final_info.current_activity);
            
            // Estimate calories for test duration
            float calories = imu_algorithms_estimate_calories(&g_imu_processor, 70.0f, IMU_TEST_DURATION_SEC / 60);
            LOG_INF("  Estimated calories: %.1f kcal", calories);
        }
    }
}

K_TIMER_DEFINE(test_timer, test_timer_handler, NULL);

static int run_imu_test(void)
{
    LOG_INF("Starting IMU algorithms test...");
    
    // Initialize IMU processor
    int ret = imu_algorithms_init(&g_imu_processor);
    if (ret != 0) {
        LOG_ERR("Failed to initialize IMU processor: %d", ret);
        return ret;
    }
    
    // Reset test variables
    test_running = true;
    test_samples_processed = 0;
    
    LOG_INF("Test will run for %d seconds at %d Hz", IMU_TEST_DURATION_SEC, IMU_SAMPLE_RATE_HZ);
    LOG_INF("Activity patterns: 0-10s: Rest, 10-20s: Walking, 20-30s: Running");
    
    // Start the test timer
    k_timer_start(&test_timer, K_MSEC(IMU_SAMPLE_INTERVAL_MS), K_MSEC(IMU_SAMPLE_INTERVAL_MS));
    
    // Wait for test to complete
    while (test_running) {
        k_sleep(K_MSEC(100));
    }
    
    k_timer_stop(&test_timer);
    
    LOG_INF("IMU algorithms test completed successfully");
    return 0;
}

int main(void)
{
    LOG_INF("IMU Test Application Started");
    LOG_INF("Testing step detection and activity classification algorithms");
    
    // Wait a moment for system to stabilize
    k_sleep(K_SECONDS(1));
    
    // Run the test
    int ret = run_imu_test();
    if (ret != 0) {
        LOG_ERR("IMU test failed with error: %d", ret);
        return ret;
    }
    
    LOG_INF("All tests completed. System idle.");
    
    // Keep the application running
    while (1) {
        k_sleep(K_SECONDS(10));
    }
    
    return 0;
}
