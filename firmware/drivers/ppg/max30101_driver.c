/*
 * MAX30101 PPG Sensor Driver Implementation
 * 
 * Implementation of the PPG sensor interface for MAX30101
 * Integrated PPG sensor with Red, IR LEDs and photodiode
 */

#include "max30101_driver.h"
#include "../interfaces/sensor_interfaces.h"
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(max30101_driver, LOG_LEVEL_DBG);

/* ==== MAX30101 REGISTER DEFINITIONS ==== */

#define MAX30101_I2C_ADDR           0x57

/* Status Registers */
#define MAX30101_REG_INT_STATUS_1   0x00
#define MAX30101_REG_INT_STATUS_2   0x01
#define MAX30101_REG_INT_ENABLE_1   0x02
#define MAX30101_REG_INT_ENABLE_2   0x03

/* FIFO Registers */
#define MAX30101_REG_FIFO_WR_PTR    0x04
#define MAX30101_REG_FIFO_OVF_CNT   0x05
#define MAX30101_REG_FIFO_RD_PTR    0x06
#define MAX30101_REG_FIFO_DATA      0x07

/* Configuration Registers */
#define MAX30101_REG_FIFO_CONFIG    0x08
#define MAX30101_REG_MODE_CONFIG    0x09
#define MAX30101_REG_SPO2_CONFIG    0x0A
#define MAX30101_REG_LED1_PA        0x0C  // Red LED
#define MAX30101_REG_LED2_PA        0x0D  // IR LED
#define MAX30101_REG_PILOT_PA       0x10
#define MAX30101_REG_MULTI_LED_1    0x11
#define MAX30101_REG_MULTI_LED_2    0x12

/* Temperature Registers */
#define MAX30101_REG_TEMP_INT       0x1F
#define MAX30101_REG_TEMP_FRAC      0x20
#define MAX30101_REG_TEMP_CONFIG    0x21

/* Part ID */
#define MAX30101_REG_REV_ID         0xFE
#define MAX30101_REG_PART_ID        0xFF
#define MAX30101_EXPECTED_PART_ID   0x15

/* Configuration Values */
#define MAX30101_MODE_HEART_RATE    0x02
#define MAX30101_MODE_SPO2          0x03
#define MAX30101_MODE_MULTI_LED     0x07

/* Sample Rate Configuration */
#define MAX30101_SR_50HZ            0x00
#define MAX30101_SR_100HZ           0x01
#define MAX30101_SR_200HZ           0x02
#define MAX30101_SR_400HZ           0x03
#define MAX30101_SR_800HZ           0x04
#define MAX30101_SR_1000HZ          0x05
#define MAX30101_SR_1600HZ          0x06
#define MAX30101_SR_3200HZ          0x07

/* Pulse Width Configuration */
#define MAX30101_PW_69US            0x00  // 15-bit resolution
#define MAX30101_PW_118US           0x01  // 16-bit resolution
#define MAX30101_PW_215US           0x02  // 17-bit resolution
#define MAX30101_PW_411US           0x03  // 18-bit resolution

/* ADC Range Configuration */
#define MAX30101_ADC_RANGE_2048     0x00
#define MAX30101_ADC_RANGE_4096     0x01
#define MAX30101_ADC_RANGE_8192     0x02
#define MAX30101_ADC_RANGE_16384    0x03

/* ==== PRIVATE DATA STRUCTURE ==== */

typedef struct {
    const struct device* i2c_dev;
    struct k_work_delayable temp_work;
    ppg_config_t current_config;
    uint32_t last_timestamp;
    bool temp_measurement_active;
    int16_t last_temperature;
} max30101_data_t;

static max30101_data_t max30101_data;

/* ==== HELPER FUNCTIONS ==== */

static int max30101_i2c_read_reg(uint8_t reg, uint8_t* data, size_t len)
{
    return i2c_burst_read(max30101_data.i2c_dev, MAX30101_I2C_ADDR, reg, data, len);
}

static int max30101_i2c_write_reg(uint8_t reg, uint8_t data)
{
    return i2c_reg_write_byte(max30101_data.i2c_dev, MAX30101_I2C_ADDR, reg, data);
}

static uint8_t max30101_sample_rate_to_reg(int sample_rate)
{
    switch (sample_rate) {
        case 50: return MAX30101_SR_50HZ;
        case 100: return MAX30101_SR_100HZ;
        case 200: return MAX30101_SR_200HZ;
        case 400: return MAX30101_SR_400HZ;
        case 800: return MAX30101_SR_800HZ;
        case 1000: return MAX30101_SR_1000HZ;
        case 1600: return MAX30101_SR_1600HZ;
        case 3200: return MAX30101_SR_3200HZ;
        default: return MAX30101_SR_100HZ;
    }
}

static uint8_t max30101_pulse_width_to_reg(int pulse_width_us)
{
    if (pulse_width_us <= 69) return MAX30101_PW_69US;
    else if (pulse_width_us <= 118) return MAX30101_PW_118US;
    else if (pulse_width_us <= 215) return MAX30101_PW_215US;
    else return MAX30101_PW_411US;
}

static uint8_t max30101_adc_range_to_reg(int adc_range)
{
    switch (adc_range) {
        case 2048: return MAX30101_ADC_RANGE_2048;
        case 4096: return MAX30101_ADC_RANGE_4096;
        case 8192: return MAX30101_ADC_RANGE_8192;
        case 16384: return MAX30101_ADC_RANGE_16384;
        default: return MAX30101_ADC_RANGE_4096;
    }
}

/* ==== TEMPERATURE MEASUREMENT ==== */

static void max30101_temp_work_handler(struct k_work* work)
{
    uint8_t temp_int, temp_frac;
    
    if (max30101_i2c_read_reg(MAX30101_REG_TEMP_INT, &temp_int, 1) == 0 &&
        max30101_i2c_read_reg(MAX30101_REG_TEMP_FRAC, &temp_frac, 1) == 0) {
        
        // Convert to 0.01°C resolution
        max30101_data.last_temperature = (int16_t)(temp_int * 100 + (temp_frac & 0x0F) * 625 / 100);
    }
    
    max30101_data.temp_measurement_active = false;
}

/* ==== PPG SENSOR INTERFACE IMPLEMENTATION ==== */

static bool max30101_init(const ppg_config_t* config)
{
    uint8_t part_id;
    
    LOG_INF("Initializing MAX30101 PPG sensor");
    
    max30101_data.i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (!device_is_ready(max30101_data.i2c_dev)) {
        LOG_ERR("I2C device not ready");
        return false;
    }
    
    // Verify part ID
    if (max30101_i2c_read_reg(MAX30101_REG_PART_ID, &part_id, 1) != 0) {
        LOG_ERR("Failed to read part ID");
        return false;
    }
    
    if (part_id != MAX30101_EXPECTED_PART_ID) {
        LOG_ERR("Invalid part ID: 0x%02X (expected 0x%02X)", part_id, MAX30101_EXPECTED_PART_ID);
        return false;
    }
    
    // Reset device
    max30101_i2c_write_reg(MAX30101_REG_MODE_CONFIG, 0x40);
    k_msleep(100);
    
    // Configure FIFO
    uint8_t fifo_config = (config->avg_samples - 1) << 5;
    if (config->fifo_enable) {
        fifo_config |= 0x10;  // FIFO rollover enable
    }
    max30101_i2c_write_reg(MAX30101_REG_FIFO_CONFIG, fifo_config);
    
    // Configure SpO2/HR mode
    uint8_t spo2_config = max30101_sample_rate_to_reg(config->sample_rate) << 2;
    spo2_config |= max30101_pulse_width_to_reg(config->pulse_width);
    max30101_i2c_write_reg(MAX30101_REG_SPO2_CONFIG, spo2_config);
    
    // Set LED currents
    max30101_i2c_write_reg(MAX30101_REG_LED1_PA, config->led_current[0]);  // Red
    max30101_i2c_write_reg(MAX30101_REG_LED2_PA, config->led_current[1]);  // IR
    
    // Configure interrupts
    max30101_i2c_write_reg(MAX30101_REG_INT_ENABLE_1, 0x40);  // FIFO almost full
    if (config->temp_enable) {
        max30101_i2c_write_reg(MAX30101_REG_INT_ENABLE_2, 0x02);  // Temperature ready
    }
    
    // Initialize work queue for temperature
    k_work_init_delayable(&max30101_data.temp_work, max30101_temp_work_handler);
    
    max30101_data.current_config = *config;
    max30101_data.last_timestamp = 0;
    max30101_data.temp_measurement_active = false;
    max30101_data.last_temperature = 2500;  // 25.00°C default
    
    LOG_INF("MAX30101 initialized successfully");
    return true;
}

static bool max30101_start(void)
{
    LOG_INF("Starting MAX30101 measurement");
    
    // Clear FIFO pointers
    max30101_i2c_write_reg(MAX30101_REG_FIFO_WR_PTR, 0x00);
    max30101_i2c_write_reg(MAX30101_REG_FIFO_OVF_CNT, 0x00);
    max30101_i2c_write_reg(MAX30101_REG_FIFO_RD_PTR, 0x00);
    
    // Start in SpO2 mode (Red + IR)
    max30101_i2c_write_reg(MAX30101_REG_MODE_CONFIG, MAX30101_MODE_SPO2);
    
    // Start temperature measurement if enabled
    if (max30101_data.current_config.temp_enable) {
        max30101_i2c_write_reg(MAX30101_REG_TEMP_CONFIG, 0x01);
        max30101_data.temp_measurement_active = true;
        k_work_schedule(&max30101_data.temp_work, K_MSEC(100));
    }
    
    max30101_data.last_timestamp = k_uptime_get_32();
    
    return true;
}

static int max30101_read_fifo(ppg_sample_t* samples, int max_samples)
{
    uint8_t fifo_data[6];  // 3 bytes per channel, 2 channels
    uint8_t wr_ptr, rd_ptr;
    int num_samples = 0;
    uint32_t timestamp = k_uptime_get_32();
    
    // Get FIFO pointers
    if (max30101_i2c_read_reg(MAX30101_REG_FIFO_WR_PTR, &wr_ptr, 1) != 0 ||
        max30101_i2c_read_reg(MAX30101_REG_FIFO_RD_PTR, &rd_ptr, 1) != 0) {
        return 0;
    }
    
    // Calculate number of samples available
    int available_samples = (wr_ptr - rd_ptr) & 0x1F;
    int samples_to_read = MIN(available_samples, max_samples);
    
    for (int i = 0; i < samples_to_read; i++) {
        if (max30101_i2c_read_reg(MAX30101_REG_FIFO_DATA, fifo_data, 6) != 0) {
            break;
        }
        
        // Parse FIFO data (18-bit values, MSB first)
        uint32_t red_raw = ((uint32_t)fifo_data[0] << 16) | 
                           ((uint32_t)fifo_data[1] << 8) | 
                           fifo_data[2];
        red_raw &= 0x3FFFF;  // 18-bit mask
        
        uint32_t ir_raw = ((uint32_t)fifo_data[3] << 16) | 
                          ((uint32_t)fifo_data[4] << 8) | 
                          fifo_data[5];
        ir_raw &= 0x3FFFF;   // 18-bit mask
        
        // Fill sample structure
        samples[i].timestamp = timestamp - (samples_to_read - i - 1) * 
                               (1000 / max30101_data.current_config.sample_rate);
        samples[i].channels[0] = (int32_t)red_raw;   // Red channel
        samples[i].channels[1] = (int32_t)ir_raw;    // IR channel
        samples[i].channels[2] = 0;                  // Green channel (not available)
        samples[i].channels[3] = 0;                  // UV channel (not available)
        samples[i].led_slots = 0x03;                 // Red + IR active
        samples[i].temperature = max30101_data.last_temperature;
        
        // Simple signal quality based on amplitude
        uint32_t amplitude = (red_raw > ir_raw) ? red_raw - ir_raw : ir_raw - red_raw;
        samples[i].quality = (uint8_t)MIN(100, amplitude / 1000);
        
        num_samples++;
    }
    
    max30101_data.last_timestamp = timestamp;
    return num_samples;
}

static bool max30101_stop(void)
{
    LOG_INF("Stopping MAX30101 measurement");
    
    // Enter shutdown mode
    max30101_i2c_write_reg(MAX30101_REG_MODE_CONFIG, 0x80);
    
    // Cancel temperature work
    k_work_cancel_delayable(&max30101_data.temp_work);
    max30101_data.temp_measurement_active = false;
    
    return true;
}

static bool max30101_reset(void)
{
    LOG_INF("Resetting MAX30101");
    
    // Software reset
    max30101_i2c_write_reg(MAX30101_REG_MODE_CONFIG, 0x40);
    k_msleep(100);
    
    return true;
}

static bool max30101_set_led_current(int led_idx, int current_ma)
{
    uint8_t reg;
    
    switch (led_idx) {
        case 0: reg = MAX30101_REG_LED1_PA; break;  // Red
        case 1: reg = MAX30101_REG_LED2_PA; break;  // IR
        default: return false;
    }
    
    max30101_data.current_config.led_current[led_idx] = current_ma;
    return max30101_i2c_write_reg(reg, (uint8_t)current_ma) == 0;
}

static bool max30101_get_temperature(int16_t* temp)
{
    if (!max30101_data.current_config.temp_enable) {
        return false;
    }
    
    *temp = max30101_data.last_temperature;
    return true;
}

static int max30101_get_fifo_count(void)
{
    uint8_t wr_ptr, rd_ptr;
    
    if (max30101_i2c_read_reg(MAX30101_REG_FIFO_WR_PTR, &wr_ptr, 1) != 0 ||
        max30101_i2c_read_reg(MAX30101_REG_FIFO_RD_PTR, &rd_ptr, 1) != 0) {
        return 0;
    }
    
    return (wr_ptr - rd_ptr) & 0x1F;
}

static bool max30101_configure_interrupts(uint32_t int_mask)
{
    uint8_t int_enable_1 = 0;
    uint8_t int_enable_2 = 0;
    
    if (int_mask & 0x01) int_enable_1 |= 0x40;  // FIFO almost full
    if (int_mask & 0x02) int_enable_1 |= 0x20;  // New FIFO data ready
    if (int_mask & 0x04) int_enable_1 |= 0x10;  // Ambient light cancellation overflow
    if (int_mask & 0x08) int_enable_2 |= 0x02;  // Temperature ready
    
    return (max30101_i2c_write_reg(MAX30101_REG_INT_ENABLE_1, int_enable_1) == 0) &&
           (max30101_i2c_write_reg(MAX30101_REG_INT_ENABLE_2, int_enable_2) == 0);
}

static const char* max30101_get_device_info(void)
{
    return "Maxim MAX30101 Integrated PPG Sensor (Red + IR LEDs)";
}

/* ==== PPG SENSOR OPERATIONS STRUCTURE ==== */

const ppg_sensor_ops_t max30101_ops = {
    .init = max30101_init,
    .start = max30101_start,
    .read_fifo = max30101_read_fifo,
    .stop = max30101_stop,
    .reset = max30101_reset,
    .set_led_current = max30101_set_led_current,
    .get_temperature = max30101_get_temperature,
    .get_fifo_count = max30101_get_fifo_count,
    .configure_interrupts = max30101_configure_interrupts,
    .get_device_info = max30101_get_device_info
};
