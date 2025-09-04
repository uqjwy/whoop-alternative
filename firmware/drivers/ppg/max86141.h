/*
 * MAX86141 PPG Sensor Driver
 * 
 * Driver for Maxim MAX86141 PPG (Photoplethysmography) sensor
 * Features: Heart rate, SpO2, ambient light cancellation
 */

#ifndef MAX86141_H
#define MAX86141_H

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <stdint.h>

/* I2C Address */
#define MAX86141_I2C_ADDR           0x5E

/* Register Map */
#define MAX86141_REG_STATUS         0x00
#define MAX86141_REG_INT_ENABLE     0x01
#define MAX86141_REG_FIFO_WR_PTR    0x04
#define MAX86141_REG_FIFO_RD_PTR    0x05
#define MAX86141_REG_FIFO_CNT       0x06
#define MAX86141_REG_FIFO_DATA      0x07
#define MAX86141_REG_MODE_CONFIG    0x08
#define MAX86141_REG_SPO2_CONFIG    0x09
#define MAX86141_REG_LED1_PA        0x0A
#define MAX86141_REG_LED2_PA        0x0B
#define MAX86141_REG_PILOT_PA       0x0C
#define MAX86141_REG_MULTI_LED1     0x11
#define MAX86141_REG_MULTI_LED2     0x12
#define MAX86141_REG_TEMP_INT       0x1F
#define MAX86141_REG_TEMP_FRAC      0x20
#define MAX86141_REG_TEMP_CONFIG    0x21
#define MAX86141_REG_REV_ID         0xFE
#define MAX86141_REG_PART_ID        0xFF

/* Configuration values */
#define MAX86141_PART_ID            0x15
#define MAX86141_MODE_HR            0x02
#define MAX86141_MODE_SPO2          0x03
#define MAX86141_MODE_MULTI_LED     0x07

/* Sample rates */
#define MAX86141_SR_50HZ            0x00
#define MAX86141_SR_100HZ           0x01
#define MAX86141_SR_200HZ           0x02
#define MAX86141_SR_400HZ           0x03
#define MAX86141_SR_800HZ           0x04
#define MAX86141_SR_1000HZ          0x05
#define MAX86141_SR_1600HZ          0x06
#define MAX86141_SR_3200HZ          0x07

/* LED pulse width */
#define MAX86141_PW_69US            0x00
#define MAX86141_PW_118US           0x01
#define MAX86141_PW_215US           0x02
#define MAX86141_PW_411US           0x03

/* Data structure for PPG sample */
struct max86141_sample {
    uint32_t red;           /* Red LED channel */
    uint32_t ir;            /* IR LED channel */
    uint32_t timestamp_ms;  /* Timestamp in milliseconds */
    uint8_t quality;        /* Signal quality indicator (0-100) */
};

/* Configuration structure */
struct max86141_config {
    uint8_t sample_rate;    /* Sample rate (MAX86141_SR_*) */
    uint8_t pulse_width;    /* LED pulse width (MAX86141_PW_*) */
    uint8_t led_current;    /* LED current (0-255, ~0.2mA per step) */
    uint8_t adc_range;      /* ADC range (0-3) */
};

/* Function prototypes */
int max86141_init(const struct device *dev);
int max86141_configure(const struct device *dev, const struct max86141_config *config);
int max86141_start_measurement(const struct device *dev);
int max86141_stop_measurement(const struct device *dev);
int max86141_read_samples(const struct device *dev, struct max86141_sample *samples, 
                         uint8_t max_samples, uint8_t *num_read);
int max86141_get_fifo_count(const struct device *dev, uint8_t *count);
int max86141_clear_fifo(const struct device *dev);
int max86141_set_led_current(const struct device *dev, uint8_t current);
int max86141_get_temperature(const struct device *dev, float *temp_c);
int max86141_reset(const struct device *dev);

/* Utility functions */
uint8_t max86141_calculate_signal_quality(const struct max86141_sample *sample);
bool max86141_is_motion_artifact(const struct max86141_sample *samples, uint8_t count);

#endif /* MAX86141_H */
