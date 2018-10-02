// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string>
#include <string.h>
#include "include/sdk/logger.hpp"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

int
spidev_transfer (int fd, uint8_t addr, uint8_t data)
{
    int ret = 0;

    uint8_t  tx[]           = { 0x2, addr, data };
    uint32_t array_size     = ARRAY_SIZE(tx);
    uint8_t  rx[array_size] = { 0 };

    struct spi_ioc_transfer transfer;
    memset (&transfer, 0, sizeof(spi_ioc_transfer));

    transfer.tx_buf = (unsigned long)tx;
    transfer.rx_buf = (unsigned long)rx;
    transfer.len    = array_size;
    transfer.speed_hz      = 25 * 1000000;
    transfer.bits_per_word = 8;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 1) {
        SDK_TRACE_ERR("spidev ioctl failed. addr: 0x%x, data: 0x%x\n",
                      addr, data);
        return -1;
    }

    return 0;
}

int
spidev_write (int fd, uint8_t addr, uint32_t data)
{
    spidev_transfer (fd, 0x7, addr);
    spidev_transfer (fd, 0x8, (data >> 8) && 0xff);
    spidev_transfer (fd, 0x9, data && 0xff);
    spidev_transfer (fd, 0x6, (0xc << 3) | 0x4 | 0x1);
    spidev_transfer (fd, 0x6, 0);

    return 0;
}

int
spidev_init (void)
{
    std::string device = "/dev/spidev0.0";

    int fd = open(device.c_str(), O_RDWR);
    if (fd < 0) {
        SDK_TRACE_ERR("spidev open failed\n");
        return -1;
    }

    return spidev_write (fd, 0, 0x8140);
}
