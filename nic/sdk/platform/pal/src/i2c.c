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
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "pal.h"

#define I2C_NODE "/dev/i2c-"

#define ADDR_LEN_8BIT    8
#define ADDR_LEN_16BIT   16

static int pal_i2c_read(const uint8_t *buffer, uint32_t size,
                        uint32_t offset, uint32_t nretry,
                        uint32_t bus, uint32_t slaveaddr,
                        uint8_t address_length)
{
    int i;
    int fd;
    char filename[64];
    uint8_t wbuf[2];
    int n = 0;

    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset[1];

    if (address_length == ADDR_LEN_16BIT) {
        wbuf[n++] = offset >> 8;
    }
    wbuf[n++] = offset;

    snprintf(filename, 64, "%s%d", I2C_NODE, bus);
    if ((fd = open(filename, O_RDWR)) < 0) {
        pal_mem_trace("Failed to open the i2c bus, try again.\n");
        return -1;
    }

    for (i = 0; i < nretry; i++) {
        if (ioctl(fd, I2C_SLAVE_FORCE, slaveaddr) < 0) {
            pal_mem_trace("Failed to acquire bus access and/or talk to slave.\n");
            continue;
        }
        msgs[0].addr = slaveaddr;
        msgs[0].flags = 0;
        msgs[0].len = n;
        msgs[0].buf = &wbuf;

        msgs[1].addr = slaveaddr;
        msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
        msgs[1].len = size;
        msgs[1].buf = (uint8_t*)buffer;

        msgset[0].msgs = msgs;
        msgset[0].nmsgs = 2;

        if (ioctl(fd, I2C_RDWR, &msgset) < 0) {
            pal_mem_trace("ioctl(I2C_RDWR) in i2c_read_offset failed\n");
            continue;
        }
        close(fd);
        return 0;
    }
    close(fd);
    return -1;
}

static int pal_i2c_write(const uint8_t *buffer, uint32_t size,
                         uint32_t addr, uint32_t nretry,
                         uint32_t bus, uint32_t slaveaddr)
{
    int i;
    int fd;
    char filename[64];
    uint8_t *wbuf = malloc(size + 1);

    wbuf[0] = addr;
    memcpy(&wbuf[1], buffer, sizeof(size));

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    snprintf(filename, 64, "%s%d", I2C_NODE, bus);
    if ((fd = open(filename, O_RDWR)) < 0) {
        pal_mem_trace("Failed to open the i2c bus, try again.\n");
        return -1;
    }

    for (i = 0; i < nretry; i++) {
        if (ioctl(fd, I2C_SLAVE_FORCE, slaveaddr) < 0) {
            pal_mem_trace("Failed to acquire bus access and/or talk to slave.\n");
            continue;
        }
    
        msgs[0].addr = slaveaddr;
        msgs[0].flags = 0;
        msgs[0].len = size + 1;
        msgs[0].buf = wbuf;

        msgset[0].msgs = msgs;
        msgset[0].nmsgs = 1;

        if (ioctl(fd, I2C_RDWR, &msgset) < 0) {
            pal_mem_trace("ioctl(I2C_RDWR) in i2c_write failed\n");
            continue;
        }

        close(fd);
        return 0;
    }
    close(fd);
    return -1;
}

int pal_fru_read(const uint8_t *buffer, uint32_t size, uint32_t nretry)
{
    int id;

    if (size != FRU_SIZE) {
        pal_mem_trace("Buffer is not of the right size \n");
        return -1;
    }
    id = pal_get_cpld_id();
    if (id == CPLD_ID_NAPLES25_SWM || id == CPLD_ID_NAPLES25_OCP)
        return pal_i2c_read(buffer, FRU_SIZE, 0, nretry, I2C_BUS, FRU_SLAVE_ADDRESS,
                            ADDR_LEN_16BIT);
     else
        return pal_i2c_read(buffer, FRU_SIZE, 0, nretry, I2C_BUS, FRU_SLAVE_ADDRESS,
                            ADDR_LEN_8BIT);
}

int pal_qsfp_read(const uint8_t *buffer, uint32_t size,
                  uint32_t offset, uint32_t nretry, uint32_t port)
{
    if (port == QSFP_PORT_1)
        return pal_i2c_read(buffer, size, offset, nretry,
                            QSFP_1_I2C_BUS, QSFP_1_SLAVE_ADDRESS, ADDR_LEN_8BIT);
    else if (port == QSFP_PORT_2)
        return pal_i2c_read(buffer, size, offset, nretry,
                            QSFP_2_I2C_BUS, QSFP_2_SLAVE_ADDRESS, ADDR_LEN_8BIT);
    return -1;
}

int pal_qsfp_write(const uint8_t *buffer, uint32_t size,
                   uint32_t addr, uint32_t nretry, uint32_t port)
{
    if (port == QSFP_PORT_1)
        return pal_i2c_write(buffer, size, addr, nretry,
                             QSFP_1_I2C_BUS, QSFP_1_SLAVE_ADDRESS);
    else if (port == QSFP_PORT_2)
        return pal_i2c_write(buffer, size, addr, nretry,
                             QSFP_2_I2C_BUS, QSFP_2_SLAVE_ADDRESS);
    return -1;
}

int smbus_write(const uint8_t *buffer, uint32_t size,
                uint32_t offset, uint32_t nretry,
                uint32_t bus, uint32_t slaveaddr)
{
    return pal_i2c_write(buffer, size, offset,
                         nretry, bus, slaveaddr);
}

int smbus_read(const uint8_t *buffer, uint32_t size,
                uint32_t offset, uint32_t nretry,
                uint32_t bus, uint32_t slaveaddr)
{
    return pal_i2c_read(buffer, size, offset,
                        nretry, bus, slaveaddr, ADDR_LEN_8BIT);
}
