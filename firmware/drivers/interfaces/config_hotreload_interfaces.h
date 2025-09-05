#ifndef CONFIG_HOTRELOAD_INTERFACES_H
#define CONFIG_HOTRELOAD_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file config_hotreload_interfaces.h
 * @brief Runtime configuration hot-reload system
 * 
 * This system enables changing configuration parameters at runtime without
 * requiring a system restart. Supports parameter validation, rollback on
 * errors, and configuration versioning. Essential for sensor tuning and
 * testing different configurations.
 */

// =============================================================================
// Configuration Types and Metadata
// =============================================================================

typedef enum {
    CONFIG_TYPE_BOOL = 0,
    CONFIG_TYPE_INT8,
    CONFIG_TYPE_UINT8,
    CONFIG_TYPE_INT16,
    CONFIG_TYPE_UINT16,
    CONFIG_TYPE_INT32,
    CONFIG_TYPE_UINT32,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_DOUBLE,
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_ENUM,
    CONFIG_TYPE_BITMASK,
    CONFIG_TYPE_ARRAY,
    CONFIG_TYPE_STRUCT,
    CONFIG_TYPE_COUNT
} config_type_t;

typedef enum {
    CONFIG_SCOPE_SYSTEM = 0,          ///< System-wide configuration
    CONFIG_SCOPE_SENSOR,              ///< Sensor-specific configuration
    CONFIG_SCOPE_ALGORITHM,           ///< Algorithm parameters
    CONFIG_SCOPE_POWER,               ///< Power management
    CONFIG_SCOPE_BLE,                 ///< Bluetooth settings
    CONFIG_SCOPE_STORAGE,             ///< Storage settings
    CONFIG_SCOPE_USER,                ///< User preferences
    CONFIG_SCOPE_DEBUG,               ///< Debug settings
    CONFIG_SCOPE_COUNT
} config_scope_t;

typedef enum {
    CONFIG_ACCESS_READ_ONLY = 0,      ///< Cannot be changed at runtime
    CONFIG_ACCESS_READ_WRITE,         ///< Can be changed anytime
    CONFIG_ACCESS_WRITE_ONCE,         ///< Can only be set once
    CONFIG_ACCESS_ADMIN_ONLY,         ///< Requires admin privileges
    CONFIG_ACCESS_DEBUG_ONLY,         ///< Only available in debug builds
    CONFIG_ACCESS_COUNT
} config_access_t;

// =============================================================================
// Configuration Parameter Definition
// =============================================================================

/**
 * @brief Configuration parameter constraints
 */
typedef struct {
    union {
        struct { int32_t min, max; } int_range;
        struct { uint32_t min, max; } uint_range;
        struct { float min, max; } float_range;
        struct { uint32_t min_len, max_len; } string_length;
        struct { const char** values; uint32_t count; } enum_values;
        struct { uint32_t valid_bits; } bitmask;
    } constraint;
    
    const char* description;          ///< Human-readable constraint description
} config_constraint_t;

/**
 * @brief Configuration parameter metadata
 */
typedef struct {
    const char* name;                 ///< Parameter name (dot notation: "sensor.ppg.sample_rate")
    const char* description;          ///< Human-readable description
    const char* units;                ///< Units (e.g., "Hz", "mA", "°C")
    
    config_type_t type;               ///< Parameter data type
    config_scope_t scope;             ///< Configuration scope
    config_access_t access;           ///< Access permissions
    
    void* current_value;              ///< Pointer to current value
    void* default_value;              ///< Pointer to default value
    uint32_t value_size;              ///< Size of value in bytes
    
    config_constraint_t constraints;  ///< Value constraints
    
    // Callbacks
    bool (*validator)(const void* value); ///< Custom validation function
    bool (*on_change)(const void* old_value, const void* new_value); ///< Change callback
    
    // Metadata
    uint32_t change_count;            ///< Number of times changed
    uint32_t last_change_time;        ///< Last change timestamp
    bool requires_restart;            ///< Requires system restart to take effect
    bool is_persistent;               ///< Should be saved to storage
} config_parameter_t;

// =============================================================================
// Configuration Change Management
// =============================================================================

/**
 * @brief Configuration change record
 */
typedef struct {
    uint32_t change_id;               ///< Unique change identifier
    const char* parameter_name;       ///< Parameter that was changed
    void* old_value;                  ///< Previous value
    void* new_value;                  ///< New value
    uint32_t timestamp;               ///< Change timestamp
    const char* source;               ///< Source of change (user, auto, etc.)
    bool is_applied;                  ///< Whether change is currently applied
    bool is_persistent;               ///< Whether change was saved to storage
} config_change_record_t;

/**
 * @brief Configuration transaction
 */
typedef struct {
    uint32_t transaction_id;          ///< Unique transaction identifier
    config_change_record_t* changes;  ///< Array of changes in transaction
    uint32_t change_count;            ///< Number of changes
    uint32_t start_time;              ///< Transaction start time
    bool is_committed;                ///< Transaction committed
    bool is_rolled_back;              ///< Transaction rolled back
    const char* description;          ///< Transaction description
} config_transaction_t;

// =============================================================================
// Configuration Manager Interface
// =============================================================================

/**
 * @brief Configuration manager statistics
 */
typedef struct {
    uint32_t total_parameters;        ///< Total registered parameters
    uint32_t total_changes;           ///< Total configuration changes
    uint32_t successful_changes;      ///< Successful changes
    uint32_t failed_changes;          ///< Failed changes
    uint32_t rollbacks;               ///< Number of rollbacks
    uint32_t active_transactions;     ///< Currently active transactions
    uint32_t last_save_time;          ///< Last configuration save time
} config_manager_stats_t;

// =============================================================================
// Core Configuration Functions
// =============================================================================

/**
 * @brief Initialize configuration manager
 */
bool config_manager_init(void);

/**
 * @brief Register configuration parameter
 */
bool config_register_parameter(config_parameter_t* param);

/**
 * @brief Unregister configuration parameter
 */
bool config_unregister_parameter(const char* name);

/**
 * @brief Get parameter value
 */
bool config_get_value(const char* name, void* value, uint32_t* size);

/**
 * @brief Set parameter value (with validation)
 */
bool config_set_value(const char* name, const void* value, const char* source);

/**
 * @brief Get parameter metadata
 */
const config_parameter_t* config_get_parameter_info(const char* name);

/**
 * @brief List all parameters matching pattern
 */
int32_t config_list_parameters(const char* pattern, char* buffer, uint32_t buffer_size);

/**
 * @brief Reset parameter to default value
 */
bool config_reset_to_default(const char* name);

/**
 * @brief Reset all parameters in scope to defaults
 */
bool config_reset_scope_to_defaults(config_scope_t scope);

// =============================================================================
// Transaction Management
// =============================================================================

/**
 * @brief Begin configuration transaction
 */
uint32_t config_begin_transaction(const char* description);

/**
 * @brief Add change to current transaction
 */
bool config_transaction_set(uint32_t transaction_id, const char* name, const void* value);

/**
 * @brief Commit transaction (apply all changes)
 */
bool config_commit_transaction(uint32_t transaction_id);

/**
 * @brief Rollback transaction (undo all changes)
 */
bool config_rollback_transaction(uint32_t transaction_id);

/**
 * @brief Get transaction status
 */
bool config_get_transaction_status(uint32_t transaction_id, config_transaction_t* status);

// =============================================================================
// Configuration Persistence
// =============================================================================

/**
 * @brief Save current configuration to storage
 */
bool config_save_to_storage(void);

/**
 * @brief Load configuration from storage
 */
bool config_load_from_storage(void);

/**
 * @brief Export configuration to string (JSON format)
 */
bool config_export_to_string(char* buffer, uint32_t buffer_size, config_scope_t scope);

/**
 * @brief Import configuration from string (JSON format)
 */
bool config_import_from_string(const char* config_string, bool validate_only);

/**
 * @brief Create configuration backup
 */
bool config_create_backup(const char* backup_name);

/**
 * @brief Restore from configuration backup
 */
bool config_restore_backup(const char* backup_name);

/**
 * @brief List available backups
 */
int32_t config_list_backups(char* buffer, uint32_t buffer_size);

// =============================================================================
// Configuration Validation and Safety
// =============================================================================

/**
 * @brief Validate all current configuration parameters
 */
bool config_validate_all(char* error_buffer, uint32_t buffer_size);

/**
 * @brief Validate specific parameter value
 */
bool config_validate_parameter(const char* name, const void* value, char* error_message);

/**
 * @brief Check if configuration requires system restart
 */
bool config_requires_restart(void);

/**
 * @brief Get list of parameters requiring restart
 */
int32_t config_get_restart_required_list(char* buffer, uint32_t buffer_size);

/**
 * @brief Enable/disable safe mode (prevents dangerous changes)
 */
bool config_set_safe_mode(bool enable);

/**
 * @brief Check if parameter change is safe in current state
 */
bool config_is_change_safe(const char* name, const void* value);

// =============================================================================
// Runtime Configuration Monitoring
// =============================================================================

/**
 * @brief Configuration change callback
 */
typedef void (*config_change_callback_t)(const char* parameter_name, 
                                         const void* old_value, 
                                         const void* new_value);

/**
 * @brief Register for configuration change notifications
 */
bool config_register_change_callback(const char* pattern, config_change_callback_t callback);

/**
 * @brief Unregister configuration change callback
 */
bool config_unregister_change_callback(config_change_callback_t callback);

/**
 * @brief Get configuration change history
 */
int32_t config_get_change_history(config_change_record_t* records, 
                                 uint32_t max_records,
                                 uint32_t since_timestamp);

/**
 * @brief Get manager statistics
 */
bool config_get_manager_stats(config_manager_stats_t* stats);

// =============================================================================
// Hot-Reload Specific Functions
// =============================================================================

/**
 * @brief Enable/disable hot-reload for parameter
 */
bool config_set_hotreload_enabled(const char* name, bool enabled);

/**
 * @brief Trigger immediate configuration reload from storage
 */
bool config_hotreload_from_storage(void);

/**
 * @brief Apply configuration changes from external source (BLE, etc.)
 */
bool config_apply_external_changes(const char* config_json, const char* source);

/**
 * @brief Get current configuration version
 */
uint32_t config_get_version(void);

/**
 * @brief Set configuration version (for compatibility checking)
 */
bool config_set_version(uint32_t version);

// =============================================================================
// Predefined Configuration Helpers
// =============================================================================

/**
 * @brief Quick integer parameter registration
 */
#define CONFIG_REGISTER_INT(name, desc, units, scope, access, ptr, def_val, min_val, max_val) \
    config_register_int_param(name, desc, units, scope, access, ptr, def_val, min_val, max_val)

/**
 * @brief Quick float parameter registration
 */
#define CONFIG_REGISTER_FLOAT(name, desc, units, scope, access, ptr, def_val, min_val, max_val) \
    config_register_float_param(name, desc, units, scope, access, ptr, def_val, min_val, max_val)

/**
 * @brief Quick boolean parameter registration
 */
#define CONFIG_REGISTER_BOOL(name, desc, scope, access, ptr, def_val) \
    config_register_bool_param(name, desc, scope, access, ptr, def_val)

/**
 * @brief Quick enum parameter registration
 */
#define CONFIG_REGISTER_ENUM(name, desc, scope, access, ptr, def_val, enum_values, enum_count) \
    config_register_enum_param(name, desc, scope, access, ptr, def_val, enum_values, enum_count)

// Helper function prototypes
bool config_register_int_param(const char* name, const char* desc, const char* units,
                               config_scope_t scope, config_access_t access,
                               int32_t* ptr, int32_t default_val, int32_t min_val, int32_t max_val);

bool config_register_float_param(const char* name, const char* desc, const char* units,
                                 config_scope_t scope, config_access_t access,
                                 float* ptr, float default_val, float min_val, float max_val);

bool config_register_bool_param(const char* name, const char* desc,
                                config_scope_t scope, config_access_t access,
                                bool* ptr, bool default_val);

bool config_register_enum_param(const char* name, const char* desc,
                                config_scope_t scope, config_access_t access,
                                uint32_t* ptr, uint32_t default_val,
                                const char** enum_values, uint32_t enum_count);

#endif // CONFIG_HOTRELOAD_INTERFACES_H
