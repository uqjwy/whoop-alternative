#ifndef SIGNAL_PIPELINE_INTERFACES_H
#define SIGNAL_PIPELINE_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>
#include "sensor_interfaces.h"

/**
 * @file signal_pipeline_interfaces.h
 * @brief Modular signal processing pipeline abstraction
 * 
 * This enables easy swapping of signal processing algorithms and
 * automatic parameter tuning when switching sensors (e.g., MAX30101 → MAX86141).
 * Each processing stage is independent and configurable.
 */

// =============================================================================
// Pipeline Stage Types
// =============================================================================

typedef enum {
    PIPELINE_STAGE_PREPROCESS = 0,    ///< Preprocessing (normalization, calibration)
    PIPELINE_STAGE_FILTER,            ///< Digital filtering (DC removal, bandpass)
    PIPELINE_STAGE_ARTIFACT_REMOVAL,  ///< Motion artifact compensation
    PIPELINE_STAGE_FEATURE_EXTRACT,   ///< Feature extraction (peaks, valleys)
    PIPELINE_STAGE_ALGORITHM,         ///< Algorithm execution (HR, HRV, SpO2)
    PIPELINE_STAGE_POSTPROCESS,       ///< Post-processing (smoothing, validation)
    PIPELINE_STAGE_COUNT
} pipeline_stage_type_t;

typedef enum {
    PIPELINE_SIGNAL_PPG = 0,
    PIPELINE_SIGNAL_IMU,
    PIPELINE_SIGNAL_TEMP,
    PIPELINE_SIGNAL_FUSION,           ///< Multi-sensor fusion
    PIPELINE_SIGNAL_COUNT
} pipeline_signal_type_t;

// =============================================================================
// Pipeline Data Structures
// =============================================================================

/**
 * @brief Generic signal data buffer
 */
typedef struct {
    float* data;                      ///< Signal data array
    uint32_t length;                  ///< Number of samples
    uint32_t sample_rate;             ///< Sample rate in Hz
    uint32_t timestamp_start;         ///< Start timestamp
    float quality_score;              ///< Signal quality (0.0-1.0)
    void* metadata;                   ///< Stage-specific metadata
} signal_buffer_t;

/**
 * @brief Pipeline stage configuration
 */
typedef struct {
    bool enabled;                     ///< Stage enabled/disabled
    uint32_t buffer_size;             ///< Required buffer size
    float parameters[16];             ///< Algorithm parameters
    uint32_t parameter_count;         ///< Number of active parameters
    char algorithm_name[32];          ///< Algorithm identifier
} pipeline_stage_config_t;

/**
 * @brief Pipeline stage operations
 */
typedef struct {
    bool (*init)(const pipeline_stage_config_t* config);
    bool (*process)(const signal_buffer_t* input, signal_buffer_t* output);
    bool (*reset)(void);
    bool (*update_config)(const pipeline_stage_config_t* config);
    bool (*get_status)(float* quality, uint32_t* latency_us);
    void (*cleanup)(void);
} pipeline_stage_ops_t;

/**
 * @brief Pipeline stage descriptor
 */
typedef struct {
    const char* name;                 ///< Stage name
    pipeline_stage_type_t type;       ///< Stage type
    pipeline_stage_ops_t* ops;        ///< Operations
    pipeline_stage_config_t config;   ///< Current configuration
    bool is_adaptive;                 ///< Supports adaptive parameter tuning
    uint32_t processing_time_us;      ///< Last processing time
} pipeline_stage_t;

// =============================================================================
// Complete Pipeline Definition
// =============================================================================

/**
 * @brief Signal processing pipeline
 */
typedef struct {
    pipeline_signal_type_t signal_type;
    pipeline_stage_t* stages[8];      ///< Processing stages (max 8)
    uint32_t stage_count;             ///< Number of active stages
    
    // Buffers
    signal_buffer_t input_buffer;     ///< Input buffer
    signal_buffer_t stage_buffers[8]; ///< Intermediate buffers
    signal_buffer_t output_buffer;    ///< Final output
    
    // Performance metrics
    uint32_t total_latency_us;        ///< Total processing latency
    float overall_quality;            ///< Overall signal quality
    uint32_t samples_processed;       ///< Total samples processed
    uint32_t errors;                  ///< Error count
    
    // Adaptive tuning
    bool adaptive_tuning;             ///< Enable adaptive parameter tuning
    float target_quality;             ///< Target quality threshold
    uint32_t adaptation_interval;     ///< Adaptation check interval
} signal_pipeline_t;

// =============================================================================
// PPG-Specific Pipeline Stages
// =============================================================================

/**
 * @brief PPG preprocessing stage (normalization, calibration)
 */
typedef struct {
    pipeline_stage_t base;
    struct {
        float adc_scale_factor;       ///< ADC to physical units conversion
        float dc_offset[4];           ///< Per-channel DC offset
        float gain_correction[4];     ///< Per-channel gain correction
        bool enable_calibration;      ///< Enable real-time calibration
    } params;
} ppg_preprocess_stage_t;

/**
 * @brief PPG filtering stage (DC removal, bandpass)
 */
typedef struct {
    pipeline_stage_t base;
    struct {
        float dc_alpha;               ///< DC removal filter alpha
        float bandpass_low_hz;        ///< Bandpass low cutoff
        float bandpass_high_hz;       ///< Bandpass high cutoff
        uint32_t filter_order;        ///< Filter order
        bool enable_notch_50hz;       ///< 50Hz notch filter
        bool enable_notch_60hz;       ///< 60Hz notch filter
    } params;
} ppg_filter_stage_t;

/**
 * @brief PPG motion artifact removal
 */
typedef struct {
    pipeline_stage_t base;
    struct {
        bool use_imu_data;            ///< Use IMU for motion compensation
        float motion_threshold;       ///< Motion detection threshold
        uint32_t artifact_window;     ///< Artifact detection window
        bool enable_adaptive_filter;  ///< Adaptive filtering
        float correlation_threshold;  ///< IMU-PPG correlation threshold
    } params;
    signal_buffer_t* imu_buffer;      ///< Reference to IMU data
} ppg_artifact_removal_stage_t;

/**
 * @brief PPG feature extraction (peaks, HR, HRV)
 */
typedef struct {
    pipeline_stage_t base;
    struct {
        float peak_threshold;         ///< Peak detection threshold
        uint32_t min_peak_distance;   ///< Minimum peak distance
        uint32_t hr_window_size;      ///< HR calculation window
        bool enable_hrv;              ///< Enable HRV calculation
        bool enable_spo2;             ///< Enable SpO2 calculation
    } params;
    float last_hr_bpm;               ///< Last calculated HR
    float last_hrv_rmssd;            ///< Last HRV RMSSD
} ppg_feature_stage_t;

// =============================================================================
// IMU-Specific Pipeline Stages
// =============================================================================

/**
 * @brief IMU preprocessing (calibration, orientation)
 */
typedef struct {
    pipeline_stage_t base;
    struct {
        float accel_bias[3];          ///< Accelerometer bias correction
        float gyro_bias[3];           ///< Gyroscope bias correction
        float sensitivity_scale[6];   ///< Sensitivity scaling factors
        bool auto_calibration;        ///< Enable automatic calibration
    } params;
} imu_preprocess_stage_t;

/**
 * @brief IMU activity classification
 */
typedef struct {
    pipeline_stage_t base;
    struct {
        float still_threshold;        ///< Still activity threshold
        float walking_threshold;      ///< Walking activity threshold
        float running_threshold;      ///< Running activity threshold
        uint32_t window_size;         ///< Classification window
        bool enable_step_counting;    ///< Enable step counter
    } params;
    uint32_t current_activity;       ///< Current activity type
    uint32_t step_count;             ///< Current step count
} imu_activity_stage_t;

// =============================================================================
// Pipeline Management Functions
// =============================================================================

/**
 * @brief Create a new signal processing pipeline
 */
signal_pipeline_t* pipeline_create(pipeline_signal_type_t signal_type);

/**
 * @brief Add processing stage to pipeline
 */
bool pipeline_add_stage(signal_pipeline_t* pipeline, pipeline_stage_t* stage);

/**
 * @brief Remove processing stage from pipeline
 */
bool pipeline_remove_stage(signal_pipeline_t* pipeline, const char* stage_name);

/**
 * @brief Process signal through complete pipeline
 */
bool pipeline_process(signal_pipeline_t* pipeline, const signal_buffer_t* input);

/**
 * @brief Get pipeline output
 */
const signal_buffer_t* pipeline_get_output(signal_pipeline_t* pipeline);

/**
 * @brief Reset entire pipeline
 */
bool pipeline_reset(signal_pipeline_t* pipeline);

/**
 * @brief Update pipeline configuration
 */
bool pipeline_update_config(signal_pipeline_t* pipeline, const char* config_string);

/**
 * @brief Get pipeline performance metrics
 */
bool pipeline_get_metrics(signal_pipeline_t* pipeline, 
                         uint32_t* latency_us, 
                         float* quality, 
                         uint32_t* throughput);

/**
 * @brief Enable/disable adaptive tuning
 */
bool pipeline_set_adaptive(signal_pipeline_t* pipeline, bool enable, float target_quality);

/**
 * @brief Cleanup and destroy pipeline
 */
void pipeline_destroy(signal_pipeline_t* pipeline);

// =============================================================================
// Sensor-Specific Pipeline Factory Functions
// =============================================================================

/**
 * @brief Create PPG pipeline optimized for specific sensor
 */
signal_pipeline_t* pipeline_create_ppg(const char* sensor_name, 
                                       const ppg_config_t* sensor_config);

/**
 * @brief Create IMU pipeline optimized for specific sensor
 */
signal_pipeline_t* pipeline_create_imu(const char* sensor_name, 
                                       const imu_config_t* sensor_config);

/**
 * @brief Create multi-sensor fusion pipeline
 */
signal_pipeline_t* pipeline_create_fusion(signal_pipeline_t* ppg_pipeline, 
                                          signal_pipeline_t* imu_pipeline);

/**
 * @brief Auto-tune pipeline for sensor migration
 */
bool pipeline_auto_tune_for_sensor(signal_pipeline_t* pipeline, 
                                   const char* old_sensor, 
                                   const char* new_sensor);

// =============================================================================
// Predefined Pipeline Configurations
// =============================================================================

// MAX30101 optimized PPG pipeline
extern const pipeline_stage_config_t PPG_MAX30101_PREPROCESS_CONFIG;
extern const pipeline_stage_config_t PPG_MAX30101_FILTER_CONFIG;
extern const pipeline_stage_config_t PPG_MAX30101_ARTIFACT_CONFIG;
extern const pipeline_stage_config_t PPG_MAX30101_FEATURE_CONFIG;

// MAX86141 optimized PPG pipeline
extern const pipeline_stage_config_t PPG_MAX86141_PREPROCESS_CONFIG;
extern const pipeline_stage_config_t PPG_MAX86141_FILTER_CONFIG;
extern const pipeline_stage_config_t PPG_MAX86141_ARTIFACT_CONFIG;
extern const pipeline_stage_config_t PPG_MAX86141_FEATURE_CONFIG;

// BMA400 optimized IMU pipeline
extern const pipeline_stage_config_t IMU_BMA400_PREPROCESS_CONFIG;
extern const pipeline_stage_config_t IMU_BMA400_ACTIVITY_CONFIG;

// BMI270 optimized IMU pipeline
extern const pipeline_stage_config_t IMU_BMI270_PREPROCESS_CONFIG;
extern const pipeline_stage_config_t IMU_BMI270_ACTIVITY_CONFIG;

// =============================================================================
// Pipeline Utilities
// =============================================================================

/**
 * @brief Load pipeline configuration from file
 */
bool pipeline_load_config(const char* filename, signal_pipeline_t* pipeline);

/**
 * @brief Save pipeline configuration to file
 */
bool pipeline_save_config(const char* filename, const signal_pipeline_t* pipeline);

/**
 * @brief Compare two pipeline configurations
 */
bool pipeline_configs_equal(const signal_pipeline_t* a, const signal_pipeline_t* b);

/**
 * @brief Get recommended configuration for use case
 */
bool pipeline_get_recommended_config(const char* use_case, 
                                     const char* sensor_name,
                                     signal_pipeline_t* pipeline);

#endif // SIGNAL_PIPELINE_INTERFACES_H
