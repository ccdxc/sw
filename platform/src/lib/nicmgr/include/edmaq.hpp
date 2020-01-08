/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __EDMAQ_HPP__
#define __EDMAQ_HPP__

#include <cstdint>
#include <ev.h>

#include "nic/include/edmaq.h"
#include "nic/sdk/platform/evutils/include/evutils.h"

#include "pd_client.hpp"
#include "pal_compat.hpp"
#include "nicmgr_utils.hpp"

#define EDMAQ_COMP_POLL_S           (0.001) // 1 ms
#define EDMAQ_COMP_TIMEOUT_S        (0.01)  // 10 ms
#define EDMAQ_MAX_TRANSFER_SZ       (((1U << 14) - 1) & (~63U))

typedef void (*edmaq_cb_t) (void *obj);

struct edmaq_ctx {
    edmaq_cb_t cb;
    void *obj;
    ev_tstamp deadline;
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
    bool Debug(bool enable);

    bool Post(edma_opcode opcode, uint64_t from, uint64_t to, uint16_t size,
        struct edmaq_ctx *ctx);
    static void PollCb(void *obj);

private:
    const char *name;
    PdClient *pd;
    bool skip_hwinit;
    bool init;

    uint16_t lif;
    uint8_t qtype;
    uint32_t qid;
    uint16_t ring_size;

    uint16_t head;
    uint16_t tail;
    uint64_t ring_base;
    uint16_t comp_tail;
    uint64_t comp_base;
    uint8_t exp_color;

    struct edmaq_ctx *pending;

    // Tasks
    EV_P;
    evutil_prepare prepare = {0};

    bool Empty();
    bool Poll();
    void Flush();
};

#endif    /* __EDMAQ_HPP__ */
