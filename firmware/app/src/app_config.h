/*
 * Application Configuration
 * 
 * Central configuration file for the Whoop Alternative firmware
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Application version */
#define APP_VERSION "0.1.0-dev"

/* Hardware configuration */
#define BOARD_NRF52840DK    1

/* Sensor sampling rates */
#define PPG_SAMPLE_RATE_REST_HZ     50
#define PPG_SAMPLE_RATE_ACTIVE_HZ   25
#define IMU_SAMPLE_RATE_HZ          25
#define TEMP_SAMPLE_RATE_NIGHT_S    60      /* 1/min */
#define TEMP_SAMPLE_RATE_DAY_S      600     /* 1/10min */

/* PPG configuration */
#define PPG_LED_CURRENT_MIN_MA      5
#define PPG_LED_CURRENT_MAX_MA      50
#define PPG_FILTER_LOW_HZ           0.7f
#define PPG_FILTER_HIGH_HZ          4.0f

/* IMU configuration */
#define IMU_RANGE_G                 8
#define IMU_ACTIVITY_THRESHOLD      1000    /* mg */

/* HRV configuration */
#define HRV_MIN_RR_INTERVALS        50
#define HRV_MAX_ARTIFACT_PERCENT    20
#define HRV_BASELINE_DAYS           30

/* Power management */
#define BATTERY_LOW_THRESHOLD_MV    3300
#define BATTERY_CRITICAL_MV         3200
#define SLEEP_TIMEOUT_S             300     /* 5 min inactivity */

/* BLE configuration */
#define BLE_DEVICE_NAME             "WhoopAlt"
#define BLE_CONN_INTERVAL_MIN_MS    400
#define BLE_CONN_INTERVAL_MAX_MS    1000
#define BLE_DATA_CHUNK_SIZE         512

/* Storage configuration */
#define FLASH_LOG_SIZE_MB           6       /* 6MB for sensor data */
#define FLASH_CONFIG_SIZE_KB        64      /* 64KB for configuration */
#define LOG_RETENTION_DAYS          30

/* Health monitoring */
#define HEALTH_HR_INCREASE_BPM      5
#define HEALTH_HRV_DECREASE_PERCENT 20
#define HEALTH_TEMP_INCREASE_C      0.3f
#define HEALTH_RESP_INCREASE_BPM    1
#define HEALTH_MIN_CRITERIA         2       /* Min criteria for alert */
#define HEALTH_MIN_NIGHTS           2       /* Min consecutive nights */

/* Debugging */
#define DEBUG_ENABLE_RTT            1
#define DEBUG_ENABLE_UART           0
#define DEBUG_LOG_LEVEL             3       /* 0=OFF, 1=ERR, 2=WRN, 3=INF, 4=DBG */

#endif /* APP_CONFIG_H */
