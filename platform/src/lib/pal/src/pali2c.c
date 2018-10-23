/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pal.h"

#define FAIL -1
#define SUCCESS 0

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
