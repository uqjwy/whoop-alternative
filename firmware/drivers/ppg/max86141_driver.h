/*
 * MAX86141 AFE4950 PPG Sensor Driver Header
 * 
 * Enhanced driver for Maxim MAX86141 - Next-generation PPG sensor
 * with improved noise rejection and power efficiency
 * 
 * Features:
 * - 6 LED channels with programmable current
 * - Advanced ambient light cancellation  
 * - Temperature compensation
 * - FIFO buffer with interrupt support
 * - Ultra-low power modes
 * - I2C interface
 * 
 * Improvements over MAX30101:
 * - 50% lower power consumption
 * - Better motion artifact rejection
 * - Higher SNR in low perfusion conditions
 * - More flexible LED configuration
 */

#ifndef MAX86141_DRIVER_H
#define MAX86141_DRIVER_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include "../interfaces/sensor_interfaces.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MAX86141 I2C Address */
#define MAX86141_I2C_ADDR           0x57

/* MAX86141 Register Addresses */
#define MAX86141_REG_INTERRUPT_STATUS_1     0x00
#define MAX86141_REG_INTERRUPT_STATUS_2     0x01
#define MAX86141_REG_INTERRUPT_ENABLE_1     0x02
#define MAX86141_REG_INTERRUPT_ENABLE_2     0x03

#define MAX86141_REG_FIFO_WR_PTR           0x04
#define MAX86141_REG_FIFO_RD_PTR           0x05
#define MAX86141_REG_OVF_COUNTER           0x06
#define MAX86141_REG_FIFO_DATA_REG         0x07
#define MAX86141_REG_FIFO_CONFIG           0x08

#define MAX86141_REG_MODE_CONFIG           0x09
#define MAX86141_REG_SPO2_CONFIG           0x0A
#define MAX86141_REG_LED1_PA               0x0C
#define MAX86141_REG_LED2_PA               0x0D
#define MAX86141_REG_LED3_PA               0x0E
#define MAX86141_REG_LED4_PA               0x0F
#define MAX86141_REG_LED5_PA               0x10
#define MAX86141_REG_LED6_PA               0x11

#define MAX86141_REG_PILOT_PA              0x12
#define MAX86141_REG_LED_RANGE             0x13
#define MAX86141_REG_LED_SEQ_1             0x14
#define MAX86141_REG_LED_SEQ_2             0x15
#define MAX86141_REG_LED_SEQ_3             0x16

#define MAX86141_REG_TEMP_INT              0x1F
#define MAX86141_REG_TEMP_FRAC             0x20
#define MAX86141_REG_TEMP_CONFIG           0x21
#define MAX86141_REG_PROX_INT_THRESH       0x30

#define MAX86141_REG_REV_ID                0xFE
#define MAX86141_REG_PART_ID               0xFF

/* Register Bit Definitions */

/* Mode Configuration */
#define MAX86141_MODE_SHUTDOWN             0x80
#define MAX86141_MODE_RESET                0x40
#define MAX86141_MODE_HR_ONLY              0x02
#define MAX86141_MODE_SPO2                 0x03
#define MAX86141_MODE_MULTI_LED            0x07

/* FIFO Configuration */
#define MAX86141_FIFO_ROLLOVER_EN          0x10
#define MAX86141_FIFO_ALMOST_FULL_SHIFT    0x00

/* SPO2 Configuration */
#define MAX86141_SPO2_ADC_RGE_2048         0x00
#define MAX86141_SPO2_ADC_RGE_4096         0x20
#define MAX86141_SPO2_ADC_RGE_8192         0x40
#define MAX86141_SPO2_ADC_RGE_16384        0x60

#define MAX86141_SPO2_SR_50                0x00
#define MAX86141_SPO2_SR_100               0x04
#define MAX86141_SPO2_SR_200               0x08
#define MAX86141_SPO2_SR_400               0x0C
#define MAX86141_SPO2_SR_800               0x10
#define MAX86141_SPO2_SR_1000              0x14
#define MAX86141_SPO2_SR_1600              0x18
#define MAX86141_SPO2_SR_3200              0x1C

#define MAX86141_SPO2_PW_68_95             0x00
#define MAX86141_SPO2_PW_117_78            0x01
#define MAX86141_SPO2_PW_215_44            0x02
#define MAX86141_SPO2_PW_411_75            0x03

/* LED Current Values */
#define MAX86141_LED_CURRENT_0MA           0x00
#define MAX86141_LED_CURRENT_0_2MA         0x01
#define MAX86141_LED_CURRENT_50MA          0xFF

/* LED Range Values */
#define MAX86141_LED_RANGE_50MA            0x00
#define MAX86141_LED_RANGE_100MA           0x01
#define MAX86141_LED_RANGE_150MA           0x02
#define MAX86141_LED_RANGE_200MA           0x03

/* Interrupt flags */
#define MAX86141_INT_A_FULL                0x80
#define MAX86141_INT_PPG_RDY               0x40
#define MAX86141_INT_ALC_OVF               0x20
#define MAX86141_INT_PROX_INT              0x10
#define MAX86141_INT_PWR_RDY               0x01

/* MAX86141 Configuration Structure */
typedef struct {
    /* Basic Configuration */
    uint8_t mode;                    /* Operating mode */
    uint8_t sample_rate;             /* Sample rate */
    uint8_t adc_range;               /* ADC range */
    uint8_t pulse_width;             /* LED pulse width */
    
    /* LED Configuration */
    uint8_t led1_current;            /* LED1 (Red) current */
    uint8_t led2_current;            /* LED2 (IR) current */
    uint8_t led3_current;            /* LED3 (Green) current */
    uint8_t led4_current;            /* LED4 (Blue) current */
    uint8_t led5_current;            /* LED5 (additional) current */
    uint8_t led6_current;            /* LED6 (additional) current */
    
    uint8_t led_range;               /* LED current range */
    
    /* FIFO Configuration */
    uint8_t fifo_almost_full;        /* FIFO almost full threshold */
    bool fifo_rollover_en;           /* Enable FIFO rollover */
    
    /* Advanced Features */
    bool temp_enable;                /* Enable temperature sensor */
    bool proximity_enable;           /* Enable proximity detection */
    uint8_t proximity_threshold;     /* Proximity interrupt threshold */
    
    /* Power Management */
    bool ambient_light_cancel;       /* Enable ambient light cancellation */
    bool low_power_mode;             /* Enable low power mode */
    
} max86141_config_t;

/* MAX86141 Device Structure */
typedef struct {
    /* Hardware Interface */
    const struct device *i2c_dev;
    struct i2c_dt_spec i2c_spec;
    
    /* GPIO for interrupt */
    const struct device *gpio_dev;
    struct gpio_dt_spec int_gpio;
    struct gpio_callback int_callback;
    
    /* Configuration */
    max86141_config_t config;
    
    /* Runtime State */
    bool initialized;
    bool data_ready;
    uint32_t sample_count;
    
    /* Sensor Interface Implementation */
    ppg_sensor_ops_t sensor_ops;
    
    /* Calibration Data */
    float temp_offset;
    float gain_correction[6];        /* Per-LED gain correction */
    
    /* Power Management */
    uint32_t power_consumption_uw;   /* Current power consumption in µW */
    
} max86141_device_t;

/* MAX86141 Sample Data */
typedef struct {
    uint32_t led1;                   /* Red LED reading */
    uint32_t led2;                   /* IR LED reading */
    uint32_t led3;                   /* Green LED reading */
    uint32_t led4;                   /* Blue LED reading */
    uint32_t led5;                   /* Additional LED reading */
    uint32_t led6;                   /* Additional LED reading */
    float temperature;               /* Temperature in Celsius */
    uint64_t timestamp;              /* Sample timestamp */
    uint8_t active_leds;             /* Bitmask of active LEDs */
} max86141_sample_t;

/* Function Prototypes */

/**
 * Initialize MAX86141 device
 * @param dev Device structure to initialize
 * @param i2c_dev I2C device for communication
 * @param config Initial configuration
 * @return 0 on success, negative error code on failure
 */
int max86141_init(max86141_device_t *dev, const struct device *i2c_dev, const max86141_config_t *config);

/**
 * Configure MAX86141 sensor
 * @param dev Device structure
 * @param config New configuration
 * @return 0 on success, negative error code on failure
 */
int max86141_configure(max86141_device_t *dev, const max86141_config_t *config);

/**
 * Start PPG measurement
 * @param dev Device structure
 * @return 0 on success, negative error code on failure
 */
int max86141_start_measurement(max86141_device_t *dev);

/**
 * Stop PPG measurement
 * @param dev Device structure
 * @return 0 on success, negative error code on failure
 */
int max86141_stop_measurement(max86141_device_t *dev);

/**
 * Read PPG sample data
 * @param dev Device structure
 * @param sample Output buffer for sample data
 * @return 0 on success, negative error code on failure
 */
int max86141_read_sample(max86141_device_t *dev, max86141_sample_t *sample);

/**
 * Read FIFO data (multiple samples)
 * @param dev Device structure
 * @param samples Output buffer for samples
 * @param max_samples Maximum number of samples to read
 * @param samples_read Number of samples actually read
 * @return 0 on success, negative error code on failure
 */
int max86141_read_fifo(max86141_device_t *dev, max86141_sample_t *samples, 
                       uint32_t max_samples, uint32_t *samples_read);

/**
 * Read temperature
 * @param dev Device structure
 * @param temperature Output temperature in Celsius
 * @return 0 on success, negative error code on failure
 */
int max86141_read_temperature(max86141_device_t *dev, float *temperature);

/**
 * Enter low power mode
 * @param dev Device structure
 * @return 0 on success, negative error code on failure
 */
int max86141_enter_low_power(max86141_device_t *dev);

/**
 * Exit low power mode
 * @param dev Device structure
 * @return 0 on success, negative error code on failure
 */
int max86141_exit_low_power(max86141_device_t *dev);

/**
 * Reset device
 * @param dev Device structure
 * @return 0 on success, negative error code on failure
 */
int max86141_reset(max86141_device_t *dev);

/**
 * Check device ID
 * @param dev Device structure
 * @return 0 if device ID matches, negative error code otherwise
 */
int max86141_check_device_id(max86141_device_t *dev);

/**
 * Calibrate sensor
 * @param dev Device structure
 * @return 0 on success, negative error code on failure
 */
int max86141_calibrate(max86141_device_t *dev);

/**
 * Get power consumption
 * @param dev Device structure
 * @return Current power consumption in µW
 */
uint32_t max86141_get_power_consumption(max86141_device_t *dev);

/**
 * Interrupt handler
 * @param dev Device structure
 */
void max86141_interrupt_handler(max86141_device_t *dev);

/**
 * Create PPG sensor interface for MAX86141
 * @param dev Device structure
 * @return Pointer to PPG sensor operations structure
 */
ppg_sensor_ops_t* max86141_create_sensor_interface(max86141_device_t *dev);

/* Utility Functions */

/**
 * Convert raw ADC value to physical units
 * @param raw_value Raw ADC reading
 * @param adc_range ADC range setting
 * @param gain_correction Per-LED gain correction factor
 * @return Calibrated value
 */
float max86141_convert_raw_value(uint32_t raw_value, uint8_t adc_range, float gain_correction);

/**
 * Calculate optimal LED current for target SNR
 * @param ambient_light Ambient light level
 * @param target_snr Target signal-to-noise ratio
 * @param led_wavelength LED wavelength (nm)
 * @return Recommended LED current setting
 */
uint8_t max86141_calculate_optimal_current(uint32_t ambient_light, float target_snr, uint16_t led_wavelength);

/**
 * Auto-configure for optimal performance
 * @param dev Device structure
 * @param use_case Intended use case (rest, activity, sleep)
 * @return 0 on success, negative error code on failure
 */
int max86141_auto_configure(max86141_device_t *dev, ppg_use_case_t use_case);

#ifdef __cplusplus
}
#endif

#endif /* MAX86141_DRIVER_H */
