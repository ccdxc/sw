/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "adminq.hpp"
#include "logger.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/sdk/asic/pd/pd.hpp"

AdminQ::AdminQ(const char *name, PdClient *pd, uint16_t lif, uint8_t req_qtype, uint32_t req_qid,
               uint16_t req_ring_size, uint8_t resp_qtype, uint32_t resp_qid,
               uint16_t resp_ring_size, adminq_cb_t handler, void *handler_obj,
               EV_P_ bool response_enabled)
    : name(name), pd(pd), lif(lif), req_qtype(req_qtype), resp_qtype(resp_qtype), req_qid(req_qid),
      resp_qid(resp_qid), req_ring_size(req_ring_size), resp_ring_size(resp_ring_size),
      handler(handler), handler_obj(handler_obj), response_enabled(response_enabled)
{
    this->loop = loop;

    if (req_ring_size & (req_ring_size - 1)) {
        NIC_LOG_ERR("{}: Request ring size has to be power of 2", name);
        throw;
    }

    if (resp_ring_size & (resp_ring_size - 1)) {
        NIC_LOG_ERR("{}: Response ring size has to be power of 2", name);
        throw;
    }

    /* alloc request queue resources */
    req_ring_base = pd->nicmgr_mem_alloc(sizeof(struct nicmgr_req_desc) * req_ring_size);
    if (req_ring_base == 0) {
        NIC_LOG_ERR("{}: Failed to get allocate request queue", name);
        throw;
    }
    NIC_LOG_DEBUG("{}: request queue address {:#x}", name, req_ring_base);

    req_comp_base = pd->nicmgr_mem_alloc(sizeof(struct nicmgr_req_comp_desc) * req_ring_size);
    if (req_comp_base == 0) {
        NIC_LOG_ERR("{}: Failed to get allocate request queue", name);
        throw;
    }
    NIC_LOG_DEBUG("{}: request completion queue address {:#x}", name, req_comp_base);

    /* allocate response queue resources */
    resp_ring_base = pd->nicmgr_mem_alloc(sizeof(struct nicmgr_resp_desc) * resp_ring_size);
    if (resp_ring_base == 0) {
        NIC_LOG_ERR("{}: Failed to get allocate response queue", name);
        throw;
    }
    NIC_LOG_DEBUG("{}: response queue address {:#x}", name, resp_ring_base);

    resp_comp_base = pd->nicmgr_mem_alloc(sizeof(struct nicmgr_resp_comp_desc) * resp_ring_size);
    if (resp_comp_base == 0) {
        NIC_LOG_ERR("{}: Failed to get allocate response completion queue", name);
        throw;
    }
    NIC_LOG_DEBUG("{}: response completion queue address {:#x}", name, resp_comp_base);

    req_init = false;
    resp_init = false;
}

bool
AdminQ::Init(uint8_t cos_sel, uint8_t cosA, uint8_t cosB)
{
    if (!AdminRequestQInit(cos_sel, cosA, cosB)) {
        NIC_LOG_ERR("{}: Failed to initialize request queue", name);
        return false;
    }

    if (!AdminResponseQInit(cos_sel, cosA, cosB)) {
        NIC_LOG_ERR("{}: Failed to initialize response queue", name);
        return false;
    }

    evutil_add_prepare(EV_A_ & adminq_prepare, AdminQ::Poll, this);
    evutil_add_check(EV_A_ & adminq_check, AdminQ::Poll, this);
    evutil_timer_start(EV_A_ & adminq_timer, AdminQ::Poll, this, 0.0, 0.001);

    return true;
}

bool
AdminQ::AdminRequestQInit(uint8_t cos_sel, uint8_t cosA, uint8_t cosB)
{
    nicmgr_req_qstate_t qstate = {0};

    req_head = req_ring_size - 1;
    req_tail = 0;
    req_comp_tail = 0;
    req_exp_color = 1;

    NIC_LOG_INFO("{}: Initializing AdminRequestQ lif {} qtype {} qid {}",
        name, lif, req_qtype, req_qid);

    // Init rings
    MEM_SET(req_ring_base, 0, sizeof(struct nicmgr_req_desc) * req_ring_size, 0);
    MEM_SET(req_comp_base, 0, sizeof(struct nicmgr_req_comp_desc) * req_ring_size, 0);

    // Init Qstate
    uint64_t req_qstate_addr = pd->lm_->get_lif_qstate_addr(lif, req_qtype, req_qid);
    if (req_qstate_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for request queue", name);
        throw;
    }
    NIC_LOG_DEBUG("{}: initializing request qstate {:#x}", name, req_qstate_addr);

    uint8_t off;
    if (pd->get_pc_offset("txdma_stage0.bin", "nicmgr_req_stage0", &off, NULL) < 0) {
        NIC_LOG_ERR(
            "Failed to get PC offset of program: txdma_stage0.bin label: nicmgr_req_stage0");
        return false;
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = 0;
    qstate.cosB = cosB;
    qstate.host = 0;
    qstate.total = 1;
    qstate.p_index0 = req_head;
    qstate.c_index0 = req_tail;
    qstate.comp_index = req_comp_tail;
    qstate.ci_fetch = 0;
    qstate.sta.color = req_exp_color;
    qstate.cfg.enable = 1;
    qstate.ring_base = req_ring_base;
    qstate.ring_size = log2(req_ring_size);
    qstate.cq_ring_base = req_comp_base;

    WRITE_MEM(req_qstate_addr, (uint8_t *)&qstate, sizeof(nicmgr_req_qstate_t), 0);

    PAL_barrier();
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(req_qstate_addr, sizeof(nicmgr_req_qstate_t),
                                                  P4PLUS_CACHE_INVALIDATE_TXDMA);

    req_init = true;

    return true;
}

bool
AdminQ::AdminResponseQInit(uint8_t cos_sel, uint8_t cosA, uint8_t cosB)
{
    nicmgr_resp_qstate_t qstate = {0};

    resp_head = 0;
    resp_tail = 0;
    resp_comp_tail = 0;
    resp_exp_color = 1;

    NIC_LOG_INFO("{}: Initializing AdminResponseQ lif {} qtype {} qid {}",
        name, lif, resp_qtype, resp_qid);

    // Init rings
    MEM_SET(resp_ring_base, 0, sizeof(struct nicmgr_resp_desc) * resp_ring_size, 0);
    MEM_SET(resp_comp_base, 0, sizeof(struct nicmgr_resp_comp_desc) * resp_ring_size, 0);

    // Init Qstate
    uint64_t resp_qstate_addr = pd->lm_->get_lif_qstate_addr(lif, resp_qtype, resp_qid);
    if (resp_qstate_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for response queue", name);
        return false;
    }
    NIC_LOG_DEBUG("{}: initializing response qstate {:#x}", name, resp_qstate_addr);

    uint8_t off;
    if (pd->get_pc_offset("txdma_stage0.bin", "nicmgr_resp_stage0", &off, NULL) < 0) {
        NIC_LOG_ERR(
            "Failed to get PC offset of program: txdma_stage0.bin label: nicmgr_resp_stage0");
        return false;
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = 0;
    qstate.cosB = cosB;
    qstate.host = 0;
    qstate.total = 1;
    qstate.p_index0 = resp_head;
    qstate.c_index0 = resp_tail;
    qstate.comp_index = resp_comp_tail;
    qstate.ci_fetch = 0;
    qstate.sta.color = resp_exp_color;
    qstate.cfg.enable = 1;
    qstate.ring_base = resp_ring_base;
    qstate.ring_size = log2(resp_ring_size);
    qstate.cq_ring_base = resp_comp_base;

    WRITE_MEM(resp_qstate_addr, (uint8_t *)&qstate, sizeof(nicmgr_resp_qstate_t), 0);

    PAL_barrier();
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(resp_qstate_addr, sizeof(nicmgr_resp_qstate_t),
                                                  P4PLUS_CACHE_INVALIDATE_TXDMA);

    resp_init = true;

    return true;
}

bool
AdminQ::Reset()
{
    if (!AdminRequestQReset()) {
        NIC_LOG_ERR("{}: Failed to reset request queue", name);
        return false;
    }

    if (!AdminResponseQReset()) {
        NIC_LOG_ERR("{}: Failed to reset response queue", name);
        return false;
    }

    evutil_remove_prepare(EV_A_ & adminq_prepare);
    evutil_remove_check(EV_A_ & adminq_check);
    evutil_timer_stop(EV_A_ & adminq_timer);

    return true;
}

bool
AdminQ::AdminRequestQReset()
{
    uint64_t req_qstate_addr;
    uint64_t db_data;
    asic_db_addr_t db_addr = { 0 };

    if (!req_init)
        return true;

    req_qstate_addr = pd->lm_->get_lif_qstate_addr(lif, req_qtype, req_qid);
    if (req_qstate_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for request queue", name);
        return false;
    }
    NIC_LOG_DEBUG("{}: resetting request qstate {:#x}", name, req_qstate_addr);

    MEM_SET(req_qstate_addr, 0, fldsiz(nicmgr_req_qstate_t, pc_offset), 0);
    PAL_barrier();
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(req_qstate_addr, sizeof(nicmgr_req_qstate_t),
                                                  P4PLUS_CACHE_INVALIDATE_TXDMA);

    db_addr.lif_id = lif;
    db_addr.upd = ASIC_DB_ADDR_UPD_FILL(ASIC_DB_UPD_SCHED_NONE,
                                        ASIC_DB_UPD_INDEX_UPDATE_NONE, false);
    db_addr.q_type = req_qtype;

    db_data = (req_qid << 24);
    sdk::asic::pd::asic_ring_db(&db_addr, db_data);

    req_init = false;

    return true;
}

bool
AdminQ::AdminResponseQReset()
{
    uint64_t resp_qstate_addr;
    uint64_t db_data;
    asic_db_addr_t db_addr = { 0 };

    if (!resp_init)
        return true;

    resp_qstate_addr = pd->lm_->get_lif_qstate_addr(lif, resp_qtype, resp_qid);
    if (resp_qstate_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for response queue", name);
        return false;
    }
    NIC_LOG_DEBUG("{}: resetting response qstate {:#x}", name, resp_qstate_addr);

    MEM_SET(resp_qstate_addr, 0, fldsiz(nicmgr_resp_qstate_t, pc_offset), 0);
    PAL_barrier();
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(resp_qstate_addr, sizeof(nicmgr_resp_qstate_t),
                                                  P4PLUS_CACHE_INVALIDATE_TXDMA);

    db_addr.lif_id = lif;
    db_addr.q_type = resp_qtype;
    db_addr.upd = ASIC_DB_ADDR_UPD_FILL(ASIC_DB_UPD_SCHED_NONE,
                                        ASIC_DB_UPD_INDEX_UPDATE_NONE, false);

    db_data = resp_qid << 24;
    sdk::asic::pd::asic_ring_db(&db_addr, db_data);

    resp_init = false;

    return true;
}

bool
AdminQ::PollRequest(struct nicmgr_req_desc *req_desc)
{
    uint64_t req_db_data, addr;
    asic_db_addr_t db_addr = { 0 };
    struct nicmgr_req_comp_desc comp = {0};

    addr = req_comp_base + req_comp_tail * sizeof(struct nicmgr_req_comp_desc);
    READ_MEM(addr, (uint8_t *)&comp, sizeof(struct nicmgr_req_comp_desc), 0);

    if (comp.color == req_exp_color) {

        // Advance cq tail and update expected color if end or ring
        req_comp_tail = (req_comp_tail + 1) % req_ring_size;
        if (req_comp_tail == 0) {
            req_exp_color = req_exp_color ? 0 : 1;
        }

        // Read request descriptor
        uint64_t req_desc_addr = req_ring_base + (sizeof(*req_desc) * req_tail);
        READ_MEM(req_desc_addr, (uint8_t *)req_desc, sizeof(*req_desc), 0);

        NIC_LOG_DEBUG("{}: request lif {} qtype {} qid {} comp_index {}"
                      " adminq_qstate_addr {:#x} desc_addr {:#x} req_head: {:#x} req_tail: {:#x}",
                      name, req_desc->lif, req_desc->qtype, req_desc->qid, req_desc->comp_index,
                      req_desc->adminq_qstate_addr, req_desc_addr, req_head, req_tail);

        req_head = (req_head + 1) & (req_ring_size - 1);
        req_tail = (req_tail + 1) & (req_ring_size - 1);

        // Ring doorbell to update the PI
        db_addr.lif_id = lif;
        db_addr.q_type = req_qtype;
        db_addr.upd = ASIC_DB_ADDR_UPD_FILL(ASIC_DB_UPD_SCHED_NONE,
                                            ASIC_DB_UPD_INDEX_SET_PINDEX,
                                            false);

        req_db_data = (req_qid << 24) | (0 << 16) | req_head;
        PAL_barrier();

        sdk::asic::pd::asic_ring_db(&db_addr, req_db_data);

        return true;
    }

    return false;
}

bool
AdminQ::PostResponse(struct nicmgr_resp_desc *resp_desc)
{
    uint64_t resp_db_data;
    asic_db_addr_t db_addr = { 0 };
    // Write response descriptor
    uint64_t resp_desc_addr = resp_ring_base + (sizeof(*resp_desc) * resp_tail);
    WRITE_MEM(resp_desc_addr, (uint8_t *)resp_desc, sizeof(*resp_desc), 0);

    NIC_LOG_DEBUG("{}: response lif {} qtype {} qid {} comp_index {}"
                  " adminq_qstate_addr {:#x} desc_addr {:#x} resp_tail: {:#x}",
                  name, resp_desc->lif, resp_desc->qtype, resp_desc->qid, resp_desc->comp_index,
                  resp_desc->adminq_qstate_addr, resp_desc_addr, resp_tail);

    resp_tail = (resp_tail + 1) & (resp_ring_size - 1);

    // Ring doorbell to update the PI and run response program
    db_addr.lif_id = lif;
    db_addr.q_type = resp_qtype;
    db_addr.upd = ASIC_DB_ADDR_UPD_FILL(ASIC_DB_UPD_SCHED_SET,
                                        ASIC_DB_UPD_INDEX_SET_PINDEX, false);

    resp_db_data = (resp_qid << 24) | (0 << 16) | resp_tail;

    PAL_barrier();
    sdk::asic::pd::asic_ring_db(&db_addr, resp_db_data);

    return true;
}

void
AdminQ::ProcessRequest(struct nicmgr_req_desc *req_desc)
{
    AdminQ *adminq = (AdminQ *)this;
    uint8_t req_data[4096] = {0};
    struct nicmgr_resp_desc resp_desc = {0};
    uint8_t resp_data[4096] = {0};
    timespec_t start_ts, end_ts, ts_diff;

    resp_desc.lif = req_desc->lif;
    resp_desc.qtype = req_desc->qtype;
    resp_desc.qid = req_desc->qid;
    resp_desc.adminq_qstate_addr = req_desc->adminq_qstate_addr;

    NIC_HEADER_TRACE("AdminCmd");
    clock_gettime(CLOCK_MONOTONIC, &start_ts);

    if (adminq->handler) {
        adminq->handler(adminq->handler_obj, &req_desc->cmd, &req_data, &resp_desc.comp,
                        &resp_data);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    ts_diff = sdk::timestamp_diff(&end_ts, &start_ts);
    NIC_LOG_DEBUG("AdminCmd Time taken: {}s.{}ns", ts_diff.tv_sec, ts_diff.tv_nsec);
    if (ts_diff.tv_sec >= ADMINQ_TIMEOUT) {
        NIC_LOG_WARN("AdminCmd took more than {} secs", ADMINQ_TIMEOUT);
    }
    NIC_HEADER_TRACE("AdminCmd End");

    if (adminq->response_enabled) {
        adminq->PostResponse(&resp_desc);
    }
}

void
AdminQ::Flush()
{
    AdminQ *adminq = (AdminQ *)this;
    struct nicmgr_req_desc req_desc = {0};

    NIC_HEADER_TRACE("Starting AdminCmd Flush");
    while (adminq->PollRequest(&req_desc)) {
        adminq->ProcessRequest(&req_desc);
    }
    NIC_HEADER_TRACE("Ending AdminCmd Flush");
}

void
AdminQ::Poll(void *obj)
{
    AdminQ *adminq = (AdminQ *)obj;
    struct nicmgr_req_desc req_desc = {0};

    if (adminq->PollRequest(&req_desc)) {
        adminq->ProcessRequest(&req_desc);
    }
}
