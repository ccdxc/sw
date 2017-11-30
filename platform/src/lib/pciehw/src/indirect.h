/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __INDIRECT_H__
#define __INDIRECT_H__

typedef enum {
#define PCIEIND_REASON_DEF(NAME, VAL)               \
    NAME,

#include "indirect_reason.h"

    PCIEIND_REASON_MAX
} pcieind_reason_t;

typedef struct {
    /* u_int64_t [0] */
    u_int64_t direct_endaddr:6;
    u_int64_t direct_blen:4;
    u_int64_t is_indirect:1;
    u_int64_t is_direct:1;
    u_int64_t is_ur:1;
    u_int64_t is_ca:1;
    u_int64_t romsksel:7;
    u_int64_t axiid:7;
    u_int64_t vfid:11;
    u_int64_t is_notify:1;
    u_int64_t size:9;
    /* u_int64_t [1] addr_lo:15 */
    u_int64_t addr:52;
    u_int64_t aspace:1;
    u_int64_t pmti:10;
    u_int64_t pmthit:1;
    u_int64_t reason:5;
    u_int64_t is_host:1;
    u_int64_t axilen:4;
    u_int64_t rsrv:3;
    u_int64_t tlpeop:1;
    u_int64_t tlpsop:1;
} __attribute__((packed)) indirect_info_t;

#endif /* __INDIRECT_H__ */
