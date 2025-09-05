/*
 * MAX86141 AFE4950 PPG Sensor Driver Implementation
 * 
 * Enhanced driver for Maxim MAX86141 - Next-generation PPG sensor
 * Implements sensor-agnostic interface for easy migration from MAX30101
 */

#include "max86141_driver.h"
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_REGISTER(max86141, LOG_LEVEL_INF);

/* Default configuration for MAX86141 */
static const max86141_config_t max86141_default_config = {
    .mode = MAX86141_MODE_SPO2,
    .sample_rate = MAX86141_SPO2_SR_100,
    .adc_range = MAX86141_SPO2_ADC_RGE_4096,
    .pulse_width = MAX86141_SPO2_PW_411_75,
    
    .led1_current = 0x24,            /* Red LED - 50mA */
    .led2_current = 0x24,            /* IR LED - 50mA */
    .led3_current = 0x24,            /* Green LED - 50mA */
    .led4_current = 0x00,            /* Blue LED - off */
    .led5_current = 0x00,            /* Additional LED - off */
    .led6_current = 0x00,            /* Additional LED - off */
    
    .led_range = MAX86141_LED_RANGE_100MA,
    
    .fifo_almost_full = 17,          /* Interrupt when 15 samples available */
    .fifo_rollover_en = true,
    
    .temp_enable = true,
    .proximity_enable = true,
    .proximity_threshold = 0x14,
    
    .ambient_light_cancel = true,
    .low_power_mode = false,
};

/* Private function prototypes */
static int max86141_write_reg(max86141_device_t *dev, uint8_t reg, uint8_t value);
static int max86141_read_reg(max86141_device_t *dev, uint8_t reg, uint8_t *value);
static int max86141_read_regs(max86141_device_t *dev, uint8_t reg, uint8_t *data, uint32_t len);
static void max86141_gpio_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

/* Sensor interface implementation */
static int max86141_sensor_init(void *dev_ctx, const sensor_config_t *config);
static int max86141_sensor_start(void *dev_ctx);
static int max86141_sensor_stop(void *dev_ctx);
static int max86141_sensor_read(void *dev_ctx, sensor_data_t *data);
static int max86141_sensor_set_power_mode(void *dev_ctx, power_mode_t mode);
static int max86141_sensor_calibrate(void *dev_ctx);
static uint32_t max86141_sensor_get_power_consumption(void *dev_ctx);
static int max86141_sensor_self_test(void *dev_ctx);

/**
 * Initialize MAX86141 device
 */
int max86141_init(max86141_device_t *dev, const struct device *i2c_dev, const max86141_config_t *config)
{
    int ret;
    uint8_t part_id, rev_id;
    
    if (!dev || !i2c_dev) {
        return -EINVAL;
    }
    
    /* Initialize device structure */
    memset(dev, 0, sizeof(max86141_device_t));
    dev->i2c_dev = i2c_dev;
    
    /* Set configuration */
    if (config) {
        dev->config = *config;
    } else {
        dev->config = max86141_default_config;
    }
    
    /* Check device ID */
    ret = max86141_check_device_id(dev);
    if (ret) {
        LOG_ERR("Device ID check failed: %d", ret);
        return ret;
    }
    
    /* Reset device */
    ret = max86141_reset(dev);
    if (ret) {
        LOG_ERR("Device reset failed: %d", ret);
        return ret;
    }
    
    /* Configure device */
    ret = max86141_configure(dev, &dev->config);
    if (ret) {
        LOG_ERR("Device configuration failed: %d", ret);
        return ret;
    }
    
    /* Initialize sensor interface */
    dev->sensor_ops.init = max86141_sensor_init;
    dev->sensor_ops.start = max86141_sensor_start;
    dev->sensor_ops.stop = max86141_sensor_stop;
    dev->sensor_ops.read = max86141_sensor_read;
    dev->sensor_ops.set_power_mode = max86141_sensor_set_power_mode;
    dev->sensor_ops.calibrate = max86141_sensor_calibrate;
    dev->sensor_ops.get_power_consumption = max86141_sensor_get_power_consumption;
    dev->sensor_ops.self_test = max86141_sensor_self_test;
    dev->sensor_ops.context = dev;
    
    /* Initialize calibration data */
    dev->temp_offset = 0.0f;
    for (int i = 0; i < 6; i++) {
        dev->gain_correction[i] = 1.0f;
    }
    
    dev->initialized = true;
    dev->power_consumption_uw = 1500; /* Typical consumption ~1.5mW */
    
    LOG_INF("MAX86141 initialized successfully (Part ID: 0x%02x, Rev: 0x%02x)", part_id, rev_id);
    
    return 0;
}

/**
 * Configure MAX86141 sensor
 */
int max86141_configure(max86141_device_t *dev, const max86141_config_t *config)
{
    int ret;
    
    if (!dev || !config) {
        return -EINVAL;
    }
    
    /* Update configuration */
    dev->config = *config;
    
    /* Configure FIFO */
    uint8_t fifo_config = (config->fifo_almost_full & 0x0F);
    if (config->fifo_rollover_en) {
        fifo_config |= MAX86141_FIFO_ROLLOVER_EN;
    }
    ret = max86141_write_reg(dev, MAX86141_REG_FIFO_CONFIG, fifo_config);
    if (ret) return ret;
    
    /* Configure Mode */
    ret = max86141_write_reg(dev, MAX86141_REG_MODE_CONFIG, config->mode);
    if (ret) return ret;
    
    /* Configure SpO2 settings */
    uint8_t spo2_config = config->adc_range | config->sample_rate | config->pulse_width;
    ret = max86141_write_reg(dev, MAX86141_REG_SPO2_CONFIG, spo2_config);
    if (ret) return ret;
    
    /* Configure LED currents */
    ret = max86141_write_reg(dev, MAX86141_REG_LED1_PA, config->led1_current);
    if (ret) return ret;
    ret = max86141_write_reg(dev, MAX86141_REG_LED2_PA, config->led2_current);
    if (ret) return ret;
    ret = max86141_write_reg(dev, MAX86141_REG_LED3_PA, config->led3_current);
    if (ret) return ret;
    ret = max86141_write_reg(dev, MAX86141_REG_LED4_PA, config->led4_current);
    if (ret) return ret;
    ret = max86141_write_reg(dev, MAX86141_REG_LED5_PA, config->led5_current);
    if (ret) return ret;
    ret = max86141_write_reg(dev, MAX86141_REG_LED6_PA, config->led6_current);
    if (ret) return ret;
    
    /* Configure LED range */
    ret = max86141_write_reg(dev, MAX86141_REG_LED_RANGE, config->led_range);
    if (ret) return ret;
    
    /* Configure interrupts */
    uint8_t int_enable = MAX86141_INT_A_FULL | MAX86141_INT_PPG_RDY;
    ret = max86141_write_reg(dev, MAX86141_REG_INTERRUPT_ENABLE_1, int_enable);
    if (ret) return ret;
    
    /* Configure temperature sensor */
    if (config->temp_enable) {
        ret = max86141_write_reg(dev, MAX86141_REG_TEMP_CONFIG, 0x01);
        if (ret) return ret;
    }
    
    /* Configure proximity detection */
    if (config->proximity_enable) {
        ret = max86141_write_reg(dev, MAX86141_REG_PROX_INT_THRESH, config->proximity_threshold);
        if (ret) return ret;
    }
    
    /* Update power consumption estimate */
    max86141_update_power_consumption(dev);
    
    LOG_INF("MAX86141 configured successfully");
    
    return 0;
}

/**
 * Start PPG measurement
 */
int max86141_start_measurement(max86141_device_t *dev)
{
    int ret;
    
    if (!dev || !dev->initialized) {
        return -EINVAL;
    }
    
    /* Clear FIFO */
    ret = max86141_write_reg(dev, MAX86141_REG_FIFO_WR_PTR, 0x00);
    if (ret) return ret;
    ret = max86141_write_reg(dev, MAX86141_REG_FIFO_RD_PTR, 0x00);
    if (ret) return ret;
    ret = max86141_write_reg(dev, MAX86141_REG_OVF_COUNTER, 0x00);
    if (ret) return ret;
    
    /* Start measurement */
    ret = max86141_write_reg(dev, MAX86141_REG_MODE_CONFIG, dev->config.mode);
    if (ret) return ret;
    
    dev->sample_count = 0;
    
    LOG_INF("MAX86141 measurement started");
    
    return 0;
}

/**
 * Stop PPG measurement
 */
int max86141_stop_measurement(max86141_device_t *dev)
{
    int ret;
    
    if (!dev || !dev->initialized) {
        return -EINVAL;
    }
    
    /* Enter shutdown mode */
    ret = max86141_write_reg(dev, MAX86141_REG_MODE_CONFIG, MAX86141_MODE_SHUTDOWN);
    if (ret) return ret;
    
    LOG_INF("MAX86141 measurement stopped");
    
    return 0;
}

/**
 * Read PPG sample data
 */
int max86141_read_sample(max86141_device_t *dev, max86141_sample_t *sample)
{
    int ret;
    uint8_t fifo_data[18]; /* 6 LEDs × 3 bytes each */
    uint32_t samples_available;
    
    if (!dev || !sample || !dev->initialized) {
        return -EINVAL;
    }
    
    /* Check if data is available */
    ret = max86141_read_fifo(dev, sample, 1, &samples_available);
    if (ret) return ret;
    
    if (samples_available == 0) {
        return -EAGAIN; /* No data available */
    }
    
    return 0;
}

/**
 * Read FIFO data (multiple samples)
 */
int max86141_read_fifo(max86141_device_t *dev, max86141_sample_t *samples, 
                       uint32_t max_samples, uint32_t *samples_read)
{
    int ret;
    uint8_t wr_ptr, rd_ptr;
    uint32_t available_samples;
    uint8_t fifo_data[18];
    
    if (!dev || !samples || !samples_read || !dev->initialized) {
        return -EINVAL;
    }
    
    /* Read FIFO pointers */
    ret = max86141_read_reg(dev, MAX86141_REG_FIFO_WR_PTR, &wr_ptr);
    if (ret) return ret;
    ret = max86141_read_reg(dev, MAX86141_REG_FIFO_RD_PTR, &rd_ptr);
    if (ret) return ret;
    
    /* Calculate available samples */
    if (wr_ptr >= rd_ptr) {
        available_samples = wr_ptr - rd_ptr;
    } else {
        available_samples = (32 - rd_ptr) + wr_ptr; /* FIFO size is 32 */
    }
    
    /* Limit to requested samples */
    if (available_samples > max_samples) {
        available_samples = max_samples;
    }
    
    *samples_read = 0;
    
    /* Read samples */
    for (uint32_t i = 0; i < available_samples; i++) {
        /* Read 18 bytes for all 6 LEDs (3 bytes each) */
        ret = max86141_read_regs(dev, MAX86141_REG_FIFO_DATA_REG, fifo_data, 18);
        if (ret) return ret;
        
        /* Parse FIFO data */
        samples[i].led1 = (fifo_data[0] << 16) | (fifo_data[1] << 8) | fifo_data[2];
        samples[i].led1 &= 0x03FFFF; /* 18-bit resolution */
        
        samples[i].led2 = (fifo_data[3] << 16) | (fifo_data[4] << 8) | fifo_data[5];
        samples[i].led2 &= 0x03FFFF;
        
        samples[i].led3 = (fifo_data[6] << 16) | (fifo_data[7] << 8) | fifo_data[8];
        samples[i].led3 &= 0x03FFFF;
        
        samples[i].led4 = (fifo_data[9] << 16) | (fifo_data[10] << 8) | fifo_data[11];
        samples[i].led4 &= 0x03FFFF;
        
        samples[i].led5 = (fifo_data[12] << 16) | (fifo_data[13] << 8) | fifo_data[14];
        samples[i].led5 &= 0x03FFFF;
        
        samples[i].led6 = (fifo_data[15] << 16) | (fifo_data[16] << 8) | fifo_data[17];
        samples[i].led6 &= 0x03FFFF;
        
        /* Apply calibration */
        samples[i].led1 = max86141_convert_raw_value(samples[i].led1, dev->config.adc_range, dev->gain_correction[0]);
        samples[i].led2 = max86141_convert_raw_value(samples[i].led2, dev->config.adc_range, dev->gain_correction[1]);
        samples[i].led3 = max86141_convert_raw_value(samples[i].led3, dev->config.adc_range, dev->gain_correction[2]);
        
        /* Set timestamp */
        samples[i].timestamp = k_uptime_get();
        samples[i].active_leds = 0x07; /* LEDs 1-3 active by default */
        
        (*samples_read)++;
        dev->sample_count++;
    }
    
    return 0;
}

/**
 * Read temperature
 */
int max86141_read_temperature(max86141_device_t *dev, float *temperature)
{
    int ret;
    uint8_t temp_int, temp_frac;
    
    if (!dev || !temperature || !dev->initialized) {
        return -EINVAL;
    }
    
    /* Trigger temperature measurement */
    ret = max86141_write_reg(dev, MAX86141_REG_TEMP_CONFIG, 0x01);
    if (ret) return ret;
    
    /* Wait for measurement to complete */
    k_msleep(100);
    
    /* Read temperature registers */
    ret = max86141_read_reg(dev, MAX86141_REG_TEMP_INT, &temp_int);
    if (ret) return ret;
    ret = max86141_read_reg(dev, MAX86141_REG_TEMP_FRAC, &temp_frac);
    if (ret) return ret;
    
    /* Convert to Celsius */
    *temperature = (float)temp_int + ((float)(temp_frac & 0x0F)) * 0.0625f;
    *temperature += dev->temp_offset; /* Apply calibration offset */
    
    return 0;
}

/**
 * Reset device
 */
int max86141_reset(max86141_device_t *dev)
{
    int ret;
    
    if (!dev) {
        return -EINVAL;
    }
    
    /* Write reset bit */
    ret = max86141_write_reg(dev, MAX86141_REG_MODE_CONFIG, MAX86141_MODE_RESET);
    if (ret) return ret;
    
    /* Wait for reset to complete */
    k_msleep(100);
    
    return 0;
}

/**
 * Check device ID
 */
int max86141_check_device_id(max86141_device_t *dev)
{
    int ret;
    uint8_t part_id;
    
    if (!dev) {
        return -EINVAL;
    }
    
    ret = max86141_read_reg(dev, MAX86141_REG_PART_ID, &part_id);
    if (ret) return ret;
    
    if (part_id != 0x36) { /* Expected MAX86141 part ID */
        LOG_ERR("Invalid part ID: 0x%02x (expected 0x36)", part_id);
        return -ENODEV;
    }
    
    return 0;
}

/* Sensor interface implementations */
static int max86141_sensor_init(void *dev_ctx, const sensor_config_t *config)
{
    max86141_device_t *dev = (max86141_device_t *)dev_ctx;
    
    /* Convert generic config to MAX86141 config if needed */
    if (config && config->ppg_config) {
        /* Apply PPG-specific configuration */
        // This would convert generic config to MAX86141-specific settings
    }
    
    return 0; /* Already initialized in max86141_init */
}

static int max86141_sensor_start(void *dev_ctx)
{
    max86141_device_t *dev = (max86141_device_t *)dev_ctx;
    return max86141_start_measurement(dev);
}

static int max86141_sensor_stop(void *dev_ctx)
{
    max86141_device_t *dev = (max86141_device_t *)dev_ctx;
    return max86141_stop_measurement(dev);
}

static int max86141_sensor_read(void *dev_ctx, sensor_data_t *data)
{
    max86141_device_t *dev = (max86141_device_t *)dev_ctx;
    max86141_sample_t sample;
    int ret;
    
    ret = max86141_read_sample(dev, &sample);
    if (ret) return ret;
    
    /* Convert to generic sensor data format */
    data->ppg.red = sample.led1;
    data->ppg.ir = sample.led2;
    data->ppg.green = sample.led3;
    data->ppg.blue = sample.led4;
    data->temperature = sample.temperature;
    data->timestamp = sample.timestamp;
    data->quality = 100; /* TODO: Calculate signal quality */
    
    return 0;
}

static int max86141_sensor_set_power_mode(void *dev_ctx, power_mode_t mode)
{
    max86141_device_t *dev = (max86141_device_t *)dev_ctx;
    
    switch (mode) {
    case POWER_MODE_LOW:
        return max86141_enter_low_power(dev);
    case POWER_MODE_NORMAL:
        return max86141_exit_low_power(dev);
    default:
        return -EINVAL;
    }
}

static int max86141_sensor_calibrate(void *dev_ctx)
{
    max86141_device_t *dev = (max86141_device_t *)dev_ctx;
    return max86141_calibrate(dev);
}

static uint32_t max86141_sensor_get_power_consumption(void *dev_ctx)
{
    max86141_device_t *dev = (max86141_device_t *)dev_ctx;
    return max86141_get_power_consumption(dev);
}

static int max86141_sensor_self_test(void *dev_ctx)
{
    max86141_device_t *dev = (max86141_device_t *)dev_ctx;
    
    /* Perform basic self-test */
    return max86141_check_device_id(dev);
}

/* Create sensor interface */
ppg_sensor_ops_t* max86141_create_sensor_interface(max86141_device_t *dev)
{
    if (!dev) {
        return NULL;
    }
    
    return &dev->sensor_ops;
}

/* Private helper functions */
static int max86141_write_reg(max86141_device_t *dev, uint8_t reg, uint8_t value)
{
    uint8_t tx_buf[2] = {reg, value};
    return i2c_write(dev->i2c_dev, tx_buf, sizeof(tx_buf), MAX86141_I2C_ADDR);
}

static int max86141_read_reg(max86141_device_t *dev, uint8_t reg, uint8_t *value)
{
    return i2c_write_read(dev->i2c_dev, MAX86141_I2C_ADDR, &reg, 1, value, 1);
}

static int max86141_read_regs(max86141_device_t *dev, uint8_t reg, uint8_t *data, uint32_t len)
{
    return i2c_write_read(dev->i2c_dev, MAX86141_I2C_ADDR, &reg, 1, data, len);
}

/* Additional utility functions implementation would continue here... */
