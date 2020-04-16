/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "nic/include/eth_common.h"
#include "nic/include/adminq.h"
#include "nic/include/edmaq.h"
#include "nic/include/notify.hpp"

#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "platform/src/lib/nicmgr/include/eth_if.h"

#include "gen/platform/mem_regions.hpp"

#include "impl.hpp"


static void
print_eth_qstate_intr(eth_qstate_intr_t *intr)
{
    printf("pc_offset=%#x\n"            "rsvd=%#x\n"
           "cosA=%#x\n"                 "cosB=%#x\n"
           "cos_sel=%#x\n"              "eval_last=%#x\n"
           "host=%#x\n"                 "total=%#x\n"
           "pid=%#x\n",
           intr->pc_offset,             intr->rsvd,
           intr->cosA,                  intr->cosB,
           intr->cos_sel,               intr->eval_last,
           intr->host,                  intr->total,
           intr->pid);
}

static void
print_eth_qstate_ring(eth_qstate_ring_t *ring, int i)
{
    printf("p_index%d=%#x\n"            "c_index%d=%#x\n",
           i, ring->p_index,            i, ring->c_index);
}

static void
print_eth_qstate_cfg(eth_qstate_cfg_t *cfg)
{
    printf("enable=%#x\n"               "debug=%#x\n"
           "host_queue=%#x\n"           "cpu_queue=%#x\n"
           "eq_enable=%#x\n"            "intr_enable=%#x\n"
           "rsvd_cfg=%#x\n",
           cfg->enable,                 cfg->debug,
           cfg->host_queue,             cfg->cpu_queue,
           cfg->eq_enable,              cfg->intr_enable,
           cfg->rsvd_cfg);
}

static void
print_eth_qstate_common(eth_qstate_common_t *q)
{
    print_eth_qstate_intr(&q->intr);
    print_eth_qstate_ring(&q->ring[0], 0);
    print_eth_qstate_ring(&q->ring[1], 1);
    print_eth_qstate_ring(&q->ring[2], 2);
    print_eth_qstate_cfg(&q->cfg);

    printf("rsvd_db_cnt=%#x\n"          "ring_size=%#x\n"
           "lif_index=%#x\n",
           q->rsvd_db_cnt,              q->ring_size,
           q->lif_index);
}

static void
print_eth_tx_qstate(eth_tx_qstate_t *tx)
{
    print_eth_qstate_common(&tx->q);

    printf(
           "comp_index=%#x\n"           "color=%#x\n"
           "armed=%#x\n"                "rsvd_sta=%#x\n"
           "lg2_desc_sz=%#x\n"          "lg2_cq_desc_sz=%#x\n"
           "lg2_sg_desc_sz=%#x\n"
           "ring_base=%#lx\n"           "cq_ring_base=%#lx\n"
           "sg_ring_base=%#lx\n"        "intr_index_or_eq_addr=%#lx\n",
           tx->comp_index,              tx->sta.color,
           tx->sta.armed,               tx->sta.rsvd,
           tx->lg2_desc_sz,             tx->lg2_cq_desc_sz,
           tx->lg2_sg_desc_sz,
           tx->ring_base,               tx->cq_ring_base,
           tx->sg_ring_base,            tx->intr_index_or_eq_addr);
}

static void
print_eth_tx2_qstate(eth_tx2_qstate_t *tx2)
{
    uint64_t tso0 = be64toh(tx2->tso_state[0]);
    uint64_t tso1 = be64toh(tx2->tso_state[1]);

    // BE bitfields not byte aligned, incompat with LE bitfields
#define EXTRACT_FIELD(v, low, width) \
    (((v) >> (low)) & ((1ul << (width)) - 1))

    printf("tso_hdr_addr=%#lx\n"
           "tso_hdr_len=%#lx\n"
           "tso_hdr_rsvd=%#lx\n"
           "tso_ipid_delta=%#lx\n"
           "tso_seq_delta=%#lx\n"
           "tso_rsvd=%#lx\n",
           EXTRACT_FIELD(tso0, 12, 52), // tso_hdr_addr
           EXTRACT_FIELD(tso0, 2, 10),  // tso_hdr_len
           EXTRACT_FIELD(tso0, 0, 2),   // tso_hdr_rsvd
           EXTRACT_FIELD(tso1, 48, 16), // tso_ipid_delta
           EXTRACT_FIELD(tso1, 16, 32), // tso_seq_delta
           EXTRACT_FIELD(tso1, 0, 16)); // tso_rsvd
}

static void
print_eth_rx_qstate(eth_rx_qstate_t *rx)
{
    print_eth_qstate_common(&rx->q);

    printf("comp_index=%#x\n"           "color=%#x\n"
           "armed=%#x\n"                "rsvd_sta=%#x\n"
           "lg2_desc_sz=%#x\n"          "lg2_cq_desc_sz=%#x\n"
           "lg2_sg_desc_sz=%#x\n"       "sg_max_elems=%#x\n"
           "ring_base=%#lx\n"           "cq_ring_base=%#lx\n"
           "sg_ring_base=%#lx\n"        "intr_index_or_eq_addr=%#lx\n",
           rx->comp_index,              rx->sta.color,
           rx->sta.armed,               rx->sta.rsvd,
           rx->lg2_desc_sz,             rx->lg2_cq_desc_sz,
           rx->lg2_sg_desc_sz,          rx->sg_max_elems,
           rx->ring_base,               rx->cq_ring_base,
           rx->sg_ring_base,            rx->intr_index_or_eq_addr);
}

void
eth_qstate(uint16_t lif, uint8_t qtype, uint32_t qid)
{
    union {
        eth_rx_qstate_t ethrx;
        eth_tx_co_qstate_t ethtx;
    } qstate = {0};
    struct admin_qstate qstate_ethaq = {0};
    notify_qstate_t qstate_notifyq = {0};
    edma_qstate_t qstate_edmaq = {0};
    nicmgr_req_qstate_t qstate_req = {0};
    nicmgr_resp_qstate_t qstate_resp = {0};
    queue_info_t qinfo[QTYPE_MAX] = {0};

    if (!get_lif_qstate(lif, qinfo)) {
        printf("Failed to get qinfo for lif %u\n", lif);
        return;
    }

    if (qinfo[qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", qtype, lif);
        return;
    }

    if (qid >= qinfo[qtype].length) {
        printf("Invalid qid %u for lif %u qtype %u\n", qid, lif, qtype);
        return;
    }

    uint64_t addr = qinfo[qtype].base + qid * qinfo[qtype].size;
    printf("\naddr: 0x%lx\n\n", addr);

    switch (qtype) {
    case 0:
        sdk::lib::pal_mem_read(addr,
                               (uint8_t *)&qstate.ethrx,
                               sizeof(qstate.ethrx));

        print_eth_rx_qstate(&qstate.ethrx);
        break;
    case 1:
        sdk::lib::pal_mem_read(addr,
                               (uint8_t *)&qstate.ethtx,
                               sizeof(qstate.ethtx));

        print_eth_tx_qstate(&qstate.ethtx.tx);
        print_eth_tx2_qstate(&qstate.ethtx.tx2);
        break;
    case 2:
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethaq, sizeof(qstate_ethaq));
        printf("pc_offset=0x%0x\n"
               "rsvd0=0x%0x\n"
               "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
               "eval_last=0x%0x\n"
               "host=0x%0x\ntotal=0x%0x\n"
               "pid=0x%0x\n"
               "p_index0=0x%0x\nc_index0=0x%0x\n"
               "comp_index=0x%0x\nci_fetch=0x%0x\n"
               "color=0x%0x\n"
               "enable=0x%0x\nhost_queue=0x%0x\nintr_enable=0x%0x\ndebug=0x%0x\n"
               "ring_base=0x%0lx\nring_size=0x%0x\n"
               "cq_ring_base=0x%0lx\nintr_assert_index=0x%0x\n"
               "nicmgr_qstate_addr=0x%0lx\n",
               qstate_ethaq.pc_offset, qstate_ethaq.rsvd0, qstate_ethaq.cosA, qstate_ethaq.cosB,
               qstate_ethaq.cos_sel, qstate_ethaq.eval_last, qstate_ethaq.host, qstate_ethaq.total,
               qstate_ethaq.pid, qstate_ethaq.p_index0, qstate_ethaq.c_index0,
               qstate_ethaq.comp_index, qstate_ethaq.ci_fetch, qstate_ethaq.sta.color,
               qstate_ethaq.cfg.enable, qstate_ethaq.cfg.host_queue, qstate_ethaq.cfg.intr_enable,
               qstate_ethaq.cfg.debug,
               qstate_ethaq.ring_base, qstate_ethaq.ring_size, qstate_ethaq.cq_ring_base,
               qstate_ethaq.intr_assert_index, qstate_ethaq.nicmgr_qstate_addr);
        break;
    case 7:
        if (qid == 0) {
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_notifyq, sizeof(qstate_notifyq));
            printf("pc_offset=0x%0x\n"
                   "rsvd0=0x%0x\n"
                   "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
                   "eval_last=0x%0x\n"
                   "host=0x%0x\ntotal=0x%0x\n"
                   "pid=0x%0x\n"
                   "p_index0=0x%0x\nc_index0=0x%0x\nhost_pindex=0x%0x\n"
                   "enable=0x%0x\nhost_queue=0x%0x\nintr_enable=0x%0x\ndebug=0x%0x\n"
                   "ring_base=0x%0lx\nring_size=0x%0x\n"
                   "host_ring_base=0x%0lx\nhost_ring_size=0x%0x\nhost_intr_assert_index=0x%0x\n",
                   qstate_notifyq.pc_offset, qstate_notifyq.rsvd0, qstate_notifyq.cosA,
                   qstate_notifyq.cosB, qstate_notifyq.cos_sel, qstate_notifyq.eval_last,
                   qstate_notifyq.host, qstate_notifyq.total, qstate_notifyq.pid,
                   qstate_notifyq.p_index0, qstate_notifyq.c_index0, qstate_notifyq.host_pindex,
                   qstate_notifyq.cfg.enable, qstate_notifyq.cfg.host_queue,
                   qstate_notifyq.cfg.intr_enable, qstate_notifyq.cfg.debug,
                   qstate_notifyq.ring_base,
                   qstate_notifyq.ring_size, qstate_notifyq.host_ring_base,
                   qstate_notifyq.host_ring_size, qstate_notifyq.host_intr_assert_index);
        }
        if (qid == 1 || qid == 4) {
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_edmaq, sizeof(qstate_edmaq));
            printf("pc_offset=0x%0x\n"
                   "rsvd0=0x%0x\n"
                   "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
                   "eval_last=0x%0x\n"
                   "host=0x%0x\ntotal=0x%0x\n"
                   "pid=0x%0x\n"
                   "p_index0=0x%0x\nc_index0=0x%0x\ncomp_index=0x%0x\n"
                   "color=0x%0x\n"
                   "enable=0x%0x\nintr_enable=0x%0x\ndebug=0x%0x\n"
                   "ring_base=0x%0lx\nring_size=0x%0x\ncq_ring_base=0x%0lx\n"
                   "intr_assert_index=0x%0x\n",
                   qstate_edmaq.pc_offset, qstate_edmaq.rsvd0, qstate_edmaq.cosA,
                   qstate_edmaq.cosB, qstate_edmaq.cos_sel, qstate_edmaq.eval_last,
                   qstate_edmaq.host, qstate_edmaq.total, qstate_edmaq.pid, qstate_edmaq.p_index0,
                   qstate_edmaq.c_index0, qstate_edmaq.comp_index, qstate_edmaq.sta.color,
                   qstate_edmaq.cfg.enable, qstate_edmaq.cfg.intr_enable, qstate_edmaq.cfg.debug,
                   qstate_edmaq.ring_base, qstate_edmaq.ring_size, qstate_edmaq.cq_ring_base,
                   qstate_edmaq.intr_assert_index);
        }
        if (qid == 2) {
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_req, sizeof(qstate_req));
            printf("pc_offset=0x%0x\n"
                "rsvd0=0x%0x\n"
                "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
                "eval_last=0x%0x\n"
                "host=0x%0x\ntotal=0x%0x\n"
                "pid=0x%0x\n"
                "p_index0=0x%0x\nc_index0=0x%0x\n"
                "comp_index=0x%0x\nci_fetch=0x%0x\n"
                "color=0x%0x\n"
                "enable=0x%0x\nintr_enable=0x%0x\ndebug=0x%0x\n"
                "ring_base=0x%0lx\nring_size=0x%0x\n"
                "cq_ring_base=0x%0lx\nintr_assert_index=0x%0x\n",
                qstate_req.pc_offset, qstate_req.rsvd0, qstate_req.cosA, qstate_req.cosB,
                qstate_req.cos_sel, qstate_req.eval_last, qstate_req.host, qstate_req.total,
                qstate_req.pid, qstate_req.p_index0, qstate_req.c_index0, qstate_req.comp_index,
                qstate_req.ci_fetch, qstate_req.sta.color, qstate_req.cfg.enable,
                qstate_req.cfg.intr_enable, qstate_req.cfg.debug,
                qstate_req.ring_base, qstate_req.ring_size,
                qstate_req.cq_ring_base, qstate_req.intr_assert_index);
        }
        if (qid == 3) {
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_resp, sizeof(qstate_resp));
            printf("pc_offset=0x%0x\n"
                "rsvd0=0x%0x\n"
                "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
                "eval_last=0x%0x\n"
                "host=0x%0x\ntotal=0x%0x\n"
                "pid=0x%0x\n"
                "p_index0=0x%0x\nc_index0=0x%0x\n"
                "comp_index=0x%0x\nci_fetch=0x%0x\n"
                "color=0x%0x\n"
                "enable=0x%0x\nintr_enable=0x%0x\ndebug=0x%0x\n"
                "ring_base=0x%0lx\nring_size=0x%0x\n"
                "cq_ring_base=0x%0lx\nintr_assert_index=0x%0x\n",
                qstate_resp.pc_offset, qstate_resp.rsvd0, qstate_resp.cosA, qstate_resp.cosB,
                qstate_resp.cos_sel, qstate_resp.eval_last, qstate_resp.host, qstate_resp.total,
                qstate_resp.pid, qstate_resp.p_index0, qstate_resp.c_index0, qstate_resp.comp_index,
                qstate_resp.ci_fetch, qstate_resp.sta.color, qstate_resp.cfg.enable,
                qstate_resp.cfg.intr_enable, qstate_resp.cfg.debug,
                qstate_resp.ring_base, qstate_resp.ring_size,
                qstate_resp.cq_ring_base, qstate_resp.intr_assert_index);
        }
        break;
    default:
        printf("Invalid qtype %u for lif %u\n", qtype, lif);
    }
}

void
eth_qpoll(uint16_t lif, uint8_t qtype)
{
    union {
        eth_rx_qstate_t ethrx;
        eth_tx_co_qstate_t ethtx;
    } qstate = {0};
    struct admin_qstate qstate_ethaq = {0};
    notify_qstate_t qstate_notifyq = {0};
    edma_qstate_t qstate_edmaq = {0};
    nicmgr_req_qstate_t qstate_req = {0};
    nicmgr_resp_qstate_t qstate_resp = {0};
    queue_info_t qinfo[QTYPE_MAX] = {0};
    uint64_t addr = 0;

    if (!get_lif_qstate(lif, qinfo)) {
        printf("Failed to get qinfo for lif %u\n", lif);
        return;
    }

    if (qinfo[qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", qtype, lif);
        return;
    }

    for (uint32_t qid = 0; qid < qinfo[qtype].length; qid++) {
        addr = qinfo[qtype].base + qid * qinfo[qtype].size;
        switch (qtype) {
        case 0:
            sdk::lib::pal_mem_read(addr,
                                   (uint8_t *)&qstate.ethrx,
                                   sizeof(qstate.ethrx));
            printf(" rx%3d: "
                   "head %6u "
                   "tail %6u "
                   "comp_index %6u "
                   "color %d\n",
                   qid,
                   qstate.ethrx.q.ring[0].p_index,
                   qstate.ethrx.comp_index, // not c_index0
                   qstate.ethrx.comp_index,
                   qstate.ethrx.sta.color);
            break;
        case 1:
            sdk::lib::pal_mem_read(addr,
                                   (uint8_t *)&qstate.ethtx,
                                   sizeof(qstate.ethtx));
            printf(" tx%3d: "
                   "head %6u "
                   "tail %6u "
                   "comp_index %6u "
                   "color %d\n",
                   qid,
                   qstate.ethtx.tx.q.ring[0].p_index,
                   qstate.ethtx.tx.q.ring[0].c_index,
                   qstate.ethtx.tx.comp_index,
                   qstate.ethtx.tx.sta.color);
            break;
        case 2:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethaq, sizeof(qstate_ethaq));
            printf(" aq%3d: head %6u tail %6u comp_index %6u color %d\n", qid,
                   qstate_ethaq.p_index0, qstate_ethaq.c_index0, qstate_ethaq.comp_index,
                   qstate_ethaq.sta.color);
            break;
        case 7:
            if (qid == 0) {
                sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_notifyq, sizeof(qstate_notifyq));
                printf(" nq%3d: head %6u tail %6u host_pindex %6u\n", qid,
                        qstate_notifyq.p_index0, qstate_notifyq.c_index0,
                        qstate_notifyq.host_pindex);
            }
            if (qid == 1 || qid == 4) {
                sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_edmaq, sizeof(qstate_edmaq));
                printf(" dq%3d: head %6u tail %6u comp_index %6u color %d\n", qid,
                        qstate_edmaq.p_index0, qstate_edmaq.c_index0,
                        qstate_edmaq.comp_index, qstate_edmaq.sta.color);
            }
            if (qid == 2) {
                sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_req, sizeof(qstate_req));
                printf("req%3d: head %6u tail %6u comp_index %6u color %d\n", qid,
                    qstate_req.p_index0, qstate_req.c_index0, qstate_req.comp_index,
                    qstate_req.sta.color);
            }
            if (qid == 3) {
                sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_resp, sizeof(qstate_resp));
                printf("rsp%3d: head %6u tail %6u comp_index %6u color %d\n", qid,
                    qstate_resp.p_index0, qstate_resp.c_index0, qstate_resp.comp_index,
                    qstate_resp.sta.color);
            }
            break;
        default:
            printf("Invalid qtype %u for lif %u\n", qtype, lif);
        }
    }
}

void
eth_qdump(uint16_t lif, uint8_t qtype, uint32_t qid, uint8_t ring)
{
    queue_info_t qinfo[QTYPE_MAX] = {0};

    if (!get_lif_qstate(lif, qinfo)) {
        printf("Failed to get qinfo for lif %u\n", lif);
        return;
    }

    if (qinfo[qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", qtype, lif);
        return;
    }

    if (qid >= qinfo[qtype].length) {
        printf("Invalid qid %u for lif %u qtype %u\n", qid, lif, qtype);
        return;
    }

    uint64_t addr = qinfo[qtype].base + qid * qinfo[qtype].size;

    switch (qtype) {
    case 7:
        if (qid == 0) {
            notify_qstate_t qstate = {0};
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate, sizeof(qstate));
            if (ring == 0) {
                uint64_t desc_addr = qstate.ring_base;
                struct notify_desc desc = {0};
                for (uint16_t i = 0; i < (1 << qstate.ring_size); i++) {
                    sdk::lib::pal_mem_read(desc_addr, (uint8_t *)&desc, sizeof(desc));
                    printf("[%5d] eid = %lu ecode = %d data = ", i, desc.eid, desc.ecode);
                    for (uint16_t j = 0; j < sizeof(desc.data); j++) {
                        printf(" %02x", desc.data[j]);
                    }
                    printf("\n");
                    desc_addr += sizeof(desc);
                }
            }
        }
        if (qid == 1 || qid == 4) {
            edma_qstate_t qstate = {0};
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate, sizeof(qstate));
            if (ring == 0) {
                uint64_t desc_addr = qstate.ring_base;
                struct edma_cmd_desc desc = {0};
                for (uint16_t i = 0; i < (1 << qstate.ring_size); i++) {
                    sdk::lib::pal_mem_read(desc_addr, (uint8_t *)&desc, sizeof(desc));
                    printf("[%5d] opcode = %3d len = %5d slif = %4d saddr 0x%013lx dlif = %4d daddr = 0x%013lx\n",
                        i, desc.opcode, desc.len,
                        desc.src_lif, desc.src_addr, desc.dst_lif, desc.dst_addr);
                    desc_addr += sizeof(desc);
                }
            }
            if (ring == 1) {
                uint64_t desc_addr = qstate.cq_ring_base;
                struct edma_comp_desc desc = {0};
                for (uint16_t i = 0; i < (1 << qstate.ring_size); i++) {
                    sdk::lib::pal_mem_read(desc_addr, (uint8_t *)&desc, sizeof(desc));
                    printf("[%5d] status = %3d comp_index = %5d color = %d\n",
                        i, desc.status, desc.comp_index, desc.color);
                    desc_addr += sizeof(desc);
                }
            }
        }
        if (qid == 2) {
            struct admin_nicmgr_qstate qstate = {0};
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate, sizeof(qstate));
            if (ring == 0) {
                uint64_t desc_addr = qstate.ring_base;
                struct nicmgr_req_desc desc = {0};
                for (uint16_t i = 0; i < (1 << qstate.ring_size); i++) {
                    sdk::lib::pal_mem_read(desc_addr, (uint8_t *)&desc, sizeof(desc));
                    uint8_t *data = (uint8_t *)&desc.cmd;
                    printf("[%5d] lif = %d qtype = %d qid = %d comp = %d data = ",
                        i, desc.lif, desc.qtype, desc.qid, desc.comp_index);
                    for (uint16_t j = 0; j < sizeof(desc.cmd); j++) {
                        printf(" %02x", data[j]);
                    }
                    printf("\n");
                    desc_addr += sizeof(desc);
                }
            }
            if (ring == 1) {
                uint64_t desc_addr = qstate.cq_ring_base;
                struct nicmgr_req_comp_desc desc = {0};
                for (uint16_t i = 0; i < (1 << qstate.ring_size); i++) {
                    sdk::lib::pal_mem_read(desc_addr, (uint8_t *)&desc, sizeof(desc));
                    printf("[%5d] color %d\n", i, desc.color);
                    desc_addr += sizeof(desc);
                }
            }
        }
        if (qid == 3) {
            struct admin_nicmgr_qstate qstate = {0};
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate, sizeof(qstate));
            if (ring == 0) {
                uint64_t desc_addr = qstate.ring_base;
                struct nicmgr_resp_desc desc = {0};
                for (uint16_t i = 0; i < (1 << qstate.ring_size); i++) {
                    sdk::lib::pal_mem_read(desc_addr, (uint8_t *)&desc, sizeof(desc));
                    uint8_t *data = (uint8_t *)&desc.comp;
                    printf("[%5d] lif = %d qtype = %d qid = %d comp = %d data = ",
                        i, desc.lif, desc.qtype, desc.qid, desc.comp_index);
                    for (uint16_t j = 0; j < sizeof(desc.comp); j++) {
                        printf(" %02x", data[j]);
                    }
                    printf("\n");
                    desc_addr += sizeof(desc);
                }
            }
            if (ring == 1) {
                uint64_t desc_addr = qstate.cq_ring_base;
                struct nicmgr_resp_comp_desc desc = {0};
                for (uint16_t i = 0; i < (1 << qstate.ring_size); i++) {
                    sdk::lib::pal_mem_read(desc_addr, (uint8_t *)&desc, sizeof(desc));
                    printf("[%5d] color %d\n", i, desc.color);
                    desc_addr += sizeof(desc);
                }
            }
        }
        break;
    }
}

void
eth_debug(uint16_t lif, uint8_t qtype, uint32_t qid, uint8_t enable)
{
    union {
        eth_qstate_cfg_t eth = {0};
    } cfg;
    struct admin_cfg_qstate qstate_aq = {0};
    struct edma_cfg_qstate qstate_edmaq = {0};
    struct notify_cfg_qstate qstate_notifyq = {0};
    struct admin_nicmgr_cfg_qstate qstate_req = {0};
    struct admin_nicmgr_cfg_qstate qstate_resp = {0};
    queue_info_t qinfo[QTYPE_MAX] = {0};

    if (!get_lif_qstate(lif, qinfo)) {
        printf("Failed to get qinfo for lif %u\n", lif);
        return;
    }

    if (qinfo[qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", qtype, lif);
        return;
    }

    if (qid >= qinfo[qtype].length) {
        printf("Invalid qid %u for lif %u qtype %u\n", qid, lif, qtype);
        return;
    }

    uint64_t addr = qinfo[qtype].base + qid * qinfo[qtype].size;

    switch (qtype) {
        case 0:
            sdk::lib::pal_mem_read(addr + offsetof(eth_qstate_common_t, cfg),
                (uint8_t *)&cfg.eth, sizeof(cfg.eth));
            cfg.eth.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(eth_qstate_common_t, cfg),
                (uint8_t *)&cfg.eth, sizeof(cfg.eth));
            PAL_barrier();
            sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr,
                sizeof(eth_rx_qstate_t), P4PLUS_CACHE_INVALIDATE_BOTH);
            break;
        case 1:
            sdk::lib::pal_mem_read(addr + offsetof(eth_qstate_common_t, cfg),
                (uint8_t *)&cfg.eth, sizeof(cfg.eth));
            cfg.eth.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(eth_qstate_common_t, cfg),
                (uint8_t *)&cfg.eth, sizeof(cfg.eth));
            PAL_barrier();
            sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr,
                sizeof(eth_tx_qstate_t), P4PLUS_CACHE_INVALIDATE_BOTH);
            break;
        case 2:
            sdk::lib::pal_mem_read(addr + offsetof(admin_qstate_t, cfg),
                (uint8_t *)&qstate_aq, sizeof(qstate_aq));
            qstate_aq.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(admin_qstate_t, cfg),
                (uint8_t *)&qstate_aq, sizeof(qstate_aq));
            PAL_barrier();
            sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr,
                sizeof(admin_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
            break;
        case 7:
        if (qid == 0) {
            sdk::lib::pal_mem_read(addr + offsetof(notify_qstate_t, cfg),
                (uint8_t *)&qstate_notifyq, sizeof(qstate_notifyq));
            qstate_notifyq.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(notify_qstate_t, cfg),
                (uint8_t *)&qstate_notifyq, sizeof(qstate_notifyq));
            PAL_barrier();
            sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr,
                sizeof(notify_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
        }
        if (qid == 1) {
            sdk::lib::pal_mem_read(addr + offsetof(edma_qstate_t, cfg),
                (uint8_t *)&qstate_edmaq, sizeof(qstate_edmaq));
            qstate_edmaq.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(edma_qstate_t, cfg),
                (uint8_t *)&qstate_edmaq, sizeof(qstate_edmaq));
            PAL_barrier();
            sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr,
                sizeof(edma_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
        }
        if (qid == 2) {
            sdk::lib::pal_mem_read(addr + offsetof(nicmgr_req_qstate_t, cfg),
                (uint8_t *)&qstate_req, sizeof(qstate_req));
            qstate_req.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(nicmgr_req_qstate_t, cfg),
                (uint8_t *)&qstate_req, sizeof(qstate_req));
            PAL_barrier();
            sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr,
                sizeof(nicmgr_req_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
        }
        if (qid == 3) {
            sdk::lib::pal_mem_read(addr + offsetof(nicmgr_resp_qstate_t, cfg),
                (uint8_t *)&qstate_resp, sizeof(qstate_resp));
            qstate_resp.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(nicmgr_resp_qstate_t, cfg),
                (uint8_t *)&qstate_resp, sizeof(qstate_resp));
            PAL_barrier();
            sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr,
                sizeof(nicmgr_resp_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
        }
    }

    eth_qstate(lif, qtype, qid);
}

void
eth_eqstate(uint64_t addr)
{
    struct eth_eq_qstate eqstate = {0};

    sdk::lib::pal_mem_read(addr, (uint8_t *)&eqstate, sizeof(eqstate));

    printf("eq_ring_base=%#lx\n"        "eq_ring_size=%#x\n"
           "eq_enable=%#x\n"            "intr_enable=%#x\n"
           "eq_index=%#x\n"             "eq_gen=%#x\n"
           "intr_index=%#x\n",
           eqstate.eq_ring_base,        eqstate.eq_ring_size,
           eqstate.cfg.eq_enable,       eqstate.cfg.intr_enable,
           eqstate.eq_index,            eqstate.eq_gen,
           eqstate.intr_index);
}

void
eth_stats(uint16_t lif)
{
    struct ionic_lif_stats stats;

    std::string mpart_json = mpart_cfg_path();
    mpartition *mp_ = mpartition::factory(mpart_json.c_str());
    assert(mp_);

    uint64_t addr = mp_->start_addr(MEM_REGION_LIF_STATS_NAME) + (lif << 10);

    printf("\naddr: 0x%lx\n\n", addr);
    sdk::lib::pal_mem_read(addr, (uint8_t *)&stats, sizeof(struct ionic_lif_stats));

    printf("rx_ucast_bytes              : %lu\n", stats.rx_ucast_bytes);
    printf("rx_ucast_packets            : %lu\n", stats.rx_ucast_packets);
    printf("rx_mcast_bytes              : %lu\n", stats.rx_mcast_bytes);
    printf("rx_mcast_packets            : %lu\n", stats.rx_mcast_packets);
    printf("rx_bcast_bytes              : %lu\n", stats.rx_bcast_bytes);
    printf("rx_bcast_packets            : %lu\n", stats.rx_bcast_packets);

    printf("rx_ucast_drop_bytes         : %lu\n", stats.rx_ucast_drop_bytes);
    printf("rx_ucast_drop_packets       : %lu\n", stats.rx_ucast_drop_packets);
    printf("rx_mcast_drop_bytes         : %lu\n", stats.rx_mcast_drop_bytes);
    printf("rx_mcast_drop_packets       : %lu\n", stats.rx_mcast_drop_packets);
    printf("rx_bcast_drop_bytes         : %lu\n", stats.rx_bcast_drop_bytes);
    printf("rx_bcast_drop_packets       : %lu\n", stats.rx_bcast_drop_packets);
    printf("rx_dma_error                : %lu\n", stats.rx_dma_error);

    printf("tx_ucast_bytes              : %lu\n", stats.tx_ucast_bytes);
    printf("tx_ucast_packets            : %lu\n", stats.tx_ucast_packets);
    printf("tx_mcast_bytes              : %lu\n", stats.tx_mcast_bytes);
    printf("tx_mcast_packets            : %lu\n", stats.tx_mcast_packets);
    printf("tx_bcast_bytes              : %lu\n", stats.tx_bcast_bytes);
    printf("tx_bcast_packets            : %lu\n", stats.tx_bcast_packets);

    printf("tx_ucast_drop_bytes         : %lu\n", stats.tx_ucast_drop_bytes);
    printf("tx_ucast_drop_packets       : %lu\n", stats.tx_ucast_drop_packets);
    printf("tx_mcast_drop_bytes         : %lu\n", stats.tx_mcast_drop_bytes);
    printf("tx_mcast_drop_packets       : %lu\n", stats.tx_mcast_drop_packets);
    printf("tx_bcast_drop_bytes         : %lu\n", stats.tx_bcast_drop_bytes);
    printf("tx_bcast_drop_packets       : %lu\n", stats.tx_bcast_drop_packets);
    printf("tx_dma_error                : %lu\n", stats.tx_dma_error);

    printf("rx_queue_disabled           : %lu\n", stats.rx_queue_disabled);
    printf("rx_queue_empty              : %lu\n", stats.rx_queue_empty);
    printf("rx_queue_error              : %lu\n", stats.rx_queue_error);
    printf("rx_desc_fetch_error         : %lu\n", stats.rx_desc_fetch_error);
    printf("rx_desc_data_error          : %lu\n", stats.rx_desc_data_error);

    printf("tx_queue_disabled           : %lu\n", stats.tx_queue_disabled);
    printf("tx_queue_error              : %lu\n", stats.tx_queue_error);
    printf("tx_desc_fetch_error         : %lu\n", stats.tx_desc_fetch_error);
    printf("tx_desc_data_error          : %lu\n", stats.tx_desc_data_error);
    printf("tx_queue_empty              : %lu\n", stats.tx_queue_empty);

    printf("tx_ucast_drop_bytes         : %lu\n", stats.tx_ucast_drop_bytes);
    printf("tx_ucast_drop_packets       : %lu\n", stats.tx_ucast_drop_packets);
    printf("tx_mcast_drop_bytes         : %lu\n", stats.tx_mcast_drop_bytes);
    printf("tx_mcast_drop_packets       : %lu\n", stats.tx_mcast_drop_packets);
    printf("tx_bcast_drop_bytes         : %lu\n", stats.tx_bcast_drop_bytes);
    printf("tx_bcast_drop_packets       : %lu\n", stats.tx_bcast_drop_packets);
    printf("tx_dma_error                : %lu\n", stats.tx_dma_error);

    printf("tx_rdma_ucast_bytes         : %lu\n", stats.tx_rdma_ucast_bytes);
    printf("tx_rdma_ucast_packets       : %lu\n", stats.tx_rdma_ucast_packets);
    printf("tx_rdma_mcast_bytes         : %lu\n", stats.tx_rdma_mcast_bytes);
    printf("tx_rdma_mcast_packets       : %lu\n", stats.tx_rdma_mcast_packets);
    printf("tx_rdma_cnp_packets         : %lu\n", stats.tx_rdma_cnp_packets);

    printf("rx_rdma_ucast_bytes         : %lu\n", stats.rx_rdma_ucast_bytes);
    printf("rx_rdma_ucast_packets       : %lu\n", stats.rx_rdma_ucast_packets);
    printf("rx_rdma_mcast_bytes         : %lu\n", stats.rx_rdma_mcast_bytes);
    printf("rx_rdma_mcast_packets       : %lu\n", stats.rx_rdma_mcast_packets);
    printf("rx_rdma_cnp_packets         : %lu\n", stats.rx_rdma_cnp_packets);
    printf("rx_rdma_ecn_packets         : %lu\n", stats.rx_rdma_ecn_packets);

    printf("rdma_packet_seq_err         : %lu\n", stats.rdma_req_rx_pkt_seq_err);
    printf("rdma_req_rnr_retry_err      : %lu\n", stats.rdma_req_rx_rnr_retry_err);
    printf("rdma_req_remote_access_err  : %lu\n", stats.rdma_req_rx_remote_access_err);
    printf("rdma_req_remote_inv_req_err : %lu\n", stats.rdma_req_rx_remote_inv_req_err);
    printf("rdma_req_remote_oper_err    : %lu\n", stats.rdma_req_rx_remote_oper_err);
    printf("rdma_implied_nak_seq_err    : %lu\n", stats.rdma_req_rx_implied_nak_seq_err);
    printf("rdma_req_cqe_err            : %lu\n", stats.rdma_req_rx_cqe_err);
    printf("rdma_req_cqe_flush_err      : %lu\n", stats.rdma_req_rx_cqe_flush_err);

    printf("rdma_duplicate_responses    : %lu\n", stats.rdma_req_rx_dup_responses);
    printf("rdma_req_invalid_packets    : %lu\n", stats.rdma_req_rx_invalid_packets);
    printf("rdma_req_local_access_err   : %lu\n", stats.rdma_req_tx_local_access_err);
    printf("rdma_req_local_oper_err     : %lu\n", stats.rdma_req_tx_local_oper_err);
    printf("rdma_req_memory_mgmt_err    : %lu\n", stats.rdma_req_tx_memory_mgmt_err);

    printf("rdma_duplicate_request      : %lu\n", stats.rdma_resp_rx_dup_requests);
    printf("rdma_out_of_buffer          : %lu\n", stats.rdma_resp_rx_out_of_buffer);
    printf("rdma_out_of_sequence        : %lu\n", stats.rdma_resp_rx_out_of_seq_pkts);
    printf("rdma_resp_cqe_err           : %lu\n", stats.rdma_resp_rx_cqe_err);
    printf("rdma_resp_cqe_flush_err     : %lu\n", stats.rdma_resp_rx_cqe_flush_err);
    printf("rdma_resp_local_len_err     : %lu\n", stats.rdma_resp_rx_local_len_err);
    printf("rdma_resp_inv_request_err   : %lu\n", stats.rdma_resp_rx_inv_request_err);
    printf("rdma_resp_local_qp_oper_err : %lu\n", stats.rdma_resp_rx_local_qp_oper_err);

    printf("rdma_out_of_atomic_resource : %lu\n", stats.rdma_resp_rx_out_of_atomic_resource);

    printf("rdma_resp_pkt_seq_err       : %lu\n", stats.rdma_resp_tx_pkt_seq_err);
    printf("rdma_resp_remote_inv_req_err: %lu\n", stats.rdma_resp_tx_remote_inv_req_err);
    printf("rdma_resp_remote_access_err : %lu\n", stats.rdma_resp_tx_remote_access_err);
    printf("rdma_resp_remote_oper_err   : %lu\n", stats.rdma_resp_tx_remote_oper_err);
    printf("rdma_resp_rnr_retry_err     : %lu\n", stats.rdma_resp_tx_rnr_retry_err);

}

void
eth_stats_reset(uint16_t lif)
{
    std::string mpart_json = mpart_cfg_path();
    mpartition *mp_ = mpartition::factory(mpart_json.c_str());
    assert(mp_);

    uint64_t addr = mp_->start_addr(MEM_REGION_LIF_STATS_NAME) + (lif << 10);

    printf("\naddr: 0x%lx\n\n", addr);
    sdk::lib::pal_mem_set(addr, 0, sizeof(struct ionic_lif_stats), 0);
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr, sizeof(struct ionic_lif_stats),
                                                  P4PLUS_CACHE_INVALIDATE_BOTH);
    sdk::asic::pd::asicpd_p4_invalidate_cache(addr, sizeof(struct ionic_lif_stats),
                                              P4_TBL_CACHE_INGRESS);
    sdk::asic::pd::asicpd_p4_invalidate_cache(addr, sizeof(struct ionic_lif_stats),
                                              P4_TBL_CACHE_EGRESS);
}

void
eth_port_config(uint64_t addr)
{
    uint8_t *buf = (uint8_t *)calloc(1, sizeof(union ionic_port_config));
    assert(buf != NULL);
    sdk::lib::pal_mem_read(addr, buf, sizeof(union ionic_port_config));
    union ionic_port_config *config = (union ionic_port_config *)buf;

    printf("\n");
    printf("port_config:\n");
    printf("  speed: %u\n", config->speed);
    printf("  mtu: %u\n", config->mtu);
    printf("  state: %u\n", config->state);
    printf("  an_enable: %u\n", config->an_enable);
    printf("  fec_type: %u\n", config->fec_type);
    printf("  pause_type: %u\n", config->pause_type);
    printf("  loopback_mode: %u\n", config->loopback_mode);
    printf("\n");

    free(buf);
}

void
eth_port_status(uint64_t addr)
{
    uint8_t *buf = (uint8_t *)calloc(1, sizeof(struct ionic_port_status));
    assert(buf != NULL);
    sdk::lib::pal_mem_read(addr, buf, sizeof(struct ionic_port_status));
    struct ionic_port_status *status = (struct ionic_port_status *)buf;

    printf("\n");
    printf("port_status:\n");
    printf("  id: %u\n", status->id);
    printf("  speed: %u\n", status->speed);
    printf("  status: %u\n", status->status);
    printf("  link_down_count: %u\n", status->link_down_count);
    printf("  xcvr:\n");
    printf("    state: %u\n", status->xcvr.state);
    printf("    phy: %u\n", status->xcvr.phy);
    printf("    pid: %u\n", status->xcvr.pid);
    printf("    sprom:\n");
    for (uint32_t i = 0; i < sizeof(status->xcvr.sprom) / 16; i++) {
        printf("      ");
        for (uint32_t j = 0; j < 16; j++) {
            printf("%02x ", status->xcvr.sprom[(i * 16) + j]);
        }
        printf("\n");
    }

    printf("\n");

    free(buf);
}

void
eth_lif_status(uint64_t addr)
{
    uint8_t *buf = (uint8_t *)calloc(1, sizeof(struct ionic_lif_status));
    assert(buf != NULL);
    sdk::lib::pal_mem_read(addr, buf, sizeof(struct ionic_lif_status));
    struct ionic_lif_status *status = (struct ionic_lif_status *)buf;

    printf("\n");
    printf("lif_status:\n");
    printf("  eid: %lu\n", status->eid);
    printf("  port_num: %u\n", status->port_num);
    printf("  link_status: %u\n", status->link_status);
    printf("  link_speed: %u\n", status->link_speed);
    printf("  link_down_count: %u\n", status->link_down_count);
    printf("\n");

    free(buf);
}
