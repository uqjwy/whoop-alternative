#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "interfaces/sensor_interfaces.h"
#include "interfaces/sensor_config.h"

/**
 * @file sensor_manager.h
 * @brief Central sensor management for sensor-agnostic architecture
 * 
 * This manager coordinates all sensors and provides a unified interface
 * to the application layer. It handles sensor registration, configuration,
 * and data collection.
 */

// =============================================================================
// Manager Structures
// =============================================================================

/**
 * @brief Sensor manager state
 */
typedef struct {
    sensor_config_t config;                    ///< Current configuration
    sensor_descriptor_t* sensors[16];          ///< Registered sensors
    int sensor_count;                          ///< Number of registered sensors
    
    // Active sensors
    ppg_sensor_ops_t* active_ppg;              ///< Active PPG sensor
    imu_sensor_ops_t* active_imu;              ///< Active IMU sensor
    
    // State flags
    bool initialized;                          ///< Manager initialized
    bool ppg_running;                          ///< PPG measurement active
    bool imu_running;                          ///< IMU measurement active
    
    // Statistics
    uint32_t ppg_samples_read;                 ///< Total PPG samples read
    uint32_t imu_samples_read;                 ///< Total IMU samples read
    uint32_t errors;                           ///< Error count
} sensor_manager_t;

// =============================================================================
// Function Declarations
// =============================================================================

/**
 * @brief Initialize sensor manager
 * @param manager Pointer to manager structure
 * @param config_file Path to configuration file (optional)
 * @return true if successful, false otherwise
 */
bool sensor_manager_init(sensor_manager_t* manager, const char* config_file);

/**
 * @brief Register a sensor with the manager
 * @param manager Pointer to manager structure
 * @param descriptor Sensor descriptor
 * @return true if successful, false otherwise
 */
bool sensor_manager_register(sensor_manager_t* manager, sensor_descriptor_t* descriptor);

/**
 * @brief Start PPG measurement
 * @param manager Pointer to manager structure
 * @return true if successful, false otherwise
 */
bool sensor_manager_start_ppg(sensor_manager_t* manager);

/**
 * @brief Start IMU measurement
 * @param manager Pointer to manager structure
 * @return true if successful, false otherwise
 */
bool sensor_manager_start_imu(sensor_manager_t* manager);

/**
 * @brief Stop PPG measurement
 * @param manager Pointer to manager structure
 * @return true if successful, false otherwise
 */
bool sensor_manager_stop_ppg(sensor_manager_t* manager);

/**
 * @brief Stop IMU measurement
 * @param manager Pointer to manager structure
 * @return true if successful, false otherwise
 */
bool sensor_manager_stop_imu(sensor_manager_t* manager);

/**
 * @brief Read PPG samples
 * @param manager Pointer to manager structure
 * @param samples Array to store samples
 * @param max_samples Maximum number of samples to read
 * @return Number of samples read
 */
int sensor_manager_read_ppg(sensor_manager_t* manager, ppg_sample_t* samples, int max_samples);

/**
 * @brief Read IMU samples
 * @param manager Pointer to manager structure
 * @param samples Array to store samples
 * @param max_samples Maximum number of samples to read
 * @return Number of samples read
 */
int sensor_manager_read_imu(sensor_manager_t* manager, imu_sample_t* samples, int max_samples);

/**
 * @brief Update sensor configuration
 * @param manager Pointer to manager structure
 * @param config New configuration
 * @return true if successful, false otherwise
 */
bool sensor_manager_set_config(sensor_manager_t* manager, const sensor_config_t* config);

/**
 * @brief Get current configuration
 * @param manager Pointer to manager structure
 * @param config Pointer to store configuration
 * @return true if successful, false otherwise
 */
bool sensor_manager_get_config(sensor_manager_t* manager, sensor_config_t* config);

/**
 * @brief Get manager status
 * @param manager Pointer to manager structure
 * @param ppg_running Pointer to store PPG status
 * @param imu_running Pointer to store IMU status
 * @return true if successful, false otherwise
 */
bool sensor_manager_get_status(sensor_manager_t* manager, bool* ppg_running, bool* imu_running);

/**
 * @brief Reset all sensors
 * @param manager Pointer to manager structure
 * @return true if successful, false otherwise
 */
bool sensor_manager_reset_all(sensor_manager_t* manager);

/**
 * @brief Switch to different sensor profile
 * @param manager Pointer to manager structure
 * @param profile Profile to switch to
 * @return true if successful, false otherwise
 */
bool sensor_manager_switch_profile(sensor_manager_t* manager, config_profile_t profile);

/**
 * @brief Get sensor statistics
 * @param manager Pointer to manager structure
 * @param ppg_samples Pointer to store PPG sample count
 * @param imu_samples Pointer to store IMU sample count
 * @param errors Pointer to store error count
 */
void sensor_manager_get_stats(sensor_manager_t* manager, 
                             uint32_t* ppg_samples, 
                             uint32_t* imu_samples, 
                             uint32_t* errors);

/**
 * @brief Cleanup and deinitialize manager
 * @param manager Pointer to manager structure
 */
void sensor_manager_cleanup(sensor_manager_t* manager);

// =============================================================================
// Convenience Macros
// =============================================================================

#define SENSOR_MANAGER_PPG_ACTIVE(mgr)  ((mgr)->ppg_running && (mgr)->active_ppg != NULL)
#define SENSOR_MANAGER_IMU_ACTIVE(mgr)  ((mgr)->imu_running && (mgr)->active_imu != NULL)
#define SENSOR_MANAGER_IS_READY(mgr)    ((mgr)->initialized && (mgr)->active_ppg != NULL)

#endif // SENSOR_MANAGER_H
