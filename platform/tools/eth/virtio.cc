/*
* Copyright (c) 2020, Pensando Systems Inc.
*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "nic/sdk/lib/pal/pal.hpp"
#include "platform/src/lib/nicmgr/include/virtio_if.h"

#include "impl.hpp"


void
virtio_qstate(uint16_t lif, uint8_t qtype, uint32_t qid)
{
    queue_info_t qinfo[QTYPE_MAX] = { 0 };
    union {
        virtio_qstate_rx_t rx;
        virtio_qstate_tx_t tx;
        uint8_t raw[0];
    } qstate = { 0 };

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

    sdk::lib::pal_mem_read(addr, qstate.raw, qinfo[qtype].size);

    switch (qtype) {
    case 0: /* Virtio RX */
        printf("pc_offset %#x\n"
               "rsvd0 %#x\n"
               "cosA %#x\n"
               "cosB %#x\n"
               "cos_sel %#x\n"
               "eval_last %#x\n"
               "host %#x\n"
               "total %#x\n"
               "pid %#x\n"
               "ring0_pi %#x\n"
               "ring0_ci %#x\n"
               "features %#lx\n"
               "rx_virtq_desc_addr %#lx\n"
               "rx_virtq_avail_addr %#lx\n"
               "rx_virtq_used_addr %#lx\n"
               "rx_intr_assert_addr %#x\n"
               "rx_queue_size_mask %#x\n"
               "rx_virtq_avail_ci %#x\n"
               "rx_virtq_used_pi %#x\n",
               qstate.rx.qs.pc_offset,
               qstate.rx.qs.rsvd0,
               qstate.rx.qs.cosA,
               qstate.rx.qs.cosB,
               qstate.rx.qs.cos_sel,
               qstate.rx.qs.eval_last,
               qstate.rx.qs.host,
               qstate.rx.qs.total,
               qstate.rx.qs.pid,
               qstate.rx.ring[0].pi,
               qstate.rx.ring[0].ci,
               qstate.rx.features,
               qstate.rx.rx_virtq_desc_addr,
               qstate.rx.rx_virtq_avail_addr,
               qstate.rx.rx_virtq_used_addr,
               qstate.rx.rx_intr_assert_addr,
               qstate.rx.rx_queue_size_mask,
               qstate.rx.rx_virtq_avail_ci,
               qstate.rx.rx_virtq_used_pi);
       break;
    case 1: /* Virtio TX */
        printf("pc_offset %#x\n"
               "rsvd0 %#x\n"
               "cosA %#x\n"
               "cosB %#x\n"
               "cos_sel %#x\n"
               "eval_last %#x\n"
               "host %#x\n"
               "total %#x\n"
               "pid %#x\n"
               "ring0_pi %#x\n"
               "ring0_ci %#x\n"
               "ring1_pi %#x\n"
               "ring1_ci %#x\n"
               "features %#lx\n"
               "tx_virtq_desc_addr %#lx\n"
               "tx_virtq_avail_addr %#lx\n"
               "tx_virtq_used_addr %#lx\n"
               "tx_intr_assert_addr %#x\n"
               "tx_queue_size_mask %#x\n"
               "tx_virtq_avail_ci %#x\n"
               "tx_virtq_used_pi %#x\n",
               qstate.tx.qs.pc_offset,
               qstate.tx.qs.rsvd0,
               qstate.tx.qs.cosA,
               qstate.tx.qs.cosB,
               qstate.tx.qs.cos_sel,
               qstate.tx.qs.eval_last,
               qstate.tx.qs.host,
               qstate.tx.qs.total,
               qstate.tx.qs.pid,
               qstate.tx.ring[0].pi,
               qstate.tx.ring[0].ci,
               qstate.tx.ring[1].pi,
               qstate.tx.ring[1].ci,
               qstate.tx.features,
               qstate.tx.tx_virtq_desc_addr,
               qstate.tx.tx_virtq_avail_addr,
               qstate.tx.tx_virtq_used_addr,
               qstate.tx.tx_intr_assert_addr,
               qstate.tx.tx_queue_size_mask,
               qstate.tx.tx_virtq_avail_ci,
               qstate.tx.tx_virtq_used_pi);
       break;
    default:
        printf("Invalid qtype %u for lif %u\n", qtype, lif);
    }
}
