#ifndef SENSOR_INTERFACES_H
#define SENSOR_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file sensor_interfaces.h
 * @brief Sensor-agnostic interfaces for PPG and IMU sensors
 * 
 * This defines unified interfaces that enable easy migration between
 * different sensor types (e.g., MAX30101 → MAX86141) without changing
 * the signal processing pipeline.
 */

// =============================================================================
// PPG Interface - Sensor-Agnostic
// =============================================================================

/**
 * @brief PPG Configuration Structure
 * Unified config that works for integrated sensors (MAX30101) and AFEs (MAX86141)
 */
typedef struct {
    int sample_rate;           ///< Sample rate in Hz (25, 50, 100, 200, 400, 800, 1000)
    int led_current[4];        ///< LED currents in mA [Red, IR, Green, UV/Ambient]
    int slot_map[4];           ///< Logical to physical slot mapping
    int pulse_width;           ///< LED pulse width in μs
    int adc_range;             ///< ADC range/gain setting
    int avg_samples;           ///< Number of samples to average (1, 2, 4, 8, 16, 32)
    
    // FIFO settings
    bool fifo_enable;          ///< Enable FIFO mode
    int fifo_almost_full;      ///< FIFO almost full threshold
    
    // Additional features
    bool temp_enable;          ///< Enable temperature measurement
    bool proximity_enable;     ///< Enable proximity detection (AFE specific)
    
    // Optical settings (AFE specific)
    int tia_gain;              ///< TIA gain for AFE sensors
    int integrator_gain;       ///< Integrator gain
} ppg_config_t;

/**
 * @brief PPG Sample Structure
 * Normalized data format for all PPG sensors
 */
typedef struct {
    uint32_t timestamp;        ///< System timestamp (ms since boot)
    int32_t channels[4];       ///< Raw counts [Red, IR, Green, UV] normalized to int32
    uint8_t led_slots;         ///< Active LED slots bitmask
    int16_t temperature;       ///< Temperature in 0.01°C units (3700 = 37.00°C)
    uint8_t quality;           ///< Signal quality indicator (0-100%)
    uint8_t sample_count;      ///< Number of samples in this packet
    uint16_t sequence;         ///< Sequence number for lost packet detection
} ppg_sample_t;

/**
 * @brief PPG Sensor Operations Interface
 * Pure C interface for sensor abstraction (equivalent to IPpgSensor)
 */
typedef struct {
    bool (*init)(const ppg_config_t* config);
    bool (*start)(void);
    int  (*read_fifo)(ppg_sample_t* samples, int max_samples);
    bool (*stop)(void);
    bool (*reset)(void);
    bool (*set_config)(const ppg_config_t* config);
    bool (*get_status)(uint8_t* status);
    int  (*get_fifo_count)(void);
} ppg_sensor_ops_t;

/**
 * @brief PPG Sensor Descriptor
 */
typedef struct {
    const char* name;              ///< Sensor name (e.g., "MAX30101", "MAX86141")
    const char* manufacturer;      ///< Manufacturer
    uint16_t device_id;            ///< Device ID for identification
    ppg_sensor_ops_t* ops;         ///< Function pointers
    const ppg_config_t* default_config;  ///< Default configuration
} ppg_sensor_desc_t;

// =============================================================================
// IMU Interface - Sensor-Agnostic
// =============================================================================

/**
 * @brief IMU Configuration Structure
 */
typedef struct {
    int accel_range;           ///< Accelerometer range (±2g, ±4g, ±8g, ±16g)
    int accel_bandwidth;       ///< Accelerometer bandwidth in Hz
    int gyro_range;            ///< Gyroscope range (±125, ±250, ±500, ±1000, ±2000 dps)
    int gyro_bandwidth;        ///< Gyroscope bandwidth in Hz
    int sample_rate;           ///< Sample rate in Hz
    bool interrupt_enable;     ///< Enable motion interrupts
    int interrupt_threshold;   ///< Motion threshold for interrupts
} imu_config_t;

/**
 * @brief IMU Sample Structure
 */
typedef struct {
    uint32_t timestamp;        ///< System timestamp (ms since boot)
    int16_t accel[3];          ///< Accelerometer [X, Y, Z] in mg
    int16_t gyro[3];           ///< Gyroscope [X, Y, Z] in mdps
    int16_t temperature;       ///< Temperature in 0.01°C units
    uint8_t sample_count;      ///< Number of samples in this packet
    uint16_t sequence;         ///< Sequence number
} imu_sample_t;

/**
 * @brief IMU Sensor Operations Interface
 * Pure C interface (equivalent to IIMU)
 */
typedef struct {
    bool (*init)(const imu_config_t* config);
    bool (*start)(void);
    int  (*read_fifo)(imu_sample_t* samples, int max_samples);
    bool (*stop)(void);
    bool (*reset)(void);
    bool (*set_config)(const imu_config_t* config);
    bool (*get_status)(uint8_t* status);
    int  (*get_fifo_count)(void);
} imu_sensor_ops_t;

/**
 * @brief IMU Sensor Descriptor
 */
typedef struct {
    const char* name;              ///< Sensor name (e.g., "BMA400", "BMI270")
    const char* manufacturer;      ///< Manufacturer
    uint16_t device_id;            ///< Device ID
    imu_sensor_ops_t* ops;         ///< Function pointers
    const imu_config_t* default_config;  ///< Default configuration
} imu_sensor_desc_t;

// =============================================================================
// Unified Sensor Descriptor (for registration)
// =============================================================================

typedef enum {
    SENSOR_TYPE_PPG = 0,
    SENSOR_TYPE_IMU,
    SENSOR_TYPE_COUNT
} sensor_type_t;

typedef struct {
    sensor_type_t type;
    union {
        ppg_sensor_desc_t ppg;
        imu_sensor_desc_t imu;
    } desc;
} sensor_descriptor_t;

// =============================================================================
// Data Logger Interface (for Replay capability)
// =============================================================================

typedef struct {
    uint32_t timestamp;
    sensor_type_t type;
    union {
        ppg_sample_t ppg;
        imu_sample_t imu;
    } data;
} logged_sample_t;

typedef struct {
    bool (*init)(const char* filename);
    bool (*log_sample)(const logged_sample_t* sample);
    bool (*close)(void);
} data_logger_ops_t;

typedef struct {
    bool (*init)(const char* filename);
    int  (*read_samples)(logged_sample_t* samples, int max_samples);
    bool (*seek)(uint32_t timestamp);
    bool (*close)(void);
} data_replay_ops_t;

// =============================================================================
// Timing and Synchronization
// =============================================================================

/**
 * @brief Synchronized timestamp for PPG/IMU alignment
 */
typedef struct {
    uint32_t system_time_ms;   ///< System time in milliseconds
    uint32_t ppg_sequence;     ///< PPG sample sequence number
    uint32_t imu_sequence;     ///< IMU sample sequence number
} sync_timestamp_t;

/**
 * @brief Get synchronized timestamp
 */
sync_timestamp_t get_sync_timestamp(void);

/**
 * @brief Calculate time offset between PPG and IMU
 */
int32_t calculate_time_offset(const ppg_sample_t* ppg, const imu_sample_t* imu);

#endif // SENSOR_INTERFACES_H
