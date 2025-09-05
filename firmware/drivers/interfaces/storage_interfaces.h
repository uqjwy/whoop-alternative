#ifndef STORAGE_INTERFACES_H
#define STORAGE_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file storage_interfaces.h
 * @brief Unified storage abstraction for different storage types
 * 
 * This abstraction enables seamless switching between storage backends
 * (Flash, RAM, SD Card) and provides consistent data management across
 * the system. Supports both streaming data and configuration storage.
 */

// =============================================================================
// Storage Types and Properties
// =============================================================================

typedef enum {
    STORAGE_TYPE_VOLATILE = 0,        ///< RAM-based (lost on power cycle)
    STORAGE_TYPE_PERSISTENT,          ///< Flash/EEPROM (survives power cycle)
    STORAGE_TYPE_REMOVABLE,           ///< SD Card/USB (removable media)
    STORAGE_TYPE_EXTERNAL,            ///< External storage via BLE/WiFi
    STORAGE_TYPE_COUNT
} storage_type_t;

typedef enum {
    STORAGE_ACCESS_SEQUENTIAL = 0,    ///< Sequential access (streaming)
    STORAGE_ACCESS_RANDOM,            ///< Random access (file-like)
    STORAGE_ACCESS_RING_BUFFER,       ///< Circular buffer (FIFO)
    STORAGE_ACCESS_KEY_VALUE,         ///< Key-value pairs (config)
    STORAGE_ACCESS_COUNT
} storage_access_t;

typedef enum {
    STORAGE_PRIORITY_LOW = 0,         ///< Low priority data (can be lost)
    STORAGE_PRIORITY_MEDIUM,          ///< Medium priority (should persist)
    STORAGE_PRIORITY_HIGH,            ///< High priority (must persist)
    STORAGE_PRIORITY_CRITICAL,        ///< Critical data (system config)
    STORAGE_PRIORITY_COUNT
} storage_priority_t;

// =============================================================================
// Storage Information
// =============================================================================

/**
 * @brief Storage device capabilities
 */
typedef struct {
    storage_type_t type;              ///< Storage type
    storage_access_t access_patterns[4]; ///< Supported access patterns
    uint32_t total_size_bytes;        ///< Total storage capacity
    uint32_t free_size_bytes;         ///< Free space available
    uint32_t block_size;              ///< Minimum write block size
    uint32_t erase_size;              ///< Erase block size (for flash)
    
    // Performance characteristics
    uint32_t read_speed_bps;          ///< Read speed (bytes/second)
    uint32_t write_speed_bps;         ///< Write speed (bytes/second)
    uint32_t erase_time_ms;           ///< Erase time per block
    uint32_t wear_cycles;             ///< Maximum erase/write cycles
    
    // Power characteristics
    uint32_t idle_power_uw;           ///< Idle power consumption
    uint32_t active_power_uw;         ///< Active power consumption
    uint32_t sleep_power_uw;          ///< Sleep mode power consumption
    
    // Features
    bool supports_wear_leveling;      ///< Hardware wear leveling
    bool supports_encryption;         ///< Hardware encryption
    bool supports_compression;        ///< Hardware compression
    bool is_removable;                ///< Can be physically removed
} storage_info_t;

/**
 * @brief Storage usage statistics
 */
typedef struct {
    uint32_t bytes_written;           ///< Total bytes written
    uint32_t bytes_read;              ///< Total bytes read
    uint32_t write_cycles;            ///< Total write cycles
    uint32_t erase_cycles;            ///< Total erase cycles
    uint32_t errors;                  ///< Error count
    float fragmentation_percent;      ///< Storage fragmentation
    uint32_t uptime_s;                ///< Storage uptime
} storage_stats_t;

// =============================================================================
// Storage Operations Interface
// =============================================================================

/**
 * @brief Storage operations interface
 */
typedef struct {
    // Basic operations
    bool (*init)(void);
    bool (*deinit)(void);
    bool (*format)(void);
    bool (*get_info)(storage_info_t* info);
    bool (*get_stats)(storage_stats_t* stats);
    
    // Data operations
    int32_t (*read)(uint32_t offset, void* buffer, uint32_t size);
    int32_t (*write)(uint32_t offset, const void* data, uint32_t size);
    bool (*erase)(uint32_t offset, uint32_t size);
    bool (*sync)(void);
    
    // Advanced operations
    bool (*set_power_mode)(uint32_t mode);
    bool (*enable_compression)(bool enable);
    bool (*enable_encryption)(bool enable, const uint8_t* key);
    
    // Maintenance
    bool (*garbage_collect)(void);
    bool (*wear_level)(void);
    bool (*verify_integrity)(void);
} storage_ops_t;

// =============================================================================
// Storage Manager Interface
// =============================================================================

/**
 * @brief Storage device descriptor
 */
typedef struct {
    const char* name;                 ///< Device name
    storage_ops_t* ops;               ///< Operations interface
    storage_info_t info;              ///< Device information
    storage_stats_t stats;            ///< Usage statistics
    bool is_mounted;                  ///< Mount status
    uint32_t mount_time;              ///< Mount timestamp
} storage_device_t;

/**
 * @brief Data stream descriptor
 */
typedef struct {
    const char* name;                 ///< Stream name
    storage_device_t* device;         ///< Target storage device
    storage_access_t access_type;     ///< Access pattern
    storage_priority_t priority;      ///< Data priority
    
    // Stream configuration
    uint32_t buffer_size;             ///< Internal buffer size
    uint32_t chunk_size;              ///< Write chunk size
    uint32_t retention_time_s;        ///< Data retention time
    bool auto_sync;                   ///< Automatic sync to storage
    uint32_t sync_interval_ms;        ///< Sync interval
    
    // Stream state
    uint32_t write_offset;            ///< Current write position
    uint32_t read_offset;             ///< Current read position
    uint32_t data_available;          ///< Bytes available for reading
    bool is_open;                     ///< Stream open status
} data_stream_t;

// =============================================================================
// Sensor Data Storage
// =============================================================================

/**
 * @brief Sensor data header
 */
typedef struct {
    uint32_t magic;                   ///< Magic number for validation
    uint16_t version;                 ///< Data format version
    uint16_t sensor_type;             ///< Sensor type identifier
    uint32_t timestamp_start;         ///< Start timestamp
    uint32_t sample_rate;             ///< Sample rate in Hz
    uint32_t sample_count;            ///< Number of samples
    uint32_t data_size;               ///< Data size in bytes
    uint32_t checksum;                ///< Data checksum
} sensor_data_header_t;

/**
 * @brief Configuration storage
 */
typedef struct {
    char key[64];                     ///< Configuration key
    uint32_t value_size;              ///< Value size in bytes
    uint8_t value_type;               ///< Value type (string, int, float, etc.)
    uint32_t timestamp;               ///< Last update timestamp
    uint32_t checksum;                ///< Value checksum
} config_entry_t;

// =============================================================================
// High-Level Storage Management
// =============================================================================

/**
 * @brief Initialize storage manager
 */
bool storage_manager_init(void);

/**
 * @brief Register storage device
 */
bool storage_manager_register_device(storage_device_t* device);

/**
 * @brief Mount storage device
 */
bool storage_manager_mount(const char* device_name);

/**
 * @brief Unmount storage device
 */
bool storage_manager_unmount(const char* device_name);

/**
 * @brief Create data stream
 */
data_stream_t* storage_create_stream(const char* name, 
                                    const char* device_name,
                                    storage_access_t access_type,
                                    storage_priority_t priority);

/**
 * @brief Open existing data stream
 */
data_stream_t* storage_open_stream(const char* name);

/**
 * @brief Write data to stream
 */
int32_t storage_stream_write(data_stream_t* stream, const void* data, uint32_t size);

/**
 * @brief Read data from stream
 */
int32_t storage_stream_read(data_stream_t* stream, void* buffer, uint32_t size);

/**
 * @brief Close data stream
 */
bool storage_stream_close(data_stream_t* stream);

/**
 * @brief Sync stream to storage
 */
bool storage_stream_sync(data_stream_t* stream);

/**
 * @brief Get stream status
 */
bool storage_stream_get_status(data_stream_t* stream, uint32_t* available, uint32_t* free);

// =============================================================================
// Sensor Data Logging
// =============================================================================

/**
 * @brief Start sensor data logging
 */
bool storage_start_sensor_logging(const char* sensor_name, 
                                 uint32_t sample_rate,
                                 storage_priority_t priority);

/**
 * @brief Log sensor sample
 */
bool storage_log_sensor_sample(const char* sensor_name, 
                              const void* sample_data, 
                              uint32_t sample_size);

/**
 * @brief Stop sensor data logging
 */
bool storage_stop_sensor_logging(const char* sensor_name);

/**
 * @brief Read logged sensor data
 */
int32_t storage_read_sensor_data(const char* sensor_name, 
                                uint32_t start_time,
                                uint32_t duration_s,
                                void* buffer,
                                uint32_t buffer_size);

// =============================================================================
// Configuration Management
// =============================================================================

/**
 * @brief Store configuration value
 */
bool storage_config_set(const char* key, const void* value, uint32_t size, uint8_t type);

/**
 * @brief Get configuration value
 */
bool storage_config_get(const char* key, void* value, uint32_t* size, uint8_t* type);

/**
 * @brief Delete configuration key
 */
bool storage_config_delete(const char* key);

/**
 * @brief List configuration keys
 */
int32_t storage_config_list_keys(char* keys_buffer, uint32_t buffer_size);

/**
 * @brief Load configuration from file
 */
bool storage_config_load_from_file(const char* filename);

/**
 * @brief Save configuration to file
 */
bool storage_config_save_to_file(const char* filename);

// =============================================================================
// Storage Policy and Management
// =============================================================================

/**
 * @brief Storage policy configuration
 */
typedef struct {
    uint32_t max_storage_percent;     ///< Maximum storage usage (percent)
    uint32_t cleanup_threshold_percent; ///< Start cleanup when reached
    uint32_t retention_time_s;        ///< Default data retention time
    storage_priority_t min_priority;   ///< Minimum priority to store
    bool enable_compression;          ///< Enable data compression
    bool enable_wear_leveling;        ///< Enable wear leveling
    uint32_t gc_interval_s;           ///< Garbage collection interval
} storage_policy_t;

/**
 * @brief Set storage policy
 */
bool storage_set_policy(const storage_policy_t* policy);

/**
 * @brief Get current storage policy
 */
bool storage_get_policy(storage_policy_t* policy);

/**
 * @brief Trigger garbage collection
 */
bool storage_garbage_collect(void);

/**
 * @brief Get storage health status
 */
bool storage_get_health(float* wear_level, uint32_t* bad_blocks, bool* needs_maintenance);

// =============================================================================
// Predefined Storage Configurations
// =============================================================================

// High-frequency sensor data (PPG, IMU)
static const storage_policy_t STORAGE_POLICY_SENSOR_DATA = {
    .max_storage_percent = 80,        // Use up to 80% of storage
    .cleanup_threshold_percent = 70,  // Start cleanup at 70%
    .retention_time_s = 7 * 24 * 3600, // 7 days retention
    .min_priority = STORAGE_PRIORITY_MEDIUM,
    .enable_compression = true,       // Compress sensor data
    .enable_wear_leveling = true,     // Distribute writes
    .gc_interval_s = 3600            // GC every hour
};

// Configuration and calibration data
static const storage_policy_t STORAGE_POLICY_CONFIG_DATA = {
    .max_storage_percent = 5,         // Use max 5% for config
    .cleanup_threshold_percent = 90,  // Very conservative cleanup
    .retention_time_s = 365 * 24 * 3600, // 1 year retention
    .min_priority = STORAGE_PRIORITY_HIGH,
    .enable_compression = false,      // Don't compress config
    .enable_wear_leveling = true,     // Distribute writes
    .gc_interval_s = 24 * 3600       // GC daily
};

#endif // STORAGE_INTERFACES_H
