/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __EDMAQ_HPP__
#define __EDMAQ_HPP__

#include <cstdint>

#include "nic/include/edmaq.h"
#include "nic/sdk/platform/evutils/include/evutils.h"

#include "pd_client.hpp"
#include "pal_compat.hpp"
#include "nicmgr_utils.hpp"

#define EDMAQ_COMP_POLL_US          (1000)
#define EDMAQ_COMP_POLL_MAX         (10)

typedef void (*edmaq_cb_t) (void *obj);

struct edmaq_ctx {
    edmaq_cb_t cb;
    void *obj;
};

class EdmaQ {
public:
    EdmaQ(
        const char *name,
        PdClient *pd,
        uint16_t lif,
        uint8_t qtype, uint32_t qid, uint16_t ring_size,
        EV_P
    );

    bool Init(uint8_t cos_sel, uint8_t cosA, uint8_t cosB);
    bool Reset();

    bool Post(edma_opcode opcode, uint64_t from, uint64_t to, uint16_t size,
        struct edmaq_ctx *ctx);
    static void Poll(void *obj);

private:
    const char *name;
    PdClient *pd;

    uint16_t lif;
    uint8_t qtype;
    uint32_t qid;
    uint16_t ring_size;

    uint16_t head;
    uint64_t ring_base;
    uint16_t comp_tail;
    uint64_t comp_base;
    uint8_t exp_color;

    struct edmaq_ctx *pending;

    // Tasks
    EV_P;
    evutil_prepare prepare = {0};
};

#endif    /* __EDMAQ_HPP__ */
