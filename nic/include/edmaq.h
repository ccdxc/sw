// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __EDMA_H__
#define __EDMA_H__

#include <stdint.h>

#pragma pack(push, 1)

struct edma_sta_qstate {
    uint8_t     rsvd1:7;
    uint8_t     color:1;
};

struct edma_cfg_qstate {
    uint8_t     rsvd2:5;
    uint8_t     debug:1;
    uint8_t     intr_enable:1;
    uint8_t     enable:1;
};

typedef struct edma_qstate {
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

    struct edma_sta_qstate sta;

    struct edma_cfg_qstate cfg;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
    uint16_t    intr_assert_index;

    uint8_t     __pad[28];

} edma_qstate_t;

static_assert (sizeof(struct edma_qstate) == 64, "");

enum edma_opcode {
    EDMA_OPCODE_HOST_TO_LOCAL = 0,
    EDMA_OPCODE_LOCAL_TO_HOST = 1,
    EDMA_OPCODE_LOCAL_TO_LOCAL = 2,
    EDMA_OPCODE_HOST_TO_HOST = 3
};

struct edma_cmd_desc {
    uint8_t opcode;
    uint16_t len;
    uint16_t src_lif;
    uint64_t src_addr;
    uint16_t dst_lif;
    uint64_t dst_addr;
    uint8_t  rsvd0[9];
};

struct edma_comp_desc {
    uint8_t status;
    uint8_t rsvd;
    uint16_t comp_index;
    uint8_t rsvd1[11];
    uint8_t rsvd2:7;
    uint8_t color:1;
};

static_assert(sizeof(struct edma_cmd_desc) == 32, "");
static_assert(sizeof(struct edma_comp_desc) == 16, "");

#pragma pack(pop)

#endif
