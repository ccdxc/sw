
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include "dtls.h"

static int
service_queues(int tunfd, uint32_t ring_size, memtun_info_t *mi, int *activity)
{
    pkt_t pkt;
    int n;

    if (rxq_recv(ring_size, mi->rxq, &pkt) == 0) {
        if (write(tunfd, pkt.data, pkt.len) != pkt.len) {
            perror("write");
            return -1;
        }
        *activity = 1;
    }
    n = read(tunfd, pkt.data, sizeof (pkt.data));
    if (n < 0 && errno != EAGAIN) {
        perror("read");
        return -1;
    }
    if (n > 0) {
        pkt.len = n;
        //printf("tun rx %u bytes\n", pkt.len);
        //dump(2, pkt.data, pkt.len);
        txq_send(ring_size, mi->txq, &pkt);
        *activity = 1;
    }
    return 0;
}

static uint64_t
time_get_us(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000ULL) + (ts.tv_nsec / 1000);
}

int
packet_loop(int tunfd, memtun_info_t *mi, packet_pollcb_t pollcb, void *arg)
{
    int is_active, activity;
    uint32_t ring_size;
    uint64_t t_timeo;

    ring_size = mi->hdr->ring_size;
    t_timeo = time_get_us() + ACTIVITY_TIMEO_US;
    is_active = 0;
    for (;;) {
        if (!is_active) {
            usleep(INACTIVITY_SLEEP_US);
        }
        activity = 0;
        if (pollcb != NULL && pollcb(tunfd, mi, &activity, arg) < 0) {
            return -1;
        }
        if (service_queues(tunfd, ring_size, mi, &activity) < 0) {
            return -1;
        }
        if (activity) {
            t_timeo = time_get_us() + ACTIVITY_TIMEO_US;
            is_active = 1;
        } else if (is_active && time_get_us() >= t_timeo) {
            is_active = 0;
        }
    }
    return 0;
}
