/*
 * Whoop Alternative - Main Application
 * 
 * Main entry point for nRF52840 firmware with Zephyr RTOS
 * Manages sensor sampling, BLE communication, and power management
 * 
 * Enhanced with advanced architecture optimizations:
 * - Sensor-agnostic architecture with runtime config
 * - Power management abstraction with adaptive profiles
 * - Modular signal processing pipeline
 * - Storage abstraction with policy management
 * - BLE service abstraction with dynamic registration
 * - Configuration hot-reload without restart
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>

#include "app_config.h"
#include "app_states.h"

/* Advanced Architecture Interfaces */
#include "../../drivers/interfaces/sensor_interfaces.h"
#include "../../drivers/interfaces/sensor_config.h"
#include "../../drivers/interfaces/power_interfaces.h"
#include "../../drivers/interfaces/signal_pipeline_interfaces.h"
#include "../../drivers/interfaces/storage_interfaces.h"
#include "../../drivers/interfaces/ble_service_interfaces.h"
#include "../../drivers/interfaces/config_hotreload_interfaces.h"

#include "../../drivers/sensor_manager.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Application state */
static app_state_t current_state = APP_STATE_INIT;

/* Advanced Architecture Components */
static sensor_manager_t sensor_manager;
static power_manager_t power_manager;
static signal_pipeline_t signal_pipeline;
static storage_manager_t storage_manager;
static ble_service_manager_t ble_manager;
static config_hotreload_t config_manager;

/* Thread stacks */
K_THREAD_STACK_DEFINE(sensor_thread_stack, 2048);
K_THREAD_STACK_DEFINE(ble_thread_stack, 2048);
K_THREAD_STACK_DEFINE(power_thread_stack, 1024);

/* Thread structures */
static struct k_thread sensor_thread_data;
static struct k_thread ble_thread_data;
static struct k_thread power_thread_data;

/* Function prototypes */
static void sensor_thread_func(void *arg1, void *arg2, void *arg3);
static void ble_thread_func(void *arg1, void *arg2, void *arg3);
static void power_thread_func(void *arg1, void *arg2, void *arg3);
static int app_init(void);

/**
 * Sensor sampling thread - Enhanced with abstraction layer
 * Handles sensor-agnostic data acquisition with power management
 */
static void sensor_thread_func(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("Sensor thread started with abstraction layer");
    
    sensor_data_t sensor_data;
    
    while (1) {
        // Get current power profile and adjust sampling accordingly
        power_profile_t profile = power_manager.ops->get_current_profile(&power_manager);
        
        switch (current_state) {
        case APP_STATE_MEASURING:
            // Use unified sensor interface for data acquisition
            if (sensor_manager_read_all(&sensor_manager, &sensor_data) == 0) {
                // Process through modular signal pipeline
                signal_pipeline.ops->process_data(&signal_pipeline, &sensor_data);
                
                // Store using abstracted storage
                storage_manager.ops->store_sensor_data(&storage_manager, &sensor_data);
                
                // Update BLE characteristics
                ble_manager.ops->update_sensor_data(&ble_manager, &sensor_data);
            }
            
            // Adaptive sampling rate based on power profile
            uint32_t sample_interval = (profile == POWER_PROFILE_ULTRA_LOW) ? 40 : 20; // 25Hz vs 50Hz
            k_msleep(sample_interval);
            break;
            
        case APP_STATE_SLEEP:
            // Sleep mode: reduced sampling with power optimization
            power_manager.ops->enter_sleep_mode(&power_manager);
            
            // Only essential sensors active
            if (sensor_manager_read_essential(&sensor_manager, &sensor_data) == 0) {
                storage_manager.ops->store_sensor_data(&storage_manager, &sensor_data);
            }
            
            k_msleep(1000); // 1Hz in sleep mode
            break;
            
        default:
            k_msleep(100);
            break;
        }
    }
}

/**
 * BLE communication thread - Enhanced with service abstraction
 * Handles dynamic service registration and management
 */
static void ble_thread_func(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("BLE thread started with service abstraction");
    
    while (1) {
        switch (current_state) {
        case APP_STATE_ADVERTISING:
            // Use abstracted BLE manager for advertising
            ble_manager.ops->start_advertising(&ble_manager);
            
            // Check for config updates via BLE
            if (config_manager.ops->has_pending_updates(&config_manager)) {
                config_manager.ops->apply_updates(&config_manager);
                LOG_INF("Configuration updated via BLE");
            }
            break;
            
        case APP_STATE_CONNECTED:
            // Handle abstracted GATT services
            ble_manager.ops->handle_connections(&ble_manager);
            
            // Check for real-time data requests
            if (ble_manager.ops->has_data_request(&ble_manager)) {
                sensor_data_t live_data;
                if (sensor_manager_read_all(&sensor_manager, &live_data) == 0) {
                    ble_manager.ops->send_live_data(&ble_manager, &live_data);
                }
            }
            break;
            
        case APP_STATE_SYNCING:
            // Transfer stored data using storage abstraction
            data_stream_t *stream = storage_manager.ops->create_data_stream(&storage_manager, STREAM_TYPE_SYNC);
            if (stream) {
                ble_manager.ops->transfer_data_stream(&ble_manager, stream);
                storage_manager.ops->close_data_stream(&storage_manager, stream);
            }
            break;
            
        default:
            break;
        }
        
        k_msleep(100);
    }
}

/**
 * Power management thread - Enhanced with abstraction layer
 * Handles adaptive power management with profile-based optimization
 */
static void power_thread_func(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("Power management thread started with abstraction layer");
    
    while (1) {
        // Update battery level and charging status
        power_manager.ops->update_battery_status(&power_manager);
        
        // Get current battery level for adaptive power management
        uint8_t battery_level = power_manager.ops->get_battery_level(&power_manager);
        
        // Automatically adjust power profile based on battery level
        if (battery_level < 20) {
            power_manager.ops->set_profile(&power_manager, POWER_PROFILE_ULTRA_LOW);
            LOG_WRN("Battery low (%d%%), switching to ultra-low power mode", battery_level);
        } else if (battery_level < 50) {
            power_manager.ops->set_profile(&power_manager, POWER_PROFILE_BALANCED);
        } else {
            power_manager.ops->set_profile(&power_manager, POWER_PROFILE_PERFORMANCE);
        }
        
        // Check if we should enter deep sleep
        if (current_state == APP_STATE_SLEEP && battery_level < 10) {
            power_manager.ops->enter_deep_sleep(&power_manager);
        }
        
        // Monitor charging status
        if (power_manager.ops->is_charging(&power_manager)) {
            // Optimize charging profile
            power_manager.ops->optimize_charging(&power_manager);
        }
        
        k_msleep(5000); // Check every 5 seconds
    }
}

/**
 * Initialize application components - Enhanced with abstraction layer
 */
static int app_init(void)
{
    int ret;
    
    LOG_INF("Initializing Whoop Alternative firmware v%s with advanced architecture", APP_VERSION);
    
    // Initialize configuration hot-reload system first
    ret = config_hotreload_init(&config_manager, "/config/sensor_config.txt");
    if (ret) {
        LOG_ERR("Failed to initialize config manager: %d", ret);
        return ret;
    }
    
    // Initialize sensor manager with configuration
    ret = sensor_manager_init(&sensor_manager, &config_manager);
    if (ret) {
        LOG_ERR("Failed to initialize sensor manager: %d", ret);
        return ret;
    }
    
    // Initialize power management
    ret = power_manager_init(&power_manager);
    if (ret) {
        LOG_ERR("Failed to initialize power manager: %d", ret);
        return ret;
    }
    
    // Initialize signal processing pipeline
    ret = signal_pipeline_init(&signal_pipeline, &sensor_manager);
    if (ret) {
        LOG_ERR("Failed to initialize signal pipeline: %d", ret);
        return ret;
    }
    
    // Initialize storage manager
    ret = storage_manager_init(&storage_manager);
    if (ret) {
        LOG_ERR("Failed to initialize storage manager: %d", ret);
        return ret;
    }
    
    // Initialize Bluetooth
    ret = bt_enable(NULL);
    if (ret) {
        LOG_ERR("Bluetooth init failed (err %d)", ret);
        return ret;
    }
    
    // Initialize BLE service manager
    ret = ble_service_manager_init(&ble_manager);
    if (ret) {
        LOG_ERR("Failed to initialize BLE service manager: %d", ret);
        return ret;
    }
    
    // Register standard BLE services
    ble_manager.ops->register_service(&ble_manager, ble_service_heart_rate_create());
    ble_manager.ops->register_service(&ble_manager, ble_service_battery_create());
    ble_manager.ops->register_service(&ble_manager, ble_service_device_info_create());
    
    // Register custom services for sensor data
    ble_manager.ops->register_service(&ble_manager, ble_service_ppg_create());
    ble_manager.ops->register_service(&ble_manager, ble_service_imu_create());
    ble_manager.ops->register_service(&ble_manager, ble_service_config_create());
    
    LOG_INF("All systems initialized successfully with advanced architecture");
    
    return 0;
}

/**
 * Main application entry point
 */
int main(void)
{
    int ret;
    
    LOG_INF("=== Whoop Alternative Firmware v%s Starting ===", APP_VERSION);
    LOG_INF("Enhanced with sensor-agnostic architecture and advanced optimizations");
    
    // Initialize all application components
    ret = app_init();
    if (ret) {
        LOG_ERR("Application initialization failed: %d", ret);
        return ret;
    }
    
    // Create and start application threads
    k_thread_create(&sensor_thread_data, sensor_thread_stack,
                    K_THREAD_STACK_SIZEOF(sensor_thread_stack),
                    sensor_thread_func, NULL, NULL, NULL,
                    K_PRIO_COOP(7), 0, K_NO_WAIT);
    k_thread_name_set(&sensor_thread_data, "sensor_thread");
    
    k_thread_create(&ble_thread_data, ble_thread_stack,
                    K_THREAD_STACK_SIZEOF(ble_thread_stack),
                    ble_thread_func, NULL, NULL, NULL,
                    K_PRIO_COOP(6), 0, K_NO_WAIT);
    k_thread_name_set(&ble_thread_data, "ble_thread");
    
    k_thread_create(&power_thread_data, power_thread_stack,
                    K_THREAD_STACK_SIZEOF(power_thread_stack),
                    power_thread_func, NULL, NULL, NULL,
                    K_PRIO_COOP(8), 0, K_NO_WAIT);
    k_thread_name_set(&power_thread_data, "power_thread");
    
    // Start advertising
    current_state = APP_STATE_ADVERTISING;
    LOG_INF("Application started successfully - entering advertising mode");
    
    // Main loop for state management and config monitoring
    while (1) {
        // Check for configuration changes
        if (config_manager.ops->has_pending_updates(&config_manager)) {
            LOG_INF("Configuration update detected - applying changes");
            
            // Apply config changes with validation
            ret = config_manager.ops->apply_updates(&config_manager);
            if (ret == 0) {
                // Notify all subsystems of config changes
                sensor_manager.ops->on_config_changed(&sensor_manager);
                power_manager.ops->on_config_changed(&power_manager);
                signal_pipeline.ops->on_config_changed(&signal_pipeline);
                storage_manager.ops->on_config_changed(&storage_manager);
                ble_manager.ops->on_config_changed(&ble_manager);
                
                LOG_INF("Configuration successfully updated at runtime");
            } else {
                LOG_ERR("Configuration update failed, rolling back: %d", ret);
                config_manager.ops->rollback(&config_manager);
            }
        }
        
        // Periodic system health checks
        if (power_manager.ops->get_battery_level(&power_manager) < 5) {
            LOG_WRN("Critical battery level - initiating emergency shutdown");
            current_state = APP_STATE_SLEEP;
            power_manager.ops->enter_deep_sleep(&power_manager);
        }
        
        // Sleep for main loop
        k_msleep(10000); // Check every 10 seconds
    }
    
    return 0;
}
        // TODO: Handle button presses
        // TODO: Handle charging events  
        // TODO: Handle state transitions
        // TODO: Watchdog refresh
        
        k_msleep(100);
    }
    
    return 0;
}
