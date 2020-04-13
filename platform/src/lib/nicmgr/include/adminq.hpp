/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __ADMINQ_HPP__
#define __ADMINQ_HPP__

#include <cstdint>

#include "nic/include/adminq.h"
#include "nic/sdk/platform/evutils/include/evutils.h"

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"

#include "pd_client.hpp"
#include "pal_compat.hpp"
#include "nicmgr_utils.hpp"

#define ADMINQ_TIMEOUT 5


typedef void (*adminq_cb_t) ( void *obj,
    void *req, void *req_data, void *resp, void *resp_data
);


class AdminQ {
public:
    AdminQ(
        const char *name,
        PdClient *pd,
        uint16_t lif,
        uint8_t req_qtype, uint32_t req_qid, uint16_t req_ring_size,
        uint8_t resp_qtype, uint32_t resp_qid, uint16_t resp_ring_size,
        adminq_cb_t handler, void *handler_obj, EV_P_
        bool response_enabled = true
    );

    bool Init(uint8_t cos_sel, uint8_t cosA, uint8_t cosB);
    bool Reset();

    bool PollRequest(struct nicmgr_req_desc *req);
    bool PostResponse(struct nicmgr_resp_desc *resp);
    void Flush(void);
    static void Poll(void *obj);

private:
    const char *name;
    PdClient *pd;

    uint16_t lif;
    uint8_t req_qtype, resp_qtype;
    uint32_t req_qid, resp_qid;
    uint16_t req_ring_size, resp_ring_size;
    bool req_init, resp_init;

    adminq_cb_t handler;
    void *handler_obj;
    bool response_enabled;

    // AdminRequestQ
    bool AdminRequestQInit(uint8_t cos_sel, uint8_t cosA, uint8_t cosB);
    bool AdminRequestQReset();
    void ProcessRequest(struct nicmgr_req_desc *req);
    uint16_t req_head;
    uint16_t req_tail;
    uint64_t req_ring_base;
    uint16_t req_comp_tail;
    uint64_t req_comp_base;
    uint8_t req_exp_color;

    // AdminResponseQ
    bool AdminResponseQInit(uint8_t cos_sel, uint8_t cosA, uint8_t cosB);
    bool AdminResponseQReset();
    uint16_t resp_head;
    uint16_t resp_tail;
    uint64_t resp_ring_base;
    uint16_t resp_comp_tail;
    uint64_t resp_comp_base;
    uint8_t resp_exp_color;

    // Tasks
    EV_P;
    evutil_prepare adminq_prepare = {0};
    evutil_check adminq_check = {0};
    evutil_timer adminq_timer = {0};
};

#endif  /* __ADMINQ_HPP__ */
