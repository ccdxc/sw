/*
* Copyright (c) 2020, Pensando Systems Inc.
*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "nic/include/adminq.h"
#include "nic/include/edmaq.h"
#include "nic/include/notify.hpp"

#include "nic/sdk/lib/pal/pal.hpp"

#include "impl.hpp"


void
nvme_qstate(uint16_t lif, uint8_t qtype, uint32_t qid)
{
    struct admin_qstate qstate_ethaq = {0};
    struct notify_qstate qstate_notifyq = {0};
    struct edma_qstate qstate_edmaq = {0};
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
    case 1:
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
    case 2:
        if (qid == 0) {
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
        if (qid == 1) {
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
 
        if (qid == 2) {
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
        if (qid == 3) {
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
                   qstate_edmaq.cfg.enable, qstate_edmaq.cfg.intr_enable,
                   qstate_edmaq.cfg.debug,
                   qstate_edmaq.ring_base,
                   qstate_edmaq.ring_size, qstate_edmaq.cq_ring_base,
                   qstate_edmaq.intr_assert_index);
        }
       break;
    default:
        printf("Invalid qtype %u for lif %u\n", qtype, lif);
    }
}
