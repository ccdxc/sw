/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pal.h"

#define I2C_NODE "/dev/i2c-"
#define FAIL -1
#define SUCCESS 0

int pal_i2c_read(u_int8_t *buffer, u_int32_t size, u_int32_t nretry,
                 u_int32_t bus, u_int32_t slaveaddr)
{
    int fd;
    char filename[64];
    u_int8_t wbuf = 0;

    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset[1];
    int i;

    snprintf(filename, 64, "%s%d", I2C_NODE, bus);
    if ((fd = open(filename, O_RDWR)) < 0) {
        printf("Failed to open the i2c bus, try again.\n");
        return FAIL;
    }

    for (i = 0; i < nretry; i++) {
        if (ioctl(fd, I2C_SLAVE, slaveaddr) < 0) {
            printf("Failed to acquire bus access and/or talk to slave.\n");
            continue;
        }
        
        msgs[0].addr = slaveaddr;
        msgs[0].flags = 0;
        msgs[0].len = 1;
        msgs[0].buf = &wbuf;

        msgs[1].addr = slaveaddr;
        msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
        msgs[1].len = size;
        msgs[1].buf = buffer;

        msgset[0].msgs = msgs;
        msgset[0].nmsgs = 2;

        if (ioctl(fd, I2C_RDWR, &msgset) < 0) {
            printf("ioctl(I2C_RDWR) in i2c_read failed\n");
            continue;
        }
        close(fd);
        return SUCCESS;
    }
    close(fd);
    return FAIL;
}

int pal_fru_read(u_int8_t *buffer, u_int32_t size, u_int32_t nretry)
{
    if (size < FRU_SIZE) {
        printf("Buffer is not of the right size \n");
        return FAIL;
    }
    return pal_i2c_read(buffer, FRU_SIZE, nretry, I2C_BUS, FRU_SLAVE_ADDRESS);
}

int pal_qsfp_read(u_int8_t *buffer, u_int32_t size, u_int32_t nretry, u_int32_t port)
{
    if (port == QSFP_PORT_1)
        return pal_i2c_read(buffer, nretry,
                            QSFP_1_I2C_BUS, QSFP_1_SLAVE_ADDRESS, size);
    else if (port == QSFP_PORT_2)
        return pal_i2c_read(buffer, nretry,
                            QSFP_2_I2C_BUS, QSFP_2_SLAVE_ADDRESS, size);
    return FAIL;
}

int pal_qsfp_dom_read(u_int8_t *buffer, u_int32_t size, u_int32_t nretry, u_int32_t port)
{
    if (port == QSFP_PORT_1)
        return pal_i2c_read(buffer, nretry,
                            QSFP_1_I2C_BUS, QSFP_DOM_1_SLAVE_ADDRESS, size);
    else if (port == QSFP_PORT_2)
        return pal_i2c_read(buffer, nretry,
                            QSFP_2_I2C_BUS, QSFP_DOM_2_SLAVE_ADDRESS, size);
    return FAIL;
}

