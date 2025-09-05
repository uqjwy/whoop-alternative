#ifndef BLE_SERVICE_INTERFACES_H
#define BLE_SERVICE_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file ble_service_interfaces.h
 * @brief Generic BLE service abstraction for easy service management
 * 
 * This abstraction enables dynamic BLE service registration and management.
 * Makes it easy to add new services without modifying core BLE stack code.
 * Supports both standard and custom GATT services.
 */

// =============================================================================
// BLE Service Types and Properties
// =============================================================================

typedef enum {
    BLE_SERVICE_TYPE_STANDARD = 0,    ///< Standard Bluetooth SIG service
    BLE_SERVICE_TYPE_CUSTOM,          ///< Custom proprietary service
    BLE_SERVICE_TYPE_VENDOR,          ///< Vendor-specific service
    BLE_SERVICE_TYPE_COUNT
} ble_service_type_t;

typedef enum {
    BLE_CHAR_PROPERTY_READ = 0x01,    ///< Characteristic supports read
    BLE_CHAR_PROPERTY_WRITE = 0x02,   ///< Characteristic supports write
    BLE_CHAR_PROPERTY_NOTIFY = 0x04,  ///< Characteristic supports notify
    BLE_CHAR_PROPERTY_INDICATE = 0x08, ///< Characteristic supports indicate
    BLE_CHAR_PROPERTY_BROADCAST = 0x10, ///< Characteristic supports broadcast
    BLE_CHAR_PROPERTY_AUTH = 0x20,    ///< Requires authentication
    BLE_CHAR_PROPERTY_ENCRYPT = 0x40  ///< Requires encryption
} ble_char_property_t;

typedef enum {
    BLE_DATA_TYPE_UINT8 = 0,
    BLE_DATA_TYPE_UINT16,
    BLE_DATA_TYPE_UINT32,
    BLE_DATA_TYPE_INT8,
    BLE_DATA_TYPE_INT16,
    BLE_DATA_TYPE_INT32,
    BLE_DATA_TYPE_FLOAT,
    BLE_DATA_TYPE_STRING,
    BLE_DATA_TYPE_BYTES,
    BLE_DATA_TYPE_STRUCT,
    BLE_DATA_TYPE_COUNT
} ble_data_type_t;

// =============================================================================
// BLE Characteristic Definition
// =============================================================================

/**
 * @brief BLE characteristic value
 */
typedef struct {
    ble_data_type_t type;             ///< Data type
    uint32_t max_length;              ///< Maximum data length
    uint32_t current_length;          ///< Current data length
    void* data;                       ///< Pointer to data
    uint32_t timestamp;               ///< Last update timestamp
} ble_char_value_t;

/**
 * @brief BLE characteristic callbacks
 */
typedef struct {
    bool (*on_read)(ble_char_value_t* value, uint32_t* length);
    bool (*on_write)(const ble_char_value_t* value);
    bool (*on_subscribe)(bool enable_notifications);
    bool (*on_unsubscribe)(void);
} ble_char_callbacks_t;

/**
 * @brief BLE characteristic descriptor
 */
typedef struct {
    uint16_t uuid16;                  ///< 16-bit UUID (if standard)
    uint8_t uuid128[16];              ///< 128-bit UUID (if custom)
    const char* name;                 ///< Human-readable name
    const char* description;          ///< Characteristic description
    
    uint32_t properties;              ///< Property flags (ble_char_property_t)
    ble_char_value_t value;           ///< Current value
    ble_char_callbacks_t callbacks;   ///< Event callbacks
    
    // Metadata
    bool is_uuid16;                   ///< True if using 16-bit UUID
    uint32_t notification_interval_ms; ///< Notification interval
    bool enable_caching;              ///< Enable value caching
    uint32_t last_notification_time;  ///< Last notification timestamp
} ble_characteristic_t;

// =============================================================================
// BLE Service Definition
// =============================================================================

/**
 * @brief BLE service callbacks
 */
typedef struct {
    bool (*on_connect)(uint16_t conn_handle);
    bool (*on_disconnect)(uint16_t conn_handle);
    bool (*on_subscribe)(uint16_t conn_handle, uint16_t char_handle);
    bool (*on_unsubscribe)(uint16_t conn_handle, uint16_t char_handle);
    bool (*on_mtu_changed)(uint16_t conn_handle, uint16_t mtu);
} ble_service_callbacks_t;

/**
 * @brief BLE service descriptor
 */
typedef struct {
    uint16_t uuid16;                  ///< 16-bit service UUID (if standard)
    uint8_t uuid128[16];              ///< 128-bit service UUID (if custom)
    const char* name;                 ///< Human-readable service name
    const char* description;          ///< Service description
    
    ble_service_type_t type;          ///< Service type
    bool is_primary;                  ///< Primary or secondary service
    bool is_uuid16;                   ///< True if using 16-bit UUID
    
    // Characteristics
    ble_characteristic_t* characteristics; ///< Array of characteristics
    uint32_t char_count;              ///< Number of characteristics
    
    // Service configuration
    uint32_t max_connections;         ///< Maximum concurrent connections
    uint32_t notification_quota;      ///< Notifications per connection interval
    bool require_encryption;          ///< Require encrypted connection
    bool require_authentication;      ///< Require authenticated connection
    
    // Callbacks
    ble_service_callbacks_t callbacks; ///< Service event callbacks
    
    // Runtime state
    bool is_registered;               ///< Service registration status
    uint16_t service_handle;          ///< GATT service handle
    uint32_t active_connections;      ///< Number of active connections
    uint32_t total_notifications;     ///< Total notifications sent
} ble_service_t;

// =============================================================================
// BLE Connection Management
// =============================================================================

/**
 * @brief BLE connection information
 */
typedef struct {
    uint16_t conn_handle;             ///< Connection handle
    uint8_t peer_addr[6];             ///< Peer device address
    uint8_t addr_type;                ///< Address type (public/random)
    uint16_t mtu;                     ///< Maximum transmission unit
    uint16_t interval;                ///< Connection interval
    uint16_t latency;                 ///< Slave latency
    uint16_t timeout;                 ///< Supervision timeout
    
    // Connection state
    bool is_encrypted;                ///< Connection encryption status
    bool is_authenticated;            ///< Connection authentication status
    uint32_t connect_time;            ///< Connection establishment time
    uint32_t last_activity_time;      ///< Last activity timestamp
    
    // Statistics
    uint32_t bytes_sent;              ///< Total bytes sent
    uint32_t bytes_received;          ///< Total bytes received
    uint32_t notifications_sent;      ///< Total notifications sent
    uint32_t write_requests;          ///< Total write requests received
} ble_connection_t;

// =============================================================================
// BLE Service Manager
// =============================================================================

/**
 * @brief BLE advertising configuration
 */
typedef struct {
    const char* device_name;          ///< Advertised device name
    uint16_t appearance;              ///< Device appearance
    uint16_t* service_uuids;          ///< Array of service UUIDs to advertise
    uint32_t service_uuid_count;      ///< Number of service UUIDs
    
    uint32_t adv_interval_min_ms;     ///< Minimum advertising interval
    uint32_t adv_interval_max_ms;     ///< Maximum advertising interval
    int8_t tx_power_dbm;              ///< Transmit power
    bool connectable;                 ///< Allow connections
    bool discoverable;                ///< Allow discovery
    uint32_t timeout_s;               ///< Advertising timeout
} ble_advertising_config_t;

/**
 * @brief BLE manager statistics
 */
typedef struct {
    uint32_t total_connections;       ///< Total connections since boot
    uint32_t active_connections;      ///< Currently active connections
    uint32_t advertising_time_s;      ///< Total time spent advertising
    uint32_t connected_time_s;        ///< Total time spent connected
    uint32_t notifications_sent;      ///< Total notifications sent
    uint32_t bytes_transferred;       ///< Total bytes transferred
    uint32_t connection_errors;       ///< Connection error count
} ble_manager_stats_t;

// =============================================================================
// BLE Service Manager Functions
// =============================================================================

/**
 * @brief Initialize BLE service manager
 */
bool ble_manager_init(void);

/**
 * @brief Register BLE service
 */
bool ble_manager_register_service(ble_service_t* service);

/**
 * @brief Unregister BLE service
 */
bool ble_manager_unregister_service(const char* service_name);

/**
 * @brief Start BLE advertising
 */
bool ble_manager_start_advertising(const ble_advertising_config_t* config);

/**
 * @brief Stop BLE advertising
 */
bool ble_manager_stop_advertising(void);

/**
 * @brief Update characteristic value
 */
bool ble_manager_update_characteristic(const char* service_name,
                                      const char* char_name,
                                      const void* value,
                                      uint32_t length);

/**
 * @brief Send notification
 */
bool ble_manager_notify_characteristic(const char* service_name,
                                      const char* char_name,
                                      uint16_t conn_handle);

/**
 * @brief Send indication
 */
bool ble_manager_indicate_characteristic(const char* service_name,
                                        const char* char_name,
                                        uint16_t conn_handle);

/**
 * @brief Get connection information
 */
bool ble_manager_get_connection_info(uint16_t conn_handle, ble_connection_t* info);

/**
 * @brief Disconnect specific connection
 */
bool ble_manager_disconnect(uint16_t conn_handle);

/**
 * @brief Get manager statistics
 */
bool ble_manager_get_stats(ble_manager_stats_t* stats);

/**
 * @brief Set connection parameters
 */
bool ble_manager_set_connection_params(uint16_t conn_handle,
                                      uint16_t min_interval,
                                      uint16_t max_interval,
                                      uint16_t latency,
                                      uint16_t timeout);

// =============================================================================
// Predefined Standard Services
// =============================================================================

/**
 * @brief Create Heart Rate Service
 */
ble_service_t* ble_create_heart_rate_service(void);

/**
 * @brief Create Battery Service
 */
ble_service_t* ble_create_battery_service(void);

/**
 * @brief Create Device Information Service
 */
ble_service_t* ble_create_device_info_service(const char* manufacturer,
                                             const char* model,
                                             const char* serial,
                                             const char* fw_version);

// =============================================================================
// Custom Health Services
// =============================================================================

/**
 * @brief Create Vitals Service (HRV, SpO2, Temperature, Respiratory Rate)
 */
ble_service_t* ble_create_vitals_service(void);

/**
 * @brief Create Health Monitor Service (Illness detection, baselines)
 */
ble_service_t* ble_create_health_monitor_service(void);

/**
 * @brief Create Data Sync Service (Historical data download)
 */
ble_service_t* ble_create_data_sync_service(void);

/**
 * @brief Create Configuration Service (Runtime configuration)
 */
ble_service_t* ble_create_config_service(void);

// =============================================================================
// Service Helper Functions
// =============================================================================

/**
 * @brief Add characteristic to service
 */
bool ble_service_add_characteristic(ble_service_t* service, ble_characteristic_t* characteristic);

/**
 * @brief Remove characteristic from service
 */
bool ble_service_remove_characteristic(ble_service_t* service, const char* char_name);

/**
 * @brief Find characteristic by name
 */
ble_characteristic_t* ble_service_find_characteristic(ble_service_t* service, const char* char_name);

/**
 * @brief Enable/disable characteristic notifications
 */
bool ble_service_set_notifications(ble_service_t* service, 
                                  const char* char_name, 
                                  bool enable);

/**
 * @brief Set characteristic notification interval
 */
bool ble_service_set_notification_interval(ble_service_t* service,
                                          const char* char_name,
                                          uint32_t interval_ms);

/**
 * @brief Validate service configuration
 */
bool ble_service_validate(const ble_service_t* service);

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * @brief Convert UUID string to binary
 */
bool ble_uuid_from_string(const char* uuid_str, uint8_t* uuid128);

/**
 * @brief Convert binary UUID to string
 */
bool ble_uuid_to_string(const uint8_t* uuid128, char* uuid_str, uint32_t str_len);

/**
 * @brief Get standard service UUID by name
 */
bool ble_get_standard_service_uuid(const char* service_name, uint16_t* uuid16);

/**
 * @brief Get standard characteristic UUID by name
 */
bool ble_get_standard_char_uuid(const char* char_name, uint16_t* uuid16);

/**
 * @brief Calculate optimal notification interval
 */
uint32_t ble_calculate_optimal_notification_interval(uint32_t data_rate_bps,
                                                    uint16_t mtu,
                                                    uint16_t conn_interval);

#endif // BLE_SERVICE_INTERFACES_H
