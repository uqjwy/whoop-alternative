#ifndef MAX30101_DRIVER_H
#define MAX30101_DRIVER_H

#include "../interfaces/sensor_interfaces.h"

/**
 * @file max30101_driver.h
 * @brief MAX30101 sensor driver implementing the unified PPG interface
 * 
 * This driver implements the ppg_sensor_ops_t interface to enable
 * sensor-agnostic operation. It can be easily swapped with other PPG
 * sensors like MAX86141 without changing the application code.
 */

// =============================================================================
// MAX30101 Register Definitions
// =============================================================================

// I2C Address
#define MAX30101_I2C_ADDRESS        0x57

// Register Map
#define MAX30101_INT_STATUS_1       0x00
#define MAX30101_INT_STATUS_2       0x01
#define MAX30101_INT_ENABLE_1       0x02
#define MAX30101_INT_ENABLE_2       0x03
#define MAX30101_FIFO_WR_PTR        0x04
#define MAX30101_OVF_COUNTER        0x05
#define MAX30101_FIFO_RD_PTR        0x06
#define MAX30101_FIFO_DATA          0x07
#define MAX30101_FIFO_CONFIG        0x08
#define MAX30101_MODE_CONFIG        0x09
#define MAX30101_SPO2_CONFIG        0x0A
#define MAX30101_LED1_PA            0x0C
#define MAX30101_LED2_PA            0x0D
#define MAX30101_PILOT_PA           0x10
#define MAX30101_MULTI_LED_CTRL1    0x11
#define MAX30101_MULTI_LED_CTRL2    0x12
#define MAX30101_TEMP_INT           0x1F
#define MAX30101_TEMP_FRAC          0x20
#define MAX30101_TEMP_CONFIG        0x21
#define MAX30101_PROX_INT_THRESH    0x30
#define MAX30101_REV_ID             0xFE
#define MAX30101_PART_ID            0xFF

// Configuration Values
#define MAX30101_PART_ID_VALUE      0x15
#define MAX30101_FIFO_SIZE          32

// =============================================================================
// Driver Functions
// =============================================================================

/**
 * @brief Initialize MAX30101 sensor
 * @param config PPG configuration structure
 * @return true if successful, false otherwise
 */
bool max30101_init(const ppg_config_t* config);

/**
 * @brief Start MAX30101 measurement
 * @return true if successful, false otherwise
 */
bool max30101_start(void);

/**
 * @brief Read samples from MAX30101 FIFO
 * @param samples Array to store samples
 * @param max_samples Maximum number of samples to read
 * @return Number of samples read
 */
int max30101_read_fifo(ppg_sample_t* samples, int max_samples);

/**
 * @brief Stop MAX30101 measurement
 * @return true if successful, false otherwise
 */
bool max30101_stop(void);

/**
 * @brief Reset MAX30101 sensor
 * @return true if successful, false otherwise
 */
bool max30101_reset(void);

/**
 * @brief Set MAX30101 configuration
 * @param config PPG configuration structure
 * @return true if successful, false otherwise
 */
bool max30101_set_config(const ppg_config_t* config);

/**
 * @brief Get MAX30101 status
 * @param status Pointer to store status byte
 * @return true if successful, false otherwise
 */
bool max30101_get_status(uint8_t* status);

/**
 * @brief Get FIFO sample count
 * @return Number of samples in FIFO, -1 on error
 */
int max30101_get_fifo_count(void);

/**
 * @brief Read temperature from MAX30101
 * @param temp Pointer to store temperature (0.01°C units)
 * @return true if successful, false otherwise
 */
bool max30101_read_temperature(int16_t* temp);

// =============================================================================
// Hardware Abstraction Functions (to be implemented by platform)
// =============================================================================

/**
 * @brief Read register from MAX30101
 * @param reg Register address
 * @param data Pointer to store data
 * @return true if successful, false otherwise
 */
bool max30101_read_reg(uint8_t reg, uint8_t* data);

/**
 * @brief Write register to MAX30101
 * @param reg Register address
 * @param data Data to write
 * @return true if successful, false otherwise
 */
bool max30101_write_reg(uint8_t reg, uint8_t data);

/**
 * @brief Read multiple bytes from MAX30101
 * @param reg Starting register address
 * @param data Buffer to store data
 * @param len Number of bytes to read
 * @return true if successful, false otherwise
 */
bool max30101_read_bytes(uint8_t reg, uint8_t* data, int len);

/**
 * @brief Delay function
 * @param ms Milliseconds to delay
 */
void max30101_delay_ms(int ms);

// =============================================================================
// Global Operations Structure
// =============================================================================

extern ppg_sensor_ops_t max30101_ops;

#endif // MAX30101_DRIVER_H
