/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#include "edmaq.hpp"
#include "logger.hpp"

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"

#include "nic/sdk/platform/capri/capri_common.hpp"


EdmaQ::EdmaQ(
    const char *name,
    PdClient *pd,
    uint16_t lif,
    uint8_t qtype, uint32_t qid, uint16_t ring_size, EV_P
) :
    name(name),
    pd(pd),
    lif(lif),
    qtype(qtype), qid(qid), ring_size(ring_size)
{
    this->loop = loop;
    skip_hwinit = pd->is_queue_hwinit_done(lif, qtype, qid);

    if (ring_size & (ring_size - 1)) {
        NIC_LOG_ERR("{}: Ring size has to be power of 2", name);
        throw;
    }

    head = 0;
    tail = 0;
    ring_base = pd->nicmgr_mem_alloc((sizeof(struct edma_cmd_desc) * ring_size));
    if (ring_base == 0) {
        NIC_LOG_ERR("{}: Failed to allocate edma ring!", name);
        throw;
    }
    MEM_CLR(ring_base, 0, (sizeof(struct edma_cmd_desc) * ring_size), skip_hwinit);

    comp_tail = 0;
    exp_color = 1;
    comp_base = pd->nicmgr_mem_alloc((sizeof(struct edma_comp_desc) * ring_size));
    if (comp_base == 0) {
        NIC_LOG_ERR("{}: Failed to allocate edma completion ring!", name);
        throw;
    }
    MEM_CLR(comp_base, 0, (sizeof(struct edma_comp_desc) * ring_size), skip_hwinit);

    NIC_LOG_INFO("{}: edma_ring_base {:#x} edma_comp_base {:#x}",
        name, ring_base, comp_base);

    pending = (struct edmaq_ctx *)calloc(1, sizeof(struct edmaq_ctx) * ring_size);

    init = false;
}

bool
EdmaQ::Init(uint8_t cos_sel, uint8_t cosA, uint8_t cosB)
{
    edma_qstate_t qstate = {0};

    head = 0;
    tail = 0;
    comp_tail = 0;
    exp_color = 1;

    NIC_LOG_INFO("{}: Initializing edmaq lif {} qtype {} qid {}",
        name, lif, qtype, qid);

    // Init rings
    MEM_SET(ring_base, 0, (sizeof(struct edma_cmd_desc) * ring_size), 0);
    MEM_SET(comp_base, 0, (sizeof(struct edma_comp_desc) * ring_size), 0);

    // Init Qstate
    uint64_t addr = pd->lm_->get_lif_qstate_addr(lif, qtype, qid);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for edma queue",
            name);
        return false;
    }

    NIC_LOG_DEBUG("{}: Initializing edma qstate {:#x}", name, addr);

    uint8_t off;
    if (pd->get_pc_offset("txdma_stage0.bin", "edma_stage0", &off, NULL) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: edma_stage0");
        return false;
    }
    qstate.pc_offset = off;
    qstate.cos_sel = cos_sel;
    qstate.cosA = cosA;
    qstate.cosB = cosB;
    qstate.host = 0;
    qstate.total = 1;
    qstate.pid = 0;
    qstate.p_index0 = head;
    qstate.c_index0 = 0;
    qstate.comp_index = comp_tail;
    qstate.sta.color = exp_color;
    qstate.cfg.enable = 1;
    qstate.ring_base = ring_base;
    qstate.ring_size = log2(ring_size);
    qstate.cq_ring_base = comp_base;
    qstate.cfg.intr_enable = 0;
    qstate.intr_assert_index = 0;
    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(edma_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);

    init = true;

    return true;
}

bool
EdmaQ::Reset()
{
    uint64_t addr, req_db_addr;

    NIC_LOG_INFO("{}: Resetting edmaq lif {} qtype {} qid {}",
        name, lif, qtype, qid);

    if (!init)
        return true;

    addr = pd->lm_->get_lif_qstate_addr(lif, qtype, qid);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for edma queue",
            name);
        return false;
    }
    NIC_LOG_DEBUG("{}: Resetting edma qstate {:#x}", name, addr);

    MEM_SET(addr, 0, fldsiz(edma_qstate_t, pc_offset), 0);
    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(edma_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);

    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b0 << 17) +
                (lif << 6) +
                (qtype << 3);

    WRITE_DB64(req_db_addr, qid << 24);

    init = false;

    return true;
}

bool
EdmaQ::Debug(bool enable)
{
    struct edma_cfg_qstate cfg = {0};

    uint64_t addr = pd->lm_->get_lif_qstate_addr(lif, qtype, qid);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for edma queue",
            name);
        return false;
    }

    READ_MEM(addr + offsetof(struct edma_qstate, cfg),
        (uint8_t *)&cfg, sizeof(cfg), 0);
    cfg.debug = enable;
    WRITE_MEM(addr + offsetof(struct edma_qstate, cfg),
        (uint8_t *)&cfg, sizeof(cfg), 0);

    p4plus_invalidate_cache(addr, sizeof(edma_qstate_t),
        P4PLUS_CACHE_INVALIDATE_TXDMA);

    return true;
}

bool
EdmaQ::Post(edma_opcode opcode, uint64_t from, uint64_t to, uint16_t size,
    struct edmaq_ctx *ctx)
{
    uint64_t addr, req_db_addr;
    auto offset = 0;
    auto chunk_sz = (size < EDMAQ_MAX_TRANSFER_SZ) ? size : EDMAQ_MAX_TRANSFER_SZ;
    auto transfer_sz = 0;
    auto bytes_left = size;
    struct edma_cmd_desc cmd = {0};

    if (!init)
        return false;

    /* MS only supports reading/writing 64B per DMA transaction */
    if (from <= CAPRI_HBM_BASE || to <= CAPRI_HBM_BASE) {
        chunk_sz = (chunk_sz < 64) ? chunk_sz : 64;
    }

    while (bytes_left > 0) {

        transfer_sz = (bytes_left <= chunk_sz) ? bytes_left : chunk_sz;

        auto ring_full = [](uint16_t h, uint16_t t, uint16_t sz) -> bool {
            return (((h + 1) % sz) == t);
        };

        /* If the ring is full, then do a blocking wait for a completion */
        if (ring_full(head, tail, ring_size)) {
            while (!Poll()) {
                ev_sleep(EDMAQ_COMP_POLL_S);
                ev_now_update(EV_A);
            }
            /* Blocking wait for completion above should have made space in the ring.
               So we should not hit the following condition at all */
            if (ring_full(head, tail, ring_size)) {
                NIC_LOG_ERR("{}: EDMA queue full head {} tail {}", name, head, tail);
                return false;
            }
        }

        /* If this is the last chunk then set the completion callback */
        if (bytes_left <= chunk_sz && ctx)
            pending[head] = *ctx;
        else
            pending[head] = {0};
        pending[head].deadline = ev_now(EV_A) + EDMAQ_COMP_TIMEOUT_S;

        /* enqueue edma command */
        addr = ring_base + head * sizeof(struct edma_cmd_desc);
        cmd = {0};
        cmd.opcode = opcode;
        cmd.len = transfer_sz;
        cmd.src_lif = lif;
        cmd.src_addr = from + offset;
        cmd.dst_lif = lif;
        cmd.dst_addr = to + offset;
        WRITE_MEM(addr, (uint8_t *)&cmd, sizeof(struct edma_cmd_desc), 0);
        head = (head + 1) % ring_size;

        offset += transfer_sz;
        bytes_left -= transfer_sz;
    }

    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1011 /* PI_UPD + SCHED_SET */ << 17) +
                (lif << 6) +
                (qtype << 3);

    PAL_barrier();
    WRITE_DB64(req_db_addr, (qid << 24) | head);

    if (ctx == NULL) {
        Flush();
    } else {
        if (ctx->cb)
            evutil_add_prepare(EV_A_ &prepare, EdmaQ::PollCb, this);
    }

    return true;
}

void
EdmaQ::PollCb(void *obj)
{
    EdmaQ *edmaq = (EdmaQ *)obj;
    edmaq->Poll();
}

bool
EdmaQ::Poll()
{
    struct edma_comp_desc comp = {0};
    uint64_t addr = comp_base + comp_tail * sizeof(struct edma_comp_desc);
    ev_tstamp now = ev_now(EV_A);
    bool timeout = (pending[tail].deadline < now);

    READ_MEM(addr, (uint8_t *)&comp, sizeof(struct edma_comp_desc), 0);
    if (comp.color == exp_color || timeout) {
        if (pending[tail].cb) {
            pending[tail].cb(pending[tail].obj);
            memset(&pending[tail], 0, sizeof(pending[tail]));
        }
        comp_tail = (comp_tail + 1) % ring_size;
        if (comp_tail == 0) {
            exp_color = exp_color ? 0 : 1;
        }
        tail = (tail + 1) % ring_size;
        if (Empty()) {
            evutil_remove_prepare(EV_A_ &prepare);
        }
        return true;
    }

    return false;
}

bool
EdmaQ::Empty()
{
    return (head == tail);
}

void
EdmaQ::Flush()
{
    while (!Empty()) {
        Poll();
        ev_sleep(EDMAQ_COMP_POLL_S);
        ev_now_update(EV_A);
    };
}
