#ifndef POWER_INTERFACES_H
#define POWER_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file power_interfaces.h
 * @brief Unified power management interfaces for all components
 * 
 * This abstraction enables consistent power management across sensors,
 * peripherals, and system components. Makes it easy to implement
 * power profiles and optimize battery life.
 */

// =============================================================================
// Power State Definitions
// =============================================================================

typedef enum {
    POWER_STATE_OFF = 0,           ///< Component completely powered off
    POWER_STATE_SLEEP,             ///< Low power sleep mode
    POWER_STATE_STANDBY,           ///< Ready to wake quickly
    POWER_STATE_LOW_POWER,         ///< Reduced performance mode
    POWER_STATE_NORMAL,            ///< Standard operation
    POWER_STATE_HIGH_PERFORMANCE,  ///< Maximum performance mode
    POWER_STATE_COUNT
} power_state_t;

typedef enum {
    POWER_PROFILE_ULTRA_LOW = 0,   ///< Maximum battery life (weeks)
    POWER_PROFILE_LOW,             ///< Extended battery life (7+ days)
    POWER_PROFILE_BALANCED,        ///< Balanced performance/battery (5-7 days)
    POWER_PROFILE_PERFORMANCE,     ///< High accuracy (3-5 days)
    POWER_PROFILE_MAX_PERFORMANCE, ///< Maximum accuracy (1-2 days)
    POWER_PROFILE_COUNT
} power_profile_t;

// =============================================================================
// Power Management Interface
// =============================================================================

/**
 * @brief Power consumption info
 */
typedef struct {
    uint32_t current_ua;           ///< Current consumption in μA
    uint32_t voltage_mv;           ///< Operating voltage in mV
    uint32_t power_uw;             ///< Power consumption in μW
    float duty_cycle;              ///< Duty cycle (0.0-1.0)
    uint32_t wakeup_time_us;       ///< Time to transition to active state
} power_info_t;

/**
 * @brief Power management operations interface
 */
typedef struct {
    bool (*set_state)(power_state_t state);
    power_state_t (*get_state)(void);
    bool (*get_power_info)(power_info_t* info);
    bool (*set_duty_cycle)(float duty_cycle);
    bool (*enable_wakeup_source)(bool enable);
    uint32_t (*get_wakeup_latency)(power_state_t from_state);
} power_ops_t;

/**
 * @brief Power-managed component descriptor
 */
typedef struct {
    const char* name;              ///< Component name
    power_ops_t* ops;              ///< Power operations
    power_state_t current_state;   ///< Current power state
    bool is_critical;              ///< Critical component (never fully off)
    uint32_t max_sleep_time_ms;    ///< Maximum sleep time before mandatory wake
} power_component_t;

// =============================================================================
// System Power Manager
// =============================================================================

/**
 * @brief System power statistics
 */
typedef struct {
    uint32_t total_power_uw;       ///< Total system power consumption
    uint32_t battery_level_percent; ///< Battery level (0-100%)
    uint32_t estimated_runtime_h;   ///< Estimated runtime in hours
    uint32_t uptime_s;             ///< System uptime in seconds
    uint32_t sleep_time_s;         ///< Total time in sleep states
    float sleep_efficiency;        ///< Sleep efficiency (sleep_time/uptime)
} power_stats_t;

/**
 * @brief Power profile configuration
 */
typedef struct {
    power_profile_t profile;
    struct {
        power_state_t ppg_state;
        uint32_t ppg_sample_rate;
        uint32_t ppg_led_current;
        float ppg_duty_cycle;
    } ppg;
    
    struct {
        power_state_t imu_state;
        uint32_t imu_sample_rate;
        bool imu_interrupt_mode;
        float imu_duty_cycle;
    } imu;
    
    struct {
        power_state_t system_state;
        uint32_t ble_interval_ms;
        uint32_t flash_flush_interval_s;
        bool enable_deep_sleep;
        uint32_t deep_sleep_timeout_s;
    } system;
} power_profile_config_t;

// =============================================================================
// System Power Manager Operations
// =============================================================================

/**
 * @brief Initialize power management system
 */
bool power_manager_init(void);

/**
 * @brief Register power-managed component
 */
bool power_manager_register_component(power_component_t* component);

/**
 * @brief Set system power profile
 */
bool power_manager_set_profile(power_profile_t profile);

/**
 * @brief Get current power profile
 */
power_profile_t power_manager_get_profile(void);

/**
 * @brief Apply custom power configuration
 */
bool power_manager_apply_config(const power_profile_config_t* config);

/**
 * @brief Request system sleep (will check all components)
 */
bool power_manager_request_sleep(uint32_t duration_ms);

/**
 * @brief Force system wake (emergency wake)
 */
void power_manager_force_wake(void);

/**
 * @brief Get system power statistics
 */
bool power_manager_get_stats(power_stats_t* stats);

/**
 * @brief Update power budget based on battery level
 */
bool power_manager_update_budget(uint32_t battery_percent);

/**
 * @brief Enable/disable adaptive power management
 */
bool power_manager_set_adaptive(bool enable);

// =============================================================================
// Predefined Power Profiles
// =============================================================================

// Ultra Low Power: Maximum battery life
static const power_profile_config_t POWER_PROFILE_ULTRA_LOW_CONFIG = {
    .profile = POWER_PROFILE_ULTRA_LOW,
    .ppg = {
        .ppg_state = POWER_STATE_LOW_POWER,
        .ppg_sample_rate = 12,         // 12 Hz minimum for HR
        .ppg_led_current = 5,          // Very low LED current
        .ppg_duty_cycle = 0.1f         // 10% duty cycle
    },
    .imu = {
        .imu_state = POWER_STATE_SLEEP,
        .imu_sample_rate = 1,          // 1 Hz for basic activity
        .imu_interrupt_mode = true,    // Interrupt-based only
        .imu_duty_cycle = 0.01f        // 1% duty cycle
    },
    .system = {
        .system_state = POWER_STATE_LOW_POWER,
        .ble_interval_ms = 2000,       // 2s BLE interval
        .flash_flush_interval_s = 3600, // 1h flash sync
        .enable_deep_sleep = true,
        .deep_sleep_timeout_s = 60     // 1min timeout
    }
};

// Balanced Profile: Default operation
static const power_profile_config_t POWER_PROFILE_BALANCED_CONFIG = {
    .profile = POWER_PROFILE_BALANCED,
    .ppg = {
        .ppg_state = POWER_STATE_NORMAL,
        .ppg_sample_rate = 50,         // 50 Hz for good HR accuracy
        .ppg_led_current = 25,         // Standard LED current
        .ppg_duty_cycle = 0.5f         // 50% duty cycle
    },
    .imu = {
        .imu_state = POWER_STATE_NORMAL,
        .imu_sample_rate = 25,         // 25 Hz for activity tracking
        .imu_interrupt_mode = false,   // Continuous sampling
        .imu_duty_cycle = 1.0f         // Always on for activity
    },
    .system = {
        .system_state = POWER_STATE_NORMAL,
        .ble_interval_ms = 400,        // 400ms BLE interval
        .flash_flush_interval_s = 300, // 5min flash sync
        .enable_deep_sleep = true,
        .deep_sleep_timeout_s = 300    // 5min timeout
    }
};

// =============================================================================
// Power-Aware Sensor Wrappers
// =============================================================================

/**
 * @brief Power-aware PPG sensor wrapper
 */
typedef struct {
    power_component_t power_component;
    void* sensor_driver;              ///< Pointer to actual sensor driver
    power_state_t target_state;       ///< Target power state
    uint32_t last_sample_time;        ///< Last sampling timestamp
    bool adaptive_sampling;           ///< Enable adaptive sampling based on signal quality
} power_aware_ppg_t;

/**
 * @brief Power-aware IMU sensor wrapper
 */
typedef struct {
    power_component_t power_component;
    void* sensor_driver;              ///< Pointer to actual sensor driver
    power_state_t target_state;       ///< Target power state
    uint32_t motion_threshold;        ///< Motion detection threshold
    bool activity_based_sampling;    ///< Adjust sampling based on activity level
} power_aware_imu_t;

// =============================================================================
// Power Management Utilities
// =============================================================================

/**
 * @brief Calculate estimated battery life for profile
 */
uint32_t power_calculate_battery_life_h(power_profile_t profile, uint32_t battery_capacity_mah);

/**
 * @brief Get optimal power profile for target battery life
 */
power_profile_t power_get_optimal_profile(uint32_t target_runtime_h, uint32_t current_battery_percent);

/**
 * @brief Check if component can enter sleep state
 */
bool power_can_component_sleep(const power_component_t* component, uint32_t duration_ms);

/**
 * @brief Calculate power impact of configuration change
 */
int32_t power_calculate_impact_uw(const power_profile_config_t* old_config, 
                                  const power_profile_config_t* new_config);

#endif // POWER_INTERFACES_H
