/*
 * Application State Management
 * 
 * Handles state machine for the Whoop Alternative device
 */

#ifndef APP_STATES_H
#define APP_STATES_H

#include <stdint.h>

/* Application states */
typedef enum {
    APP_STATE_INIT = 0,         /* Initial startup */
    APP_STATE_MEASURING,        /* Normal measurement mode */
    APP_STATE_SLEEP,           /* Sleep mode (reduced sampling) */
    APP_STATE_CHARGING,        /* Device is charging */
    APP_STATE_ADVERTISING,     /* BLE advertising */
    APP_STATE_CONNECTED,       /* BLE connected */
    APP_STATE_SYNCING,         /* Data synchronization */
    APP_STATE_ERROR,           /* Error state */
    APP_STATE_SHUTDOWN         /* Shutting down */
} app_state_t;

/* State transition functions */
int app_state_init(void);
int app_state_set(app_state_t new_state);
app_state_t app_state_get(void);
const char* app_state_to_string(app_state_t state);

/* State handlers */
int handle_measuring_state(void);
int handle_sleep_state(void);
int handle_charging_state(void);
int handle_ble_advertising_state(void);
int handle_ble_connected_state(void);
int handle_sync_state(void);
int handle_error_state(void);

#endif /* APP_STATES_H */
