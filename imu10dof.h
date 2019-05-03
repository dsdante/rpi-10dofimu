#ifndef IMU10DOF_H
#define IMU10DOF_H

#include <stdint.h>

// Member order is important
struct accel_data
{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int16_t magnet_x;
    int16_t magnet_y;
    int16_t magnet_z;
    uint16_t pressure;
};

int imu10dof_start();
int imu10dof_read(struct accel_data *accel_data);
void imu10dof_stop();

#endif  // IMU10DOF_H
