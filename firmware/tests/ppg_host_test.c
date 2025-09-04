/*
 * PPG Simulator Host Test
 * 
 * Simplified version for testing on host system without Zephyr
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

// Mock Zephyr types
typedef struct {
    int dummy;
} k_timeout_t;

#define K_MSEC(ms) ((k_timeout_t){ms})
#define LOG_INF printf
#define LOG_ERR printf
#define printk printf

// Include PPG simulator (we'll create a host-compatible version)
#include "ppg_simulator_host.h"

/* Test scenarios */
static struct test_scenario {
    const char *name;
    float heart_rate;
    float noise_level;
    float motion_artifacts;
    int sleep_mode;
    int duration_seconds;
} test_scenarios[] = {
    {"Rest - Clean Signal", 70.0f, 0.05f, 0.0f, 0, 10},
    {"Rest - Noisy Signal", 70.0f, 0.2f, 0.0f, 0, 10},
    {"Exercise - Light Motion", 120.0f, 0.1f, 0.3f, 0, 10},
    {"Exercise - Heavy Motion", 140.0f, 0.15f, 0.6f, 0, 10},
    {"Sleep - Clean", 55.0f, 0.03f, 0.0f, 1, 10},
    {"Sleep - Movement", 60.0f, 0.1f, 0.2f, 1, 10},
};

static void test_scenario(struct test_scenario *scenario);
static void print_statistics(float *samples, int count, const char *scenario_name);

int main(void)
{
    printf("=== PPG Simulator Host Test ===\n\n");
    
    // Initialize random seed
    srand(time(NULL));
    
    int num_scenarios = sizeof(test_scenarios) / sizeof(test_scenarios[0]);
    
    for (int i = 0; i < num_scenarios; i++) {
        printf("--- Test %d: %s ---\n", i + 1, test_scenarios[i].name);
        test_scenario(&test_scenarios[i]);
        printf("\n");
    }
    
    printf("=== All PPG Tests Completed ===\n");
    return 0;
}

static void test_scenario(struct test_scenario *scenario)
{
    const int sample_rate_hz = 50;
    const int total_samples = scenario->duration_seconds * sample_rate_hz;
    const int interval_ms = 1000 / sample_rate_hz;
    
    float *samples = malloc(total_samples * sizeof(float));
    if (!samples) {
        printf("Error: Could not allocate memory for samples\n");
        return;
    }
    
    // Configure PPG simulator
    struct ppg_sim_config config = {
        .heart_rate_bpm = scenario->heart_rate,
        .noise_level = scenario->noise_level,
        .motion_artifacts = scenario->motion_artifacts,
        .sleep_mode = scenario->sleep_mode,
        .breathing_rate_bpm = 16.0f,
        .signal_quality = scenario->sleep_mode ? 80 : 95
    };
    
    ppg_sim_init(&config);
    
    printf("  Configuration:\n");
    printf("    Heart Rate: %.1f bpm\n", config.heart_rate_bpm);
    printf("    Noise Level: %.2f\n", config.noise_level);
    printf("    Motion Artifacts: %.2f\n", config.motion_artifacts);
    printf("    Sleep Mode: %s\n", config.sleep_mode ? "Yes" : "No");
    printf("    Duration: %d seconds (%d samples)\n", 
           scenario->duration_seconds, total_samples);
    
    // Generate samples
    printf("  Generating samples");
    fflush(stdout);
    
    for (int i = 0; i < total_samples; i++) {
        uint32_t timestamp_ms = i * interval_ms;
        samples[i] = ppg_sim_generate_sample(timestamp_ms);
        
        // Progress indicator
        if (i % (total_samples / 10) == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf(" Done!\n");
    
    // Analyze and print statistics
    print_statistics(samples, total_samples, scenario->name);
    
    // Save first 100 samples to see waveform
    printf("  First 10 samples:\n    ");
    for (int i = 0; i < 10 && i < total_samples; i++) {
        printf("%.3f ", samples[i]);
    }
    printf("\n");
    
    free(samples);
}

static void print_statistics(float *samples, int count, const char *scenario_name)
{
    if (count == 0) return;
    
    // Calculate basic statistics
    float min_val = samples[0];
    float max_val = samples[0];
    float sum = 0.0f;
    
    for (int i = 0; i < count; i++) {
        if (samples[i] < min_val) min_val = samples[i];
        if (samples[i] > max_val) max_val = samples[i];
        sum += samples[i];
    }
    
    float mean = sum / count;
    
    // Calculate standard deviation
    float variance_sum = 0.0f;
    for (int i = 0; i < count; i++) {
        float diff = samples[i] - mean;
        variance_sum += diff * diff;
    }
    float std_dev = sqrt(variance_sum / count);
    
    printf("  Statistics:\n");
    printf("    Mean: %.4f\n", mean);
    printf("    Min: %.4f, Max: %.4f\n", min_val, max_val);
    printf("    Std Dev: %.4f\n", std_dev);
    printf("    Range: %.4f\n", max_val - min_val);
    
    // Simple heart rate estimation (count peaks)
    int peak_count = 0;
    for (int i = 1; i < count - 1; i++) {
        if (samples[i] > samples[i-1] && samples[i] > samples[i+1] && 
            samples[i] > mean + std_dev) {
            peak_count++;
        }
    }
    
    float duration_minutes = count / (50.0f * 60.0f); // 50 Hz sample rate
    float estimated_hr = peak_count / duration_minutes;
    
    printf("    Detected Peaks: %d\n", peak_count);
    printf("    Estimated HR: %.1f bpm\n", estimated_hr);
}
