#include <byteswap.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rpi.h"
#include "imu10dof.h"

// MPU9255 accelerometer/gyroscope
#define MPU9255_I2C_ADDRESS     0x68  // I²C address
#define MPU9255_ID              0x73  // device ID
#define MPU9255_DATA_LENGTH     14    // measurement data length
// Registers
#define MPU9255_GYRO_CONFIG     0x1B  // gyroscope configuration
#define MPU9255_ACCEL_CONFIG    0x1C  // accelerometer configuration
#define MPU9255_INT_PIN_CFG     0x37  // I²C bypass configuration
#define MPU9255_ACCEL_XOUT_H    0x3B  // measurement data beginning
#define MPU9255_PWR_MGMT_1      0x6B  // power management
#define MPU9255_WHO_AM_I        0x75  // get device ID

// AK8963 magnetometer
#define AK8963_I2C_ADDRESS      0x0C  // I²C address
#define AK8963_ID               0x48  // device ID
#define AK8963_DATA_LENGTH      7     // measurement data length
// Registers
#define AK8963_WIA              0x00  // get device ID
#define AK8963_HXL              0x03  // measurement data beginning
#define AK8963_CNTL1            0x0A  // configuration

// BMP180 pressure sensor
#define BMP180_I2C_ADDRESS      0x77  // I²C address
#define BMP180_ID               0x55  // device ID
#define BMP180_DATA_LENGTH      2     // measurement data length
// Registers
#define BMP180_GET_ID           0xD0  // get device ID
#define BMP180_CTRL_MEAS        0xF4  // measurement control
#define BMP180_OUT_MSB          0xF6  // measurement data beginning

int imu10dof_start()
{
    if (bcm_model() == 0) {
        fputs("This system is not a Raspberry Pi.\n", stderr);
        return 1;
    }
    if (i2c_open(1))
        return 1;

    // MPU-9255 accelerometer/gyroscope
    uint8_t device_id;
    if (i2c_read(MPU9255_I2C_ADDRESS, MPU9255_WHO_AM_I, 1, &device_id))
        return 1;
    if (device_id != MPU9255_ID) {
        fprintf(stderr, "Expected device ID %#02x (MPU-9255), actual is %#02x.\n", MPU9255_ID, device_id);
        return 1;
    }
    i2c_write(MPU9255_I2C_ADDRESS, MPU9255_PWR_MGMT_1, 0x81);  // reset all
    i2c_write(MPU9255_I2C_ADDRESS, MPU9255_GYRO_CONFIG, 0x18);  // ±2000 °/s  ==  ±34.907 rad/s
    i2c_write(MPU9255_I2C_ADDRESS, MPU9255_ACCEL_CONFIG, 0x18);  // ±16 g  ==  ±156.91 m/s²
    i2c_write(MPU9255_I2C_ADDRESS, MPU9255_INT_PIN_CFG, 0x02);  // bypass AK8963 to the common I²C bus

    // AK8963 magnetometer
    if (i2c_read(AK8963_I2C_ADDRESS, AK8963_WIA, 1, &device_id))
        return 1;
    if (device_id != AK8963_ID) {
        fprintf(stderr, "Expected device ID %#02x (AK893), actual is %#02x.\n", AK8963_ID, device_id);
        return 1;
    }
    i2c_write(AK8963_I2C_ADDRESS, AK8963_CNTL1, 0x16);  // continuous 16-bit measurement

    // BMP180 pressure sensor
    if (i2c_read(BMP180_I2C_ADDRESS, BMP180_GET_ID, 1, &device_id))
        return 1;
    if (device_id != BMP180_ID) {
        fprintf(stderr, "Expected device ID %#02x (BMP180), actual is %#02x.\n", BMP180_ID, device_id);
        return 1;
    }
    i2c_write(BMP180_I2C_ADDRESS, BMP180_CTRL_MEAS, 0x34);  // request the first measurement
    return 0;
}

int imu10dof_read(struct accel_data *accel_data)
{
    // MPU-9255 accelerometer/gyroscope
    if (i2c_read(MPU9255_I2C_ADDRESS, MPU9255_ACCEL_XOUT_H, MPU9255_DATA_LENGTH, (uint8_t*)(&accel_data->accel_x)))
        return 1;
    // Restore the byte order
    int16_t *buff = (int16_t*)accel_data;
    for (int i = 0; i < MPU9255_DATA_LENGTH/sizeof(int16_t); i++)
        buff[i] = bswap_16(buff[i]);

    // AK8963 magnetometer
    static uint8_t mag_buff[AK8963_DATA_LENGTH];
    if (i2c_read(AK8963_I2C_ADDRESS, AK8963_HXL, AK8963_DATA_LENGTH, mag_buff))  // reading the 7th byte triggers the next measurement
        return 2;
    memcpy(&accel_data->magnet_x, mag_buff, 6);

    // BMP180 pressure sensor
    static uint16_t pressure = 0;  // keep old if the new measurement is not ready
    uint8_t status;
    if (i2c_read(BMP180_I2C_ADDRESS, BMP180_CTRL_MEAS, 1, &status))
        return 3;
    if (!(status & 0x20)) { // measurement ready
        i2c_read(BMP180_I2C_ADDRESS, BMP180_OUT_MSB, sizeof(pressure), (uint8_t*)&pressure);
        i2c_write(BMP180_I2C_ADDRESS, BMP180_CTRL_MEAS, 0x34);  // request the next measurement
    }
    accel_data->pressure = bswap_16(pressure);

    return 0;
}

void imu10dof_stop()
{
    i2c_close();
}
