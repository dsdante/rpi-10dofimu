#if _POSIX_C_SOURCE < 199309L  // required for sigaction on RPi
#define _POSIX_C_SOURCE 199309L
#endif  // _POSIX_C_SOURCE < 199309L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "imu10dof.h"

void sigint(int s)
{
    imu10dof_stop();
    puts("\nExiting.");
    exit(0);
}

int main()
{
    if (imu10dof_start(1))
        return 1;

    struct sigaction sigint_handler = { 0 };
    sigint_handler.sa_handler = sigint;
    sigemptyset(&sigint_handler.sa_mask);
    sigaction(SIGINT, &sigint_handler, NULL);

    puts(" Acl-X  Acl-Y  Acl-Z  Gyr-X  Gyr-Y  Gyr-Z  Mag-X  Mag-Y  Mag-Z   Temp  Press");
    struct accel_data accel_data;
    const struct timespec sleep_ts = { 0, 1e+6 };
    while (1) {
        if (imu10dof_read(&accel_data)) {
            puts("                                  No data.                                  \r");
            continue;
        }

        printf("\r%6d %6d %6d %6d %6d %6d %6d %6d %6d %6d %6d",
                accel_data.accel_x,
                accel_data.accel_y,
                accel_data.accel_z,
                accel_data.gyro_x,
                accel_data.gyro_y,
                accel_data.gyro_z,
                accel_data.magnet_x,
                accel_data.magnet_y,
                accel_data.magnet_z,
                accel_data.temp,
                accel_data.pressure);

        nanosleep(&sleep_ts, NULL);
    }
}
