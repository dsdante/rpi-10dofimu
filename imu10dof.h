#ifndef IMU10DOF_H
#define IMU10DOF_H

#include <stdint.h>

// Byte order is important
struct accel_data
{
    uint16_t accel_x;
    uint16_t accel_y;
    uint16_t accel_z;
    uint16_t temp;
    uint16_t gyro_x;
    uint16_t gyro_y;
    uint16_t gyro_z;
    uint16_t magnet_x;
    uint16_t magnet_y;
    uint16_t magnet_z;
    // TODO: pressure
};

int imu10dof_start();
int imu10dof_read(struct accel_data *accel_data);
void imu10dof_stop();

#endif  // IMU10DOF_H
