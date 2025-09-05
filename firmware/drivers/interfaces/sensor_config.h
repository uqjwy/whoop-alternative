#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "sensor_interfaces.h"

/**
 * @file sensor_config.h
 * @brief Configuration-driven sensor management
 * 
 * This enables easy sensor switching through configuration files
 * without code changes. Simply change sensor_type = "MAX86141" 
 * and adjust parameters to migrate.
 */

// =============================================================================
// Configuration Profile Types
// =============================================================================

typedef enum {
    PROFILE_DEFAULT = 0,       ///< Balanced performance
    PROFILE_LOW_POWER,         ///< Battery optimization
    PROFILE_HIGH_ACCURACY,     ///< Maximum signal quality
    PROFILE_EXERCISE,          ///< Activity tracking optimized
    PROFILE_SLEEP,             ///< Sleep monitoring optimized
    PROFILE_CUSTOM,            ///< User-defined settings
    PROFILE_COUNT
} config_profile_t;

typedef enum {
    SENSOR_PRIORITY_POWER = 0, ///< Optimize for low power
    SENSOR_PRIORITY_ACCURACY,  ///< Optimize for accuracy
    SENSOR_PRIORITY_SPEED,     ///< Optimize for fast response
} sensor_priority_t;

// =============================================================================
// System Configuration
// =============================================================================

typedef struct {
    // Sensor selection (key for migration!)
    char ppg_sensor[32];       ///< "MAX30101", "MAX86141", "MAXM86146"
    char imu_sensor[32];       ///< "BMA400", "BMI270", "ICM42688"
    
    // Profile and priority
    config_profile_t profile;
    sensor_priority_t priority;
    
    // System settings
    bool debug_enable;
    int log_level;             ///< 0=Error, 1=Warn, 2=Info, 3=Debug, 4=Verbose
    bool data_logging;         ///< Enable raw data logging for replay
    char log_filename[64];     ///< Log file path
    
    // Timing and sync
    uint32_t base_sample_rate; ///< Base system sample rate
    bool timestamp_sync;       ///< Enable PPG/IMU timestamp synchronization
    int32_t clock_offset_ms;   ///< Clock offset compensation
} system_config_t;

// =============================================================================
// Complete Sensor Configuration
// =============================================================================

typedef struct {
    system_config_t system;
    ppg_config_t ppg;
    imu_config_t imu;
    
    // Calibration data (sensor-specific)
    float spo2_calibration[4]; ///< SpO2 calibration constants [a, b, c, d]
    float temp_offset;         ///< Temperature offset correction
    float accel_bias[3];       ///< Accelerometer bias correction
    float gyro_bias[3];        ///< Gyroscope bias correction
} sensor_config_t;

// =============================================================================
// Predefined Configurations
// =============================================================================

// MAX30101 Default Configuration
static const ppg_config_t MAX30101_DEFAULT = {
    .sample_rate = 100,
    .led_current = {25, 25, 0, 0},     // Red, IR, Green, UV
    .slot_map = {0, 1, 2, 3},          // Direct mapping
    .pulse_width = 411,                // 411μs
    .adc_range = 4096,                 // 15-bit ADC range
    .avg_samples = 1,                  // No averaging
    .fifo_enable = true,
    .fifo_almost_full = 17,            // Interrupt when 17 samples
    .temp_enable = true,
    .proximity_enable = false,         // Not available on MAX30101
    .tia_gain = 0,                     // Not used for integrated sensor
    .integrator_gain = 0
};

// MAX86141 Default Configuration (AFE with external optics)
static const ppg_config_t MAX86141_DEFAULT = {
    .sample_rate = 100,
    .led_current = {50, 50, 0, 0},     // Higher current for external LEDs
    .slot_map = {0, 1, 2, 3},          // Configurable slot mapping
    .pulse_width = 411,                // 411μs
    .adc_range = 262144,               // 19-bit ADC range
    .avg_samples = 1,                  // No averaging
    .fifo_enable = true,
    .fifo_almost_full = 17,            // Interrupt when 17 samples
    .temp_enable = true,
    .proximity_enable = true,          // Available on MAX86141
    .tia_gain = 50000,                 // 50kΩ TIA gain
    .integrator_gain = 1               // 1x integrator gain
};

// BMA400 Default Configuration
static const imu_config_t BMA400_DEFAULT = {
    .accel_range = 4,                  // ±4g
    .accel_bandwidth = 100,            // 100Hz bandwidth
    .gyro_range = 0,                   // No gyro on BMA400
    .gyro_bandwidth = 0,
    .sample_rate = 100,                // 100Hz
    .interrupt_enable = false,
    .interrupt_threshold = 128         // Motion threshold
};

// BMI270 Default Configuration
static const imu_config_t BMI270_DEFAULT = {
    .accel_range = 4,                  // ±4g
    .accel_bandwidth = 100,            // 100Hz bandwidth
    .gyro_range = 500,                 // ±500dps
    .gyro_bandwidth = 100,             // 100Hz bandwidth
    .sample_rate = 100,                // 100Hz
    .interrupt_enable = false,
    .interrupt_threshold = 128         // Motion threshold
};

// =============================================================================
// Profile-Specific Configurations
// =============================================================================

static const struct {
    config_profile_t profile;
    ppg_config_t ppg;
    imu_config_t imu;
} PROFILE_CONFIGS[] = {
    // Low Power Profile
    {
        .profile = PROFILE_LOW_POWER,
        .ppg = {
            .sample_rate = 25,
            .led_current = {10, 10, 0, 0},
            .avg_samples = 4,           // More averaging for lower noise
            .fifo_almost_full = 8       // Smaller FIFO for lower latency
        },
        .imu = {
            .accel_range = 2,           // Lower range for better resolution
            .sample_rate = 25,
            .interrupt_enable = true    // Use interrupts to save power
        }
    },
    
    // High Accuracy Profile
    {
        .profile = PROFILE_HIGH_ACCURACY,
        .ppg = {
            .sample_rate = 200,
            .led_current = {50, 50, 25, 0},  // Use Green channel too
            .avg_samples = 1,                // No averaging for best time resolution
            .fifo_almost_full = 32           // Larger FIFO for batch processing
        },
        .imu = {
            .accel_range = 4,
            .gyro_range = 500,
            .sample_rate = 200,
            .interrupt_enable = false        // Continuous sampling
        }
    },
    
    // Exercise Profile
    {
        .profile = PROFILE_EXERCISE,
        .ppg = {
            .sample_rate = 100,
            .led_current = {30, 30, 15, 0},  // Green for motion artifacts
            .avg_samples = 2,                // Light averaging
            .proximity_enable = true         // Detect sensor contact
        },
        .imu = {
            .accel_range = 8,                // Higher range for vigorous movement
            .gyro_range = 1000,
            .sample_rate = 100,
            .interrupt_enable = true,        // Motion detection
            .interrupt_threshold = 64        // Lower threshold for activity
        }
    }
};

// =============================================================================
// Configuration Management Functions
// =============================================================================

/**
 * @brief Load configuration from file
 */
bool config_load_from_file(const char* filename, sensor_config_t* config);

/**
 * @brief Save configuration to file
 */
bool config_save_to_file(const char* filename, const sensor_config_t* config);

/**
 * @brief Get default configuration for sensor
 */
bool config_get_default(const char* ppg_sensor, const char* imu_sensor, sensor_config_t* config);

/**
 * @brief Apply profile to configuration
 */
bool config_apply_profile(config_profile_t profile, sensor_config_t* config);

/**
 * @brief Validate configuration parameters
 */
bool config_validate(const sensor_config_t* config);

/**
 * @brief Get sensor-specific calibration
 */
bool config_get_calibration(const char* sensor_name, float* calibration_data);

/**
 * @brief Update calibration data
 */
bool config_set_calibration(const char* sensor_name, const float* calibration_data);

// =============================================================================
// Migration Helper Functions
// =============================================================================

/**
 * @brief Check if sensor migration is needed
 * @param current Current sensor configuration
 * @param target Target sensor name
 * @return true if migration is needed
 */
bool config_needs_migration(const sensor_config_t* current, const char* target);

/**
 * @brief Migrate configuration to new sensor
 * @param config Configuration to migrate
 * @param new_sensor Target sensor name
 * @return true if migration successful
 */
bool config_migrate_sensor(sensor_config_t* config, const char* new_sensor);

/**
 * @brief Get migration recommendations
 * @param from_sensor Source sensor
 * @param to_sensor Target sensor
 * @param recommendations Buffer for recommendations text
 * @param max_len Maximum length of recommendations
 */
void config_get_migration_recommendations(const char* from_sensor, 
                                        const char* to_sensor,
                                        char* recommendations, 
                                        int max_len);

#endif // SENSOR_CONFIG_H
