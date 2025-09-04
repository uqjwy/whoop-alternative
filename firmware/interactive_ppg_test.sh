#!/bin/bash
# PPG Simulator Interactive Test Script

echo "🔬 PPG Simulator Interactive Tests"
echo "=================================="

# Custom test with user input
read -p "Enter Heart Rate (40-200 bpm): " hr
read -p "Enter Noise Level (0.0-1.0): " noise
read -p "Enter Motion Artifacts (0.0-1.0): " motion
read -p "Sleep Mode? (y/n): " sleep_input
read -p "Test Duration (seconds): " duration

if [ "$sleep_input" = "y" ]; then
    sleep_mode=1
else
    sleep_mode=0
fi

# Create custom test file
cat > custom_ppg_test.c << EOF
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppg_simulator_host.h"

int main() {
    struct ppg_sim_config config = {
        .heart_rate_bpm = ${hr},
        .noise_level = ${noise},
        .motion_artifacts = ${motion},
        .sleep_mode = ${sleep_mode},
        .breathing_rate_bpm = 16.0f,
        .signal_quality = 95
    };
    
    ppg_sim_init(&config);
    
    printf("Custom PPG Test:\\n");
    printf("HR: %.1f bpm, Noise: %.2f, Motion: %.2f, Sleep: %s\\n",
           config.heart_rate_bpm, config.noise_level, config.motion_artifacts,
           config.sleep_mode ? "Yes" : "No");
    
    const int sample_rate = 50;
    const int total_samples = ${duration} * sample_rate;
    
    // Generate and display samples
    printf("\\nGenerating %d samples over %d seconds:\\n", total_samples, ${duration});
    
    for (int i = 0; i < total_samples; i++) {
        uint32_t timestamp_ms = i * (1000 / sample_rate);
        float sample = ppg_sim_generate_sample(timestamp_ms);
        
        // Print every 10th sample
        if (i % 10 == 0) {
            printf("Time: %6.2fs, PPG: %.4f\\n", timestamp_ms / 1000.0f, sample);
        }
    }
    
    return 0;
}
EOF

# Compile and run custom test
gcc -Itests custom_ppg_test.c -lm -o custom_ppg_test
echo ""
echo "Running custom test..."
./custom_ppg_test

# Cleanup
rm custom_ppg_test.c custom_ppg_test
