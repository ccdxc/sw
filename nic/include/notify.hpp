// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __NOTIFY_H__
#define __NOTIFY_H__

#include <stdint.h>

#pragma pack(push, 1)

struct notify_sta_qstate {
    uint8_t     rsvd1:7;
    uint8_t     host_color:1;
};

struct notify_cfg_qstate {
    uint8_t     rsvd2:5;
    uint8_t     intr_enable:1;
    uint8_t     host_queue:1;
    uint8_t     enable:1;
};

typedef struct notify_qstate {
    uint8_t     pc_offset;
    uint8_t     rsvd0;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     host : 4;
    uint8_t     total : 4;
    uint16_t    pid;

    uint16_t    p_index0;
    uint16_t    c_index0;
    uint16_t    host_pindex;

    struct notify_sta_qstate sta;

    struct notify_cfg_qstate cfg;

    uint64_t    ring_base;
    uint16_t    ring_size;

    uint64_t    host_ring_base;
    uint16_t    host_ring_size;
    uint16_t    host_intr_assert_index;

    uint8_t     __pad[26];

} notify_qstate_t;

static_assert (sizeof(struct notify_qstate) == 64, "");

struct notify_event_desc {
    uint8_t data[63];
    uint8_t color : 1;
    uint8_t rsvd0 : 7;
};

static_assert(sizeof(struct notify_event_desc) == 64, "");

#pragma pack(pop)

#endif
