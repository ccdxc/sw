
/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include "dtls.h"

void
txq_send(uint32_t ring_size, volatile pkt_queue_t *qp, const pkt_t *pkt)
{
    uint32_t posted_index = qp->posted_index;
    uint32_t n_posted_index = (posted_index + 1) % ring_size;
    uint32_t fetch_index;
    do {
        fetch_index = qp->fetch_index;
    } while (fetch_index == n_posted_index);
    qp->ring[posted_index].len = pkt->len;
    w_memcpy((void *)qp->ring[posted_index].data, pkt->data, pkt->len);
    asm volatile("" ::: "memory");
    qp->posted_index = n_posted_index;
}

int
rxq_recv(uint32_t ring_size, volatile pkt_queue_t *qp, pkt_t *pkt)
{
    uint32_t fetch_index = qp->fetch_index;
    uint32_t posted_index = qp->posted_index;
    uint32_t len;
    int r = 0;
    if (posted_index == fetch_index) {
        return -EAGAIN;
    }
    len = qp->ring[fetch_index].len;
    if (len == 0 || len > MAX_PKTSIZE) {
        fprintf(stderr, "invalid packet size %u\n", len);
        r = -EINVAL;
    } else {
        pkt->len = len;
        w_memcpy(pkt->data, (void *)qp->ring[fetch_index].data, len);
    }
    asm volatile("" ::: "memory");
    qp->fetch_index = (fetch_index + 1) % ring_size;
    return r;
}
