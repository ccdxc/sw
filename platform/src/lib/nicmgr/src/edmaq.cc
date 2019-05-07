/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#include "edmaq.hpp"
#include "logger.hpp"

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"


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

    if (ring_size & (ring_size - 1)) {
        NIC_LOG_ERR("{}: Ring size has to be power of 2", name);
        throw;
    }

    head = 0;
    ring_base = pd->nicmgr_mem_alloc((sizeof(struct edma_cmd_desc) * ring_size));
    if (ring_base == 0) {
        NIC_LOG_ERR("{}: Failed to allocate edma ring!", name);
        throw;
    }
    MEM_SET(ring_base, 0, (sizeof(struct edma_cmd_desc) * ring_size), 0);

    comp_tail = 0;
    exp_color = 1;
    comp_base = pd->nicmgr_mem_alloc((sizeof(struct edma_comp_desc) * ring_size));
    if (comp_base == 0) {
        NIC_LOG_ERR("{}: Failed to allocate edma completion ring!", name);
        throw;
    }
    MEM_SET(comp_base, 0, (sizeof(struct edma_comp_desc) * ring_size), 0);

    NIC_LOG_INFO("{}: edma_ring_base {:#x} edma_comp_base {:#x}",
        name, ring_base, comp_base);

    pending = (struct edmaq_ctx *)calloc(1, sizeof(struct edmaq_ctx) * ring_size);
}

bool
EdmaQ::Init(uint8_t cos_sel, uint8_t cosA, uint8_t cosB)
{
    edma_qstate_t qstate = {0};

    head = 0;
    comp_tail = 0;
    exp_color = 1;

    // Reset rings
    MEM_SET(ring_base, 0, (sizeof(struct edma_cmd_desc) * ring_size), 0);
    MEM_SET(comp_base, 0, (sizeof(struct edma_comp_desc) * ring_size), 0);

    // Init Qstate
    uint64_t addr = pd->lm_->get_lif_qstate_addr(lif, qtype, qid);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for edma queue",
            name);
        return false;
    }

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

    return true;
}

bool
EdmaQ::Reset()
{
    uint64_t addr = pd->lm_->get_lif_qstate_addr(lif, qtype, qid);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for edma queue",
            name);
        return false;
    }
    MEM_SET(addr, 0, fldsiz(edma_qstate_t, pc_offset), 0);
    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(edma_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);

    // Reset rings
    MEM_SET(ring_base, 0, (sizeof(struct edma_cmd_desc) * ring_size), 0);
    MEM_SET(comp_base, 0, (sizeof(struct edma_comp_desc) * ring_size), 0);

    return true;
}

bool
EdmaQ::Post(edma_opcode opcode, uint64_t from, uint64_t to, uint16_t size,
    struct edmaq_ctx *ctx)
{
    // Enqueue edma command
    struct edma_cmd_desc cmd = {
        .opcode = opcode,
        .len = size,
        .src_lif = lif,
        .src_addr = from,
        .dst_lif = lif,
        .dst_addr = to,
    };

    if (ctx != NULL)
        pending[head] = *ctx;
    else
        pending[head] = {0};

    uint64_t addr, req_db_addr;

    addr = ring_base + head * sizeof(struct edma_cmd_desc);
    WRITE_MEM(addr, (uint8_t *)&cmd, sizeof(struct edma_cmd_desc), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1011 /* PI_UPD + SCHED_SET */ << 17) +
                (lif << 6) +
                (qtype << 3);

    head = (head + 1) % ring_size;
    PAL_barrier();
    WRITE_DB64(req_db_addr, (qid << 24) | head);

    if (ctx == NULL) {  // Blocking wait for completion
        struct edma_comp_desc comp = {0};
        uint8_t npolls = 0;
        do {
            comp = {0};
            npolls = 0;
            addr = comp_base + comp_tail * sizeof(struct edma_comp_desc);
            do {
                READ_MEM(addr, (uint8_t *)&comp, sizeof(struct edma_comp_desc), 0);
                usleep(EDMAQ_COMP_POLL_US);
            } while (comp.color != exp_color &&
                    ++npolls < EDMAQ_COMP_POLL_MAX);

            if (npolls == EDMAQ_COMP_POLL_MAX) {
                NIC_LOG_ERR("{}: EDMA timeout opcode {} from {:#x} to {:#x} len {}",
                    name, opcode, from, to);
                return false;   // Fail return!
            } else {
                if (pending[comp.comp_index].cb) {
                    // Complete pending requests
                    pending[comp.comp_index].cb(pending[comp.comp_index].obj);
                }
                comp_tail = (comp_tail + 1) % ring_size;
                if (comp_tail == 0) {
                    exp_color = exp_color ? 0 : 1;
                }
            }
        } while(comp.comp_index != head);
    } else {
        evutil_add_prepare(EV_A_ &prepare, &EdmaQ::Poll, this);
    }

    return true;
}

void
EdmaQ::Poll(void *obj)
{
    EdmaQ *edmaq = (EdmaQ *)obj;
    struct edma_comp_desc comp = {0};
    uint64_t addr = edmaq->comp_base + edmaq->comp_tail * sizeof(struct edma_comp_desc);

    READ_MEM(addr, (uint8_t *)&comp, sizeof(struct edma_comp_desc), 0);
    if (comp.color == edmaq->exp_color) {
        if (edmaq->pending[comp.comp_index].cb != NULL) {
            // Complete pending requests
            edmaq->pending[comp.comp_index].cb(edmaq->pending[comp.comp_index].obj);
        }
        edmaq->comp_tail = (edmaq->comp_tail + 1) % edmaq->ring_size;
        if (edmaq->comp_tail == 0) {
            edmaq->exp_color = edmaq->exp_color ? 0 : 1;
        }
    }
}
