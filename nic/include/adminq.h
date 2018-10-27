// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __ADMINQ_H__
#define __ADMINQ_H__

#pragma pack(push, 1)

struct admin_sta_qstate {
    uint8_t     rsvd1:7;
    uint8_t     color:1;
};

struct admin_cfg_qstate {
    uint8_t     rsvd2:5;
    uint8_t     intr_enable:1;
    uint8_t     host_queue:1;
    uint8_t     enable:1;
};

typedef struct admin_qstate {
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
    uint16_t    comp_index;
    uint16_t    ci_fetch;

    struct admin_sta_qstate sta;

    struct admin_cfg_qstate cfg;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
    uint16_t    intr_assert_index;
    uint64_t    nicmgr_qstate_addr;

    uint8_t     __pad[18 + 64];

} admin_qstate_t;

static_assert (sizeof(struct admin_qstate) == 128, "");

struct nicmgr_sta_qstate {
    uint8_t     rsvd1:7;
    uint8_t     color:1;
};

struct nicmgr_cfg_qstate {
    uint8_t     rsvd2:6;
    uint8_t     intr_enable:1;
    uint8_t     enable:1;
};

struct nicmgr_qstate {
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
    uint16_t    comp_index;
    uint16_t    ci_fetch;

    struct nicmgr_sta_qstate sta;

    struct nicmgr_cfg_qstate cfg;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
    uint16_t    intr_assert_index;

    uint8_t     __pad[26 + 64];
};

static_assert (sizeof(struct nicmgr_qstate) == 128, "");

typedef struct nicmgr_qstate nicmgr_req_qstate_t;
typedef struct nicmgr_qstate nicmgr_resp_qstate_t;

#pragma pack(pop)

#endif
