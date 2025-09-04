/*
 * BMA400 IMU Driver
 * 
 * Driver for Bosch BMA400 3-axis accelerometer
 * Ultra-low power consumption with intelligent motion detection
 */

#ifndef BMA400_H
#define BMA400_H

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <stdint.h>

/* I2C Address */
#define BMA400_I2C_ADDR_PRIMARY     0x14
#define BMA400_I2C_ADDR_SECONDARY   0x15

/* Register Map */
#define BMA400_REG_CHIP_ID          0x00
#define BMA400_REG_ERR_REG          0x02
#define BMA400_REG_STATUS           0x03
#define BMA400_REG_ACC_X_LSB        0x04
#define BMA400_REG_ACC_X_MSB        0x05
#define BMA400_REG_ACC_Y_LSB        0x06
#define BMA400_REG_ACC_Y_MSB        0x07
#define BMA400_REG_ACC_Z_LSB        0x08
#define BMA400_REG_ACC_Z_MSB        0x09
#define BMA400_REG_SENSOR_TIME_0    0x0A
#define BMA400_REG_SENSOR_TIME_1    0x0B
#define BMA400_REG_SENSOR_TIME_2    0x0C
#define BMA400_REG_EVENT            0x0D
#define BMA400_REG_INT_STATUS0      0x0E
#define BMA400_REG_INT_STATUS1      0x0F
#define BMA400_REG_INT_STATUS2      0x10
#define BMA400_REG_TEMP_DATA        0x11
#define BMA400_REG_FIFO_LENGTH0     0x12
#define BMA400_REG_FIFO_LENGTH1     0x13
#define BMA400_REG_FIFO_DATA        0x14
#define BMA400_REG_STEP_CNT_0       0x15
#define BMA400_REG_STEP_CNT_1       0x16
#define BMA400_REG_STEP_CNT_2       0x17
#define BMA400_REG_STEP_STAT        0x18
#define BMA400_REG_ACC_CONFIG0      0x19
#define BMA400_REG_ACC_CONFIG1      0x1A
#define BMA400_REG_ACC_CONFIG2      0x1B
#define BMA400_REG_INT_CONFIG0      0x1F
#define BMA400_REG_INT_CONFIG1      0x20
#define BMA400_REG_INT1_MAP         0x21
#define BMA400_REG_INT2_MAP         0x22
#define BMA400_REG_INT12_MAP        0x23
#define BMA400_REG_INT12_IO_CTRL    0x24
#define BMA400_REG_FIFO_CONFIG0     0x26
#define BMA400_REG_FIFO_CONFIG1     0x27
#define BMA400_REG_FIFO_CONFIG2     0x28
#define BMA400_REG_FIFO_PWR_CONFIG  0x29
#define BMA400_REG_AUTOLOWPOW_0     0x2A
#define BMA400_REG_AUTOLOWPOW_1     0x2B
#define BMA400_REG_AUTOWAKEUP_0     0x2C
#define BMA400_REG_AUTOWAKEUP_1     0x2D
#define BMA400_REG_WKUP_INT_CONFIG0 0x2F
#define BMA400_REG_WKUP_INT_CONFIG1 0x30
#define BMA400_REG_WKUP_INT_CONFIG2 0x31
#define BMA400_REG_WKUP_INT_CONFIG3 0x32
#define BMA400_REG_WKUP_INT_CONFIG4 0x33
#define BMA400_REG_ORIENTCH_CONFIG0 0x35
#define BMA400_REG_ORIENTCH_CONFIG1 0x36
#define BMA400_REG_ORIENTCH_CONFIG2 0x37
#define BMA400_REG_ORIENTCH_CONFIG3 0x38
#define BMA400_REG_ORIENTCH_CONFIG4 0x39
#define BMA400_REG_ORIENTCH_CONFIG5 0x3A
#define BMA400_REG_ORIENTCH_CONFIG6 0x3B
#define BMA400_REG_ORIENTCH_CONFIG7 0x3C
#define BMA400_REG_ORIENTCH_CONFIG8 0x3D
#define BMA400_REG_ORIENTCH_CONFIG9 0x3E
#define BMA400_REG_GEN1INT_CONFIG0  0x3F
#define BMA400_REG_GEN1INT_CONFIG1  0x40
#define BMA400_REG_GEN1INT_CONFIG2  0x41
#define BMA400_REG_GEN1INT_CONFIG3  0x42
#define BMA400_REG_GEN1INT_CONFIG31 0x43
#define BMA400_REG_GEN1INT_CONFIG4  0x44
#define BMA400_REG_GEN1INT_CONFIG5  0x45
#define BMA400_REG_GEN1INT_CONFIG6  0x46
#define BMA400_REG_GEN1INT_CONFIG7  0x47
#define BMA400_REG_GEN1INT_CONFIG8  0x48
#define BMA400_REG_GEN1INT_CONFIG9  0x49
#define BMA400_REG_GEN2INT_CONFIG0  0x4A
#define BMA400_REG_GEN2INT_CONFIG1  0x4B
#define BMA400_REG_GEN2INT_CONFIG2  0x4C
#define BMA400_REG_GEN2INT_CONFIG3  0x4D
#define BMA400_REG_GEN2INT_CONFIG31 0x4E
#define BMA400_REG_GEN2INT_CONFIG4  0x4F
#define BMA400_REG_GEN2INT_CONFIG5  0x50
#define BMA400_REG_GEN2INT_CONFIG6  0x51
#define BMA400_REG_GEN2INT_CONFIG7  0x52
#define BMA400_REG_GEN2INT_CONFIG8  0x53
#define BMA400_REG_GEN2INT_CONFIG9  0x54
#define BMA400_REG_ACTCH_CONFIG0    0x55
#define BMA400_REG_ACTCH_CONFIG1    0x56
#define BMA400_REG_TAP_CONFIG0      0x57
#define BMA400_REG_TAP_CONFIG1      0x58
#define BMA400_REG_CMD              0x7E

/* Configuration values */
#define BMA400_CHIP_ID              0x90

/* Power modes */
#define BMA400_POWER_MODE_SLEEP     0x00
#define BMA400_POWER_MODE_LOW       0x01
#define BMA400_POWER_MODE_NORMAL    0x02

/* Output data rates */
#define BMA400_ODR_12_5HZ           0x05
#define BMA400_ODR_25HZ             0x06
#define BMA400_ODR_50HZ             0x07
#define BMA400_ODR_100HZ            0x08
#define BMA400_ODR_200HZ            0x09
#define BMA400_ODR_400HZ            0x0A
#define BMA400_ODR_800HZ            0x0B

/* Acceleration ranges */
#define BMA400_RANGE_2G             0x00
#define BMA400_RANGE_4G             0x01
#define BMA400_RANGE_8G             0x02
#define BMA400_RANGE_16G            0x03

/* Data structure for acceleration sample */
struct bma400_sample {
    int16_t x;              /* X-axis acceleration (mg) */
    int16_t y;              /* Y-axis acceleration (mg) */
    int16_t z;              /* Z-axis acceleration (mg) */
    uint32_t timestamp_ms;  /* Timestamp in milliseconds */
    int8_t temperature;     /* Temperature (°C) */
};

/* Configuration structure */
struct bma400_config {
    uint8_t power_mode;     /* Power mode (BMA400_POWER_MODE_*) */
    uint8_t odr;           /* Output data rate (BMA400_ODR_*) */
    uint8_t range;         /* Acceleration range (BMA400_RANGE_*) */
    bool enable_fifo;      /* Enable FIFO mode */
    bool enable_interrupts; /* Enable motion interrupts */
};

/* Activity states */
typedef enum {
    BMA400_ACTIVITY_STILL = 0,
    BMA400_ACTIVITY_WALKING,
    BMA400_ACTIVITY_RUNNING,
    BMA400_ACTIVITY_UNKNOWN
} bma400_activity_t;

/* Function prototypes */
int bma400_init(const struct device *dev);
int bma400_configure(const struct device *dev, const struct bma400_config *config);
int bma400_start_measurement(const struct device *dev);
int bma400_stop_measurement(const struct device *dev);
int bma400_read_sample(const struct device *dev, struct bma400_sample *sample);
int bma400_read_samples(const struct device *dev, struct bma400_sample *samples, 
                       uint8_t max_samples, uint8_t *num_read);
int bma400_get_fifo_count(const struct device *dev, uint16_t *count);
int bma400_clear_fifo(const struct device *dev);
int bma400_set_power_mode(const struct device *dev, uint8_t mode);
int bma400_enable_motion_interrupt(const struct device *dev, uint16_t threshold_mg);
int bma400_get_step_count(const struct device *dev, uint32_t *steps);
int bma400_reset_step_count(const struct device *dev);
int bma400_soft_reset(const struct device *dev);

/* Utility functions */
bma400_activity_t bma400_classify_activity(const struct bma400_sample *samples, uint8_t count);
uint32_t bma400_calculate_magnitude(const struct bma400_sample *sample);
bool bma400_detect_motion(const struct bma400_sample *samples, uint8_t count, uint16_t threshold_mg);

#endif /* BMA400_H */
