/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "mnet.h"

int
create_mnet(struct mnet_dev_create_req_t *req)
{
    int ret, fd;
    char* dev_node = MNET_DEVICE_FILE;

    fd = open(dev_node, O_RDWR);

    if(fd < 0)
    {
        fprintf(stderr, "Error: %s %s\n", dev_node, strerror(errno));
        return fd;
    }

    ret = ioctl(fd, MNET_CREATE_DEV, req);

    if (ret)
    {
        fprintf(stderr, "Error: %s %s\n", "MNET_CREATE_DEV", strerror(errno));
        return ret;
    }

    close(fd);

    return 0;
}

int
remove_mnet(const char* if_name)
{
    int ret, fd;
    char* dev_node = MNET_DEVICE_FILE;

    fd = open(dev_node, O_RDWR);

    if(fd < 0)
    {
        fprintf(stderr, "Error: %s %s\n", dev_node, strerror(errno));
        return fd;
    }

    ret = ioctl(fd, MNET_DESTROY_DEV, if_name);

    if (ret)
    {
        fprintf(stderr, "Error: %s %s\n", "MNET_DESTROY_DEV", strerror(errno));
        return ret;
    }

    close(fd);

    return 0;
}

