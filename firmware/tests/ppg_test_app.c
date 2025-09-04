/*
 * PPG Pipeline Test Application
 * 
 * Tests PPG signal processing pipeline with simulated data
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "ppg_simulator.h"
#include "ppg_pipeline.h"

LOG_MODULE_REGISTER(ppg_test, LOG_LEVEL_INF);

/* Test configuration */
#define TEST_DURATION_S         60      /* Test duration in seconds */
#define TEST_SAMPLE_RATE_HZ     50      /* Sample rate */
#define TEST_INTERVAL_MS        (1000 / TEST_SAMPLE_RATE_HZ)

/* Test scenarios */
static struct test_scenario {
    const char *name;
    float heart_rate;
    float noise_level;
    float motion_artifacts;
    bool sleep_mode;
} test_scenarios[] = {
    {"Rest - Clean Signal", 70.0f, 0.05f, 0.0f, false},
    {"Rest - Noisy Signal", 70.0f, 0.2f, 0.0f, false},
    {"Exercise - Light Motion", 120.0f, 0.1f, 0.3f, false},
    {"Exercise - Heavy Motion", 140.0f, 0.15f, 0.6f, false},
    {"Sleep - Clean", 55.0f, 0.03f, 0.0f, true},
    {"Sleep - Movement", 60.0f, 0.1f, 0.2f, true},
};

static struct ppg_pipeline pipeline;
static struct ppg_sim_config sim_config;

static void test_scenario(struct test_scenario *scenario);
static void print_test_results(struct test_scenario *scenario, 
                              struct ppg_hr_result *result, 
                              uint32_t duration_ms);

void main(void)
{
    LOG_INF("PPG Pipeline Test Application Started");
    
    /* Initialize PPG pipeline */
    int ret = ppg_pipeline_init(&pipeline);
    if (ret != 0) {
        LOG_ERR("Failed to initialize PPG pipeline: %d", ret);
        return;
    }
    
    /* Initialize PPG simulator */
    ret = ppg_sim_init(NULL);
    if (ret != 0) {
        LOG_ERR("Failed to initialize PPG simulator: %d", ret);
        return;
    }
    
    LOG_INF("Starting PPG algorithm validation tests...");
    LOG_INF("Each test runs for %d seconds at %d Hz", TEST_DURATION_S, TEST_SAMPLE_RATE_HZ);
    LOG_INF("=====================================");
    
    /* Run all test scenarios */
    for (int i = 0; i < ARRAY_SIZE(test_scenarios); i++) {
        LOG_INF("Test %d: %s", i + 1, test_scenarios[i].name);
        test_scenario(&test_scenarios[i]);
        k_sleep(K_MSEC(1000));  /* Pause between tests */
    }
    
    LOG_INF("=====================================");
    LOG_INF("All PPG tests completed successfully!");
}

static void test_scenario(struct test_scenario *scenario)
{
    /* Configure simulator for this scenario */
    ppg_sim_set_heart_rate(scenario->heart_rate);
    ppg_sim_set_noise_level(scenario->noise_level);
    ppg_sim_set_motion_artifacts(scenario->motion_artifacts);
    ppg_sim_set_sleep_mode(scenario->sleep_mode);
    
    /* Reset pipeline for clean test */
    ppg_pipeline_init(&pipeline);
    
    uint32_t start_time = k_uptime_get_32();
    uint32_t samples_processed = 0;
    uint32_t last_sample_time = start_time;
    
    /* Run test for specified duration */
    while ((k_uptime_get_32() - start_time) < (TEST_DURATION_S * 1000)) {
        uint32_t current_time = k_uptime_get_32();
        
        /* Generate sample at correct rate */
        if ((current_time - last_sample_time) >= TEST_INTERVAL_MS) {
            /* Generate PPG sample */
            float ppg_value = ppg_sim_generate_sample(current_time);
            uint32_t raw_value = (uint32_t)(ppg_value * 65535.0f);  /* Convert to 16-bit */
            uint8_t quality = ppg_sim_get_signal_quality();
            
            /* Process sample through pipeline */
            int ret = ppg_pipeline_add_sample(&pipeline, raw_value, current_time, quality);
            if (ret != 0) {
                LOG_WRN("Failed to add sample to pipeline: %d", ret);
            }
            
            samples_processed++;
            last_sample_time = current_time;
        }
        
        /* Calculate heart rate every 5 seconds */
        if ((samples_processed % (TEST_SAMPLE_RATE_HZ * 5)) == 0 && samples_processed > 0) {
            struct ppg_hr_result result;
            int ret = ppg_pipeline_get_heart_rate(&pipeline, &result);
            
            if (ret == 0 && result.hr_bpm > 0) {
                LOG_INF("  Interim HR: %d bpm (confidence: %d%%, RR count: %d)", 
                       result.hr_bpm, result.confidence, result.rr_count);
            }
        }
        
        k_sleep(K_MSEC(1));  /* Small delay to prevent busy waiting */
    }
    
    /* Get final heart rate result */
    struct ppg_hr_result final_result;
    int ret = ppg_pipeline_get_heart_rate(&pipeline, &final_result);
    
    uint32_t test_duration = k_uptime_get_32() - start_time;
    
    /* Print detailed test results */
    print_test_results(scenario, (ret == 0) ? &final_result : NULL, test_duration);
    
    LOG_INF("  Samples processed: %d", samples_processed);
    LOG_INF("  Actual sample rate: %.1f Hz", 
           (float)samples_processed * 1000.0f / test_duration);
}

static void print_test_results(struct test_scenario *scenario, 
                              struct ppg_hr_result *result, 
                              uint32_t duration_ms)
{
    LOG_INF("  Expected HR: %.1f bpm", scenario->heart_rate);
    
    if (result && result->hr_bpm > 0) {
        float error = fabsf((float)result->hr_bpm - scenario->heart_rate);
        float error_percent = (error / scenario->heart_rate) * 100.0f;
        
        LOG_INF("  Detected HR: %d bpm", result->hr_bpm);
        LOG_INF("  Error: %.1f bpm (%.1f%%)", error, error_percent);
        LOG_INF("  Confidence: %d%%", result->confidence);
        LOG_INF("  RR intervals: %d", result->rr_count);
        
        /* Print quality assessment */
        if (error_percent < 5.0f) {
            LOG_INF("  Quality: EXCELLENT");
        } else if (error_percent < 10.0f) {
            LOG_INF("  Quality: GOOD");
        } else if (error_percent < 20.0f) {
            LOG_INF("  Quality: ACCEPTABLE");
        } else {
            LOG_INF("  Quality: POOR");
        }
        
        /* Show some RR intervals */
        if (result->rr_count > 0) {
            LOG_INF("  Recent RR intervals (ms): ");
            for (int i = 0; i < MIN(5, result->rr_count); i++) {
                printk("%d ", result->rr_intervals[i]);
            }
            printk("\n");
        }
    } else {
        LOG_WRN("  Failed to detect heart rate");
        LOG_INF("  Quality: FAILED");
    }
    
    /* Get signal quality */
    uint8_t signal_quality = ppg_pipeline_get_signal_quality(&pipeline);
    LOG_INF("  Signal quality: %d%%", signal_quality);
    
    LOG_INF("  Test duration: %.1f seconds", duration_ms / 1000.0f);
    LOG_INF("  --------------------------------");
}
