/*
 * Sensor Manager Implementation
 * 
 * Central management for PPG and IMU sensors with abstraction layer
 * Handles sensor initialization, configuration, and data synchronization
 */

#include "sensor_manager.h"
#include "interfaces/sensor_interfaces.h"
#include "interfaces/sensor_config.h"
#include "ppg/max30101_driver.h"
#include "ppg/max86141_driver.h"
#include "imu/bma400_driver.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(sensor_manager, LOG_LEVEL_DBG);

/* ==== SENSOR REGISTRY ==== */

typedef struct sensor_registry_entry {
    const char* name;
    const ppg_sensor_ops_t* ppg_ops;
    const imu_sensor_ops_t* imu_ops;
} sensor_registry_entry_t;

static const sensor_registry_entry_t ppg_sensor_registry[] = {
    {"MAX30101", &max30101_ops, NULL},
    {"MAX86141", &max86141_ops, NULL},  // Will be implemented
    {NULL, NULL, NULL}
};

static const sensor_registry_entry_t imu_sensor_registry[] = {
    {"BMA400", NULL, &bma400_ops},      // Will be implemented
    {"BMI270", NULL, &bmi270_ops},      // Will be implemented
    {NULL, NULL, NULL}
};

/* ==== PRIVATE FUNCTIONS ==== */

static const ppg_sensor_ops_t* find_ppg_sensor_ops(const char* sensor_type)
{
    for (int i = 0; ppg_sensor_registry[i].name != NULL; i++) {
        if (strcmp(ppg_sensor_registry[i].name, sensor_type) == 0) {
            return ppg_sensor_registry[i].ppg_ops;
        }
    }
    return NULL;
}

static const imu_sensor_ops_t* find_imu_sensor_ops(const char* sensor_type)
{
    for (int i = 0; imu_sensor_registry[i].name != NULL; i++) {
        if (strcmp(imu_sensor_registry[i].name, sensor_type) == 0) {
            return imu_sensor_registry[i].imu_ops;
        }
    }
    return NULL;
}

/* ==== PPG SENSOR FUNCTIONS ==== */

bool ppg_sensor_init(ppg_sensor_t* sensor, const char* sensor_type, const ppg_config_t* config)
{
    if (!sensor || !sensor_type || !config) {
        LOG_ERR("Invalid parameters for PPG sensor init");
        return false;
    }
    
    // Find sensor operations
    const ppg_sensor_ops_t* ops = find_ppg_sensor_ops(sensor_type);
    if (!ops) {
        LOG_ERR("Unknown PPG sensor type: %s", sensor_type);
        return false;
    }
    
    // Initialize sensor structure
    memset(sensor, 0, sizeof(ppg_sensor_t));
    sensor->ops = ops;
    sensor->config = *config;
    sensor->initialized = false;
    sensor->running = false;
    
    // Initialize hardware
    if (!ops->init(config)) {
        LOG_ERR("Failed to initialize PPG sensor: %s", sensor_type);
        return false;
    }
    
    sensor->initialized = true;
    LOG_INF("PPG sensor initialized: %s", sensor_type);
    return true;
}

bool ppg_sensor_start(ppg_sensor_t* sensor)
{
    if (!sensor || !sensor->initialized) {
        LOG_ERR("PPG sensor not initialized");
        return false;
    }
    
    if (sensor->running) {
        LOG_WRN("PPG sensor already running");
        return true;
    }
    
    if (!sensor->ops->start()) {
        LOG_ERR("Failed to start PPG sensor");
        return false;
    }
    
    sensor->running = true;
    LOG_INF("PPG sensor started");
    return true;
}

int ppg_sensor_read(ppg_sensor_t* sensor, ppg_sample_t* samples, int max_samples)
{
    if (!sensor || !sensor->running || !samples || max_samples <= 0) {
        return 0;
    }
    
    return sensor->ops->read_fifo(samples, max_samples);
}

bool ppg_sensor_stop(ppg_sensor_t* sensor)
{
    if (!sensor || !sensor->running) {
        return true;
    }
    
    if (!sensor->ops->stop()) {
        LOG_ERR("Failed to stop PPG sensor");
        return false;
    }
    
    sensor->running = false;
    LOG_INF("PPG sensor stopped");
    return true;
}

bool ppg_sensor_reset(ppg_sensor_t* sensor)
{
    if (!sensor || !sensor->initialized) {
        return false;
    }
    
    bool was_running = sensor->running;
    
    if (was_running) {
        ppg_sensor_stop(sensor);
    }
    
    if (!sensor->ops->reset()) {
        LOG_ERR("Failed to reset PPG sensor");
        return false;
    }
    
    if (was_running) {
        return ppg_sensor_start(sensor);
    }
    
    LOG_INF("PPG sensor reset");
    return true;
}

/* ==== IMU SENSOR FUNCTIONS ==== */

bool imu_sensor_init(imu_sensor_t* sensor, const char* sensor_type, const imu_config_t* config)
{
    if (!sensor || !sensor_type || !config) {
        LOG_ERR("Invalid parameters for IMU sensor init");
        return false;
    }
    
    // Find sensor operations
    const imu_sensor_ops_t* ops = find_imu_sensor_ops(sensor_type);
    if (!ops) {
        LOG_ERR("Unknown IMU sensor type: %s", sensor_type);
        return false;
    }
    
    // Initialize sensor structure
    memset(sensor, 0, sizeof(imu_sensor_t));
    sensor->ops = ops;
    sensor->config = *config;
    sensor->initialized = false;
    sensor->running = false;
    
    // Initialize hardware
    if (!ops->init(config)) {
        LOG_ERR("Failed to initialize IMU sensor: %s", sensor_type);
        return false;
    }
    
    sensor->initialized = true;
    LOG_INF("IMU sensor initialized: %s", sensor_type);
    return true;
}

bool imu_sensor_start(imu_sensor_t* sensor)
{
    if (!sensor || !sensor->initialized) {
        LOG_ERR("IMU sensor not initialized");
        return false;
    }
    
    if (sensor->running) {
        LOG_WRN("IMU sensor already running");
        return true;
    }
    
    if (!sensor->ops->start()) {
        LOG_ERR("Failed to start IMU sensor");
        return false;
    }
    
    sensor->running = true;
    LOG_INF("IMU sensor started");
    return true;
}

int imu_sensor_read(imu_sensor_t* sensor, imu_sample_t* samples, int max_samples)
{
    if (!sensor || !sensor->running || !samples || max_samples <= 0) {
        return 0;
    }
    
    return sensor->ops->read_fifo(samples, max_samples);
}

bool imu_sensor_stop(imu_sensor_t* sensor)
{
    if (!sensor || !sensor->running) {
        return true;
    }
    
    if (!sensor->ops->stop()) {
        LOG_ERR("Failed to stop IMU sensor");
        return false;
    }
    
    sensor->running = false;
    LOG_INF("IMU sensor stopped");
    return true;
}

bool imu_sensor_reset(imu_sensor_t* sensor)
{
    if (!sensor || !sensor->initialized) {
        return false;
    }
    
    bool was_running = sensor->running;
    
    if (was_running) {
        imu_sensor_stop(sensor);
    }
    
    if (!sensor->ops->reset()) {
        LOG_ERR("Failed to reset IMU sensor");
        return false;
    }
    
    if (was_running) {
        return imu_sensor_start(sensor);
    }
    
    LOG_INF("IMU sensor reset");
    return true;
}

/* ==== SENSOR MANAGER FUNCTIONS ==== */

bool sensor_manager_init(sensor_manager_t* manager, const sensor_system_config_t* config)
{
    if (!manager || !config) {
        LOG_ERR("Invalid parameters for sensor manager init");
        return false;
    }
    
    memset(manager, 0, sizeof(sensor_manager_t));
    manager->config = *config;
    manager->base_timestamp = k_uptime_get_32();
    manager->synchronized = config->sync_timestamps;
    
    // Allocate sensor structures
    manager->ppg = k_malloc(sizeof(ppg_sensor_t));
    manager->imu = k_malloc(sizeof(imu_sensor_t));
    
    if (!manager->ppg || !manager->imu) {
        LOG_ERR("Failed to allocate sensor structures");
        return false;
    }
    
    // Initialize PPG sensor
    if (!ppg_sensor_init(manager->ppg, config->ppg_sensor_type, &config->ppg_config)) {
        LOG_ERR("Failed to initialize PPG sensor");
        return false;
    }
    
    // Initialize IMU sensor
    if (!imu_sensor_init(manager->imu, config->imu_sensor_type, &config->imu_config)) {
        LOG_ERR("Failed to initialize IMU sensor");
        return false;
    }
    
    LOG_INF("Sensor manager initialized with PPG:%s, IMU:%s", 
            config->ppg_sensor_type, config->imu_sensor_type);
    return true;
}

bool sensor_manager_start(sensor_manager_t* manager)
{
    if (!manager) {
        return false;
    }
    
    // Start sensors
    bool ppg_ok = ppg_sensor_start(manager->ppg);
    bool imu_ok = imu_sensor_start(manager->imu);
    
    if (!ppg_ok || !imu_ok) {
        LOG_ERR("Failed to start sensors (PPG:%d, IMU:%d)", ppg_ok, imu_ok);
        return false;
    }
    
    // Reset base timestamp for synchronization
    manager->base_timestamp = k_uptime_get_32();
    
    LOG_INF("Sensor manager started");
    return true;
}

int sensor_manager_read_synchronized(sensor_manager_t* manager, synchronized_sample_t* sample)
{
    if (!manager || !sample) {
        return 0;
    }
    
    ppg_sample_t ppg_data;
    imu_sample_t imu_data;
    
    // Read from both sensors
    int ppg_samples = ppg_sensor_read(manager->ppg, &ppg_data, 1);
    int imu_samples = imu_sensor_read(manager->imu, &imu_data, 1);
    
    if (ppg_samples == 0 && imu_samples == 0) {
        return 0;
    }
    
    // Fill synchronized sample
    sample->timestamp = k_uptime_get_32();
    sample->ppg_valid = (ppg_samples > 0);
    sample->imu_valid = (imu_samples > 0);
    
    if (sample->ppg_valid) {
        sample->ppg = ppg_data;
    }
    
    if (sample->imu_valid) {
        sample->imu = imu_data;
    }
    
    // Perform timestamp synchronization if enabled
    if (manager->synchronized && sample->ppg_valid && sample->imu_valid) {
        sensor_sync_timestamps(&sample->ppg.timestamp, &sample->imu.timestamp, 
                               manager->base_timestamp);
    }
    
    return 1;
}

bool sensor_manager_stop(sensor_manager_t* manager)
{
    if (!manager) {
        return true;
    }
    
    bool ppg_ok = ppg_sensor_stop(manager->ppg);
    bool imu_ok = imu_sensor_stop(manager->imu);
    
    LOG_INF("Sensor manager stopped (PPG:%d, IMU:%d)", ppg_ok, imu_ok);
    return ppg_ok && imu_ok;
}

/* ==== UTILITY FUNCTIONS ==== */

uint32_t sensor_get_timestamp(void)
{
    return k_uptime_get_32();
}

bool sensor_sync_timestamps(uint32_t* ppg_ts, uint32_t* imu_ts, uint32_t base_ts)
{
    if (!ppg_ts || !imu_ts) {
        return false;
    }
    
    // Simple synchronization: align to base timestamp
    uint32_t current_time = k_uptime_get_32();
    uint32_t offset = current_time - base_ts;
    
    *ppg_ts = base_ts + offset;
    *imu_ts = base_ts + offset;
    
    return true;
}
