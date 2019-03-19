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
#include <pthread.h>

#include "mnet.h"

static pthread_mutex_t mnet_mutex = PTHREAD_MUTEX_INITIALIZER;

int
create_mnet(struct mnet_dev_create_req_t *req)
{
    int ret, fd;

    fd = open(MNET_DEVICE_FILE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Error: %s %s\n", MNET_DEVICE_FILE, strerror(errno));
        return fd;
    }

    pthread_mutex_lock(&mnet_mutex);
    ret = ioctl(fd, MNET_CREATE_DEV, req);
    pthread_mutex_unlock(&mnet_mutex);
    close(fd);

    if (ret) {
        fprintf(stderr, "Error: %s %s\n", "MNET_CREATE_DEV", strerror(errno));
        return ret;
    }

    return 0;
}

int
remove_mnet(const char* if_name)
{
    int ret, fd;

    fd = open(MNET_DEVICE_FILE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Error: %s %s\n", MNET_DEVICE_FILE, strerror(errno));
        return fd;
    }

    pthread_mutex_lock(&mnet_mutex);
    ret = ioctl(fd, MNET_DESTROY_DEV, if_name);
    pthread_mutex_unlock(&mnet_mutex);
    close(fd);

    if (ret) {
        fprintf(stderr, "Error: %s %s\n", "MNET_DESTROY_DEV", strerror(errno));
        return ret;
    }

    return 0;
}

