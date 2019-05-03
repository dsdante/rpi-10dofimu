#include <stdint.h>
#include <stdio.h>
#include "rpi.h"
#include "imu10dof.h"

#define MPU9255_ADDRESS 0x68  // I2C address
#define MPU9255_ACCEL_XOUT_H 0x3B  // first accelerometer register
#define MPU9255_ACCEL_LENGTH 14  // number of accelerometer registers
#define MPU9255_HXL 0x03  // first magnetic register
#define MPU9255_MAGNETIC_LENGTH 6  // number of magnetic registers

void imu10dof_stop()
{
    i2c_close();
}

int imu10dof_start()
{
    if (bcm_model() == 0) {
        fputs("This system is not a Raspberry Pi.\n", stderr);
        return 1;
    }
    return i2c_open(1);
}

int imu10dof_read(struct accel_data *accel_data)
{
    int result = i2c_read(
            MPU9255_ADDRESS,
            MPU9255_ACCEL_XOUT_H,
            MPU9255_ACCEL_LENGTH,
            (uint8_t*)(&accel_data->accel_x));
    if (result)
        return result;
    result = i2c_read(
            MPU9255_ADDRESS,
            MPU9255_HXL,
            MPU9255_MAGNETIC_LENGTH,
            (uint8_t*)(&accel_data->magnet_x));
    if (result)
        return result;

    // Restore the byte order
    uint16_t *buff = (uint16_t*)accel_data;
    for (int i = 0; i < sizeof(accel_data)/sizeof(uint16_t); i++)
        buff[i] = (buff[i] >> 8) | (buff[i] << 8);
    accel_data->temp = (accel_data->temp >> 8) | (accel_data->temp << 8);
    accel_data->accel_z = (accel_data->accel_z >> 8) | (accel_data->accel_z << 8);

    return 0;
}
