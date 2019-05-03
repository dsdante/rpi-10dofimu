#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "common.h"

// Get BCM SoC model. Zero means not a BCM system and thus not a Raspberry Pi.
int bcm_model()
{
    static int bcm = -1;
    if (bcm >= 0)
        return bcm;
    bcm = 0;  // not a BCM by default

    size_t cpuinfo_length = 16*1024;
    char *cpuinfo = read_file("/proc/cpuinfo", &cpuinfo_length);
    if (cpuinfo == NULL) {
        free(cpuinfo);
        return bcm;
    }

    char *token = strstr(cpuinfo, "Hardware");
    if (token == NULL) {
        free(cpuinfo);
        return bcm;
    }
    token = strstr(token, "BCM");
    if (token == NULL) {
        free(cpuinfo);
        return bcm;
    }
    token += 3;  // skip "BCM"

    bcm = atoi(token);
    free(cpuinfo);
    return bcm;
}

static int i2c_bus = -1;

void i2c_close()
{
    if (i2c_bus >= 0) {
        i2c_bus = -1;
        close(i2c_bus);
    }
}

int i2c_open(uint8_t bus)
{
    if (i2c_bus >= 0)
        i2c_close();

    char i2c_filename[16];
    sprintf(i2c_filename, "/dev/i2c-%d", bus);
    i2c_bus = open(i2c_filename, O_RDWR);
    if (i2c_bus < 0) {
        fprintf(stderr, "Cannot open '%s': %s\n", i2c_filename, strerror(errno));
        if (errno == ENOENT)
            fputs("Try running raspi-config and turning I²C on.\n", stderr);
        if (errno == EACCES)
            fputs("Try adding your user to the group 'i2c': # adduser your-user i2c\n", stderr);
        return errno;
    }
    return 0;
}

int i2c_read(uint16_t address, uint8_t reg, uint16_t length, uint8_t *buff)
{
    if (i2c_bus < 0) {
        fputs("I²C is not open.\n", stderr);
        return 1;
    }

    struct i2c_msg i2c_msgs[2] = {
            [0].addr = address,
            [0].len = 1,
            [0].buf = &reg,
            [1].addr = address,
            [1].flags = I2C_M_RD,
            [1].len = length,
            [1].buf = buff,
    };
    struct i2c_rdwr_ioctl_data i2c_transfer = {
            .msgs = i2c_msgs,
            .nmsgs = 2,
    };

    if (ioctl(i2c_bus, I2C_RDWR, &i2c_transfer) < 0) {
        fprintf(stderr, "Cannot read %#02x:%#02x: %s\n", address, reg, strerror(errno));
        return errno;
    }

    return 0;
}

int i2c_write(uint16_t address, uint8_t reg, uint8_t value)
{
    if (i2c_bus < 0) {
        fputs("I²C is not open.\n", stderr);
        return 1;
    }

    struct i2c_msg i2c_msg = {
        .addr = address,
        .len = 2,
        .buf = (uint8_t[]){ reg, value },
    };
    struct i2c_rdwr_ioctl_data i2c_transfer = {
            .msgs = &i2c_msg,
            .nmsgs = 1,
    };

    if (ioctl(i2c_bus, I2C_RDWR, &i2c_transfer) < 0) {
        fprintf(stderr, "Cannot write %#02x to %#02x:%#02x: %s\n", value, address, reg, strerror(errno));
        return errno;
    }

    return 0;
}
