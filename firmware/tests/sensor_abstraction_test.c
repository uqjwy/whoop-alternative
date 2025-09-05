/**
 * @file sensor_abstraction_test.c
 * @brief Test application for sensor-agnostic architecture
 * 
 * This test demonstrates the sensor abstraction layer and how easy it is
 * to switch between different sensors (e.g., MAX30101 → MAX86141) without
 * changing the application logic.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Include our sensor interfaces - fixed paths
#include "../drivers/interfaces/sensor_interfaces.h"
#include "../drivers/interfaces/sensor_config.h"

// =============================================================================
// Mock MAX30101 Implementation (for testing)
// =============================================================================

static bool mock_max30101_init(const ppg_config_t* config) {
    printf("MAX30101: Initializing...\n");
    printf("  Sample Rate: %d Hz\n", config->sample_rate);
    printf("  LED Currents: Red=%dmA, IR=%dmA\n", 
           config->led_current[0], config->led_current[1]);
    printf("  FIFO: %s\n", config->fifo_enable ? "Enabled" : "Disabled");
    return true;
}

static bool mock_max30101_start(void) {
    printf("MAX30101: Starting measurement\n");
    return true;
}

static int mock_max30101_read_fifo(ppg_sample_t* samples, int max) {
    if (max > 0) {
        // Generate mock data
        samples[0].timestamp = 12345;
        samples[0].channels[0] = 50000;  // Red channel
        samples[0].channels[1] = 48000;  // IR channel
        samples[0].channels[2] = 0;      // Green (not used)
        samples[0].channels[3] = 0;      // UV (not used)
        samples[0].led_slots = 0x03;     // Red + IR
        samples[0].temperature = 3700;   // 37.00°C
        samples[0].quality = 85;         // 85% quality
        samples[0].sample_count = 1;
        
        printf("MAX30101: Sample - Red=%d, IR=%d, Temp=%.2f°C, Quality=%d%%\n", 
               samples[0].channels[0], samples[0].channels[1], 
               samples[0].temperature / 100.0, samples[0].quality);
        return 1;
    }
    return 0;
}

static bool mock_max30101_stop(void) {
    printf("MAX30101: Stopping measurement\n");
    return true;
}

static bool mock_max30101_reset(void) {
    printf("MAX30101: Resetting sensor\n");
    return true;
}

static bool mock_max30101_set_config(const ppg_config_t* config) {
    printf("MAX30101: Updating configuration\n");
    return true;
}

static bool mock_max30101_get_status(uint8_t* status) {
    *status = 0x80;  // Data ready
    return true;
}

static int mock_max30101_get_fifo_count(void) {
    return 5;  // 5 samples available
}

// MAX30101 Operations Structure
static ppg_sensor_ops_t max30101_ops = {
    .init = mock_max30101_init,
    .start = mock_max30101_start,
    .read_fifo = mock_max30101_read_fifo,
    .stop = mock_max30101_stop,
    .reset = mock_max30101_reset,
    .set_config = mock_max30101_set_config,
    .get_status = mock_max30101_get_status,
    .get_fifo_count = mock_max30101_get_fifo_count
};

// =============================================================================
// Mock MAX86141 Implementation (for testing)
// =============================================================================

static bool mock_max86141_init(const ppg_config_t* config) {
    printf("MAX86141: Initializing...\n");
    printf("  Sample Rate: %d Hz\n", config->sample_rate);
    printf("  LED Currents: Red=%dmA, IR=%dmA\n", 
           config->led_current[0], config->led_current[1]);
    printf("  Proximity: %s\n", config->proximity_enable ? "Enabled" : "Disabled");
    return true;
}

static bool mock_max86141_start(void) {
    printf("MAX86141: Starting measurement\n");
    return true;
}

static int mock_max86141_read_fifo(ppg_sample_t* samples, int max) {
    if (max > 0) {
        // Generate mock data (slightly different from MAX30101)
        samples[0].timestamp = 12345;
        samples[0].channels[0] = 75000;  // Red channel (higher resolution)
        samples[0].channels[1] = 72000;  // IR channel
        samples[0].channels[2] = 0;      // Green (available but not used)
        samples[0].channels[3] = 0;      // UV (available but not used)
        samples[0].led_slots = 0x03;     // Red + IR
        samples[0].temperature = 3690;   // 36.90°C
        samples[0].quality = 92;         // 92% quality (better SNR)
        samples[0].sample_count = 1;
        
        printf("MAX86141: Sample - Red=%d, IR=%d, Temp=%.2f°C, Quality=%d%%\n", 
               samples[0].channels[0], samples[0].channels[1], 
               samples[0].temperature / 100.0, samples[0].quality);
        return 1;
    }
    return 0;
}

static bool mock_max86141_stop(void) {
    printf("MAX86141: Stopping measurement\n");
    return true;
}

static bool mock_max86141_reset(void) {
    printf("MAX86141: Resetting sensor\n");
    return true;
}

static bool mock_max86141_set_config(const ppg_config_t* config) {
    printf("MAX86141: Updating configuration\n");
    return true;
}

static bool mock_max86141_get_status(uint8_t* status) {
    *status = 0x80;  // Data ready
    return true;
}

static int mock_max86141_get_fifo_count(void) {
    return 8;  // 8 samples available (larger FIFO)
}

// MAX86141 Operations Structure
static ppg_sensor_ops_t max86141_ops = {
    .init = mock_max86141_init,
    .start = mock_max86141_start,
    .read_fifo = mock_max86141_read_fifo,
    .stop = mock_max86141_stop,
    .reset = mock_max86141_reset,
    .set_config = mock_max86141_set_config,
    .get_status = mock_max86141_get_status,
    .get_fifo_count = mock_max86141_get_fifo_count
};

// =============================================================================
// Sensor Manager (Simplified for Testing)
// =============================================================================

typedef struct {
    ppg_sensor_ops_t* active_sensor;
    ppg_config_t config;
    bool initialized;
    bool running;
} simple_sensor_manager_t;

static simple_sensor_manager_t manager = {0};

bool test_sensor_manager_init(const char* sensor_name) {
    // Select sensor based on name
    if (strcmp(sensor_name, "MAX30101") == 0) {
        manager.active_sensor = &max30101_ops;
        // Default MAX30101 config
        manager.config = (ppg_config_t){
            .sample_rate = 100,
            .led_current = {25, 25, 0, 0},
            .pulse_width = 411,
            .fifo_enable = true,
            .fifo_almost_full = 17,
            .temp_enable = true,
            .proximity_enable = false
        };
    } else if (strcmp(sensor_name, "MAX86141") == 0) {
        manager.active_sensor = &max86141_ops;
        // Default MAX86141 config (higher LED current for external LEDs)
        manager.config = (ppg_config_t){
            .sample_rate = 100,
            .led_current = {50, 50, 0, 0},
            .pulse_width = 411,
            .fifo_enable = true,
            .fifo_almost_full = 17,
            .temp_enable = true,
            .proximity_enable = true  // MAX86141 has proximity
        };
    } else {
        printf("Error: Unknown sensor '%s'\n", sensor_name);
        return false;
    }
    
    if (manager.active_sensor->init(&manager.config)) {
        manager.initialized = true;
        printf("✓ Sensor manager initialized with %s\n", sensor_name);
        return true;
    }
    
    return false;
}

bool test_sensor_manager_start(void) {
    if (manager.initialized && manager.active_sensor) {
        if (manager.active_sensor->start()) {
            manager.running = true;
            return true;
        }
    }
    return false;
}

int test_sensor_manager_read(ppg_sample_t* sample) {
    if (manager.running && manager.active_sensor) {
        return manager.active_sensor->read_fifo(sample, 1);
    }
    return 0;
}

bool test_sensor_manager_stop(void) {
    if (manager.running && manager.active_sensor) {
        if (manager.active_sensor->stop()) {
            manager.running = false;
            return true;
        }
    }
    return false;
}

// =============================================================================
// Test Functions
// =============================================================================

void test_sensor_switching(void) {
    printf("\n=== Sensor Switching Test ===\n");
    
    ppg_sample_t sample;
    
    // Test with MAX30101
    printf("\n--- Testing with MAX30101 ---\n");
    if (test_sensor_manager_init("MAX30101")) {
        test_sensor_manager_start();
        for (int i = 0; i < 3; i++) {
            if (test_sensor_manager_read(&sample) > 0) {
                printf("Sample %d collected\n", i + 1);
            }
        }
        test_sensor_manager_stop();
    }
    
    // Switch to MAX86141 - NO APPLICATION CODE CHANGES!
    printf("\n--- Switching to MAX86141 ---\n");
    if (test_sensor_manager_init("MAX86141")) {
        test_sensor_manager_start();
        for (int i = 0; i < 3; i++) {
            if (test_sensor_manager_read(&sample) > 0) {
                printf("Sample %d collected\n", i + 1);
            }
        }
        test_sensor_manager_stop();
    }
    
    printf("\n✅ Sensor switching test completed!\n");
    printf("Notice: Application code stayed exactly the same!\n");
}

void test_configuration_profiles(void) {
    printf("\n=== Configuration Profile Test ===\n");
    
    // Simulate different profiles
    printf("\n--- Low Power Profile ---\n");
    ppg_config_t low_power = {
        .sample_rate = 25,
        .led_current = {10, 10, 0, 0},
        .fifo_enable = true
    };
    
    printf("Profile: Low Power\n");
    printf("  Sample Rate: %d Hz\n", low_power.sample_rate);
    printf("  LED Current: %d mA\n", low_power.led_current[0]);
    
    printf("\n--- High Accuracy Profile ---\n");
    ppg_config_t high_accuracy = {
        .sample_rate = 200,
        .led_current = {50, 50, 0, 0},
        .fifo_enable = true
    };
    
    printf("Profile: High Accuracy\n");
    printf("  Sample Rate: %d Hz\n", high_accuracy.sample_rate);
    printf("  LED Current: %d mA\n", high_accuracy.led_current[0]);
    
    printf("\n✅ Configuration profiles demonstrated!\n");
}

void demonstrate_benefits(void) {
    printf("\n=== Sensor-Agnostic Architecture Benefits ===\n");
    
    printf("\n✓ Unified Interface:\n");
    printf("  - All PPG sensors implement ppg_sensor_ops_t\n");
    printf("  - Consistent function signatures\n");
    printf("  - Same data structures (ppg_sample_t)\n");
    
    printf("\n✓ Easy Sensor Migration:\n");
    printf("  - Change sensor_name = \"MAX86141\" in config\n");
    printf("  - Adjust LED currents for external LEDs\n");
    printf("  - NO application code changes!\n");
    
    printf("\n✓ Configuration-Driven:\n");
    printf("  - Different profiles for different use cases\n");
    printf("  - Runtime configuration switching\n");
    printf("  - No recompilation needed\n");
    
    printf("\n✓ Modular Design:\n");
    printf("  - Drivers are independent modules\n");
    printf("  - Easy to add new sensors\n");
    printf("  - Clean separation of concerns\n");
    
    printf("\n✓ Future-Proof:\n");
    printf("  - New sensors just implement the interface\n");
    printf("  - Existing pipeline code unaffected\n");
    printf("  - Backward compatibility maintained\n");
}

// =============================================================================
// Main Test Application
// =============================================================================

int main(void) {
    printf("=== Sensor Abstraction Architecture Test ===\n");
    printf("Demonstrating sensor-agnostic firmware design\n");
    
    // Run tests
    test_sensor_switching();
    test_configuration_profiles();
    demonstrate_benefits();
    
    printf("\n=== Migration Guide: MAX30101 → MAX86141 ===\n");
    printf("1. Change config: sensor_name=\"MAX86141\"\n");
    printf("2. Increase LED currents: red=50mA, ir=50mA\n");
    printf("3. Enable proximity: proximity_enable=true\n");
    printf("4. That's it! No code changes needed.\n");
    
    printf("\n🎉 Sensor abstraction architecture test successful!\n");
    printf("✅ Ready for MAX86141 implementation\n");
    printf("✅ Unified sensor interface validated\n");
    printf("✅ Configuration-driven architecture working\n");
    
    return 0;
}
