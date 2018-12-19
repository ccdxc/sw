
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "nic/include/adminq.h"
#include "nic/include/capri_common.h"
#include "nic/include/edma.hpp"
#include "nic/include/eth_common.h"
#include "nic/include/notify.hpp"
#include "nic/sdk/include/sdk/pal.hpp"
#include "nic/sdk/include/sdk/platform/utils/mpartition.hpp"

#include "nic/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_wa_c_hdr.h"

/* Supply these for ionic_if.h */
#define BIT(n)                  (1 << n)
#define TEST_BIT(x, n)          ((x) & (1 << n))
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define dma_addr_t uint64_t

#include "platform/drivers/common/ionic_if.h"


typedef struct {
    uint64_t base;
    uint32_t size;
    uint32_t length;
} queue_info_t;

bool
get_lif_qstate(uint16_t lif, queue_info_t qinfo[8])
{
    uint32_t cnt[4] = {0};
    uint32_t size[8] = {0};
    uint32_t length[8] = {0};

    sdk::lib::pal_reg_read(CAP_ADDR_BASE_DB_WA_OFFSET +
                               CAP_WA_CSR_DHS_LIF_QSTATE_MAP_BYTE_ADDRESS + (16 * lif),
                           cnt, 4);

    // decode lif qstate table:
    uint8_t valid = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_VLD_GET(cnt[0]);
    if (!valid) {
        printf("Invalid lif %u\n", lif);
        return false;
    }

    uint64_t base =
        (uint64_t)CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_QSTATE_BASE_GET(cnt[0]);
    // Qstate Size: 3 bit size is qstate size (32B/64B/128B)
    size[0] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_SIZE0_GET(cnt[0]);
    size[1] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE1_GET(cnt[1]);
    size[2] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE2_GET(cnt[1]);
    size[3] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE3_GET(cnt[1]);
    size[4] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE4_GET(cnt[1]);
    size[5] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SIZE5_GET(cnt[2]);
    size[6] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SIZE6_GET(cnt[2]);
    size[7] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SIZE7_GET(cnt[2]);
    // log2(number_of_queues)
    length[0] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_LENGTH0_GET(cnt[0]);
    length[1] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_LENGTH1_0_0_GET(cnt[0]) |
                (CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH1_4_1_GET(cnt[1]) << 1);
    length[2] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH2_GET(cnt[1]);
    length[3] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH3_GET(cnt[1]);
    length[4] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH4_GET(cnt[1]);
    length[5] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH5_0_0_GET(cnt[1]) |
                (CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_LENGTH5_4_1_GET(cnt[2]) << 1);
    length[6] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_LENGTH6_GET(cnt[2]);
    length[7] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_LENGTH7_GET(cnt[2]);

    base = base << 12;

    for (uint8_t qtype = 0; qtype < 8 && size[qtype] != 0; qtype++) {

        qinfo[qtype].base = base;
        qinfo[qtype].size = 32 * (1 << size[qtype]);
        qinfo[qtype].length = 1 << length[qtype];

        base += qinfo[qtype].size * qinfo[qtype].length;
    }

    return true;
}

#define NUM_POSTED(N, h, t) (h == t ? 0 : h > t ? h - t + 1 : ((N)-1) - (h - t + 1))

void
eth_qpoll(uint16_t lif, uint8_t qtype)
{
    struct eth_rx_qstate qstate_ethrx = {0};
    struct eth_tx_qstate qstate_ethtx = {0};
    struct admin_qstate qstate_ethaq = {0};
    struct notify_qstate qstate_notifyq = {0};
    struct edma_qstate qstate_edmaq = {0};

    queue_info_t qinfo[8] = {0};

    assert(get_lif_qstate(lif, qinfo));

    if (qinfo[qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", qtype, lif);
        return;
    }

    for (uint32_t qid = 0; qid < qinfo[qtype].length; qid++) {
        uint64_t addr = qinfo[qtype].base + qid * qinfo[qtype].size;
        uint32_t posted = 0;
        switch (qtype) {
        case 0:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethrx, sizeof(qstate_ethrx));
            posted = NUM_POSTED(1 << qstate_ethrx.ring_size, qstate_ethrx.p_index0,
                                qstate_ethrx.c_index0);
            printf("rx%3d: head %6u tail %6u posted %6d comp_index %6u color %d\n", qid,
                   qstate_ethrx.p_index0, qstate_ethrx.c_index0, posted, qstate_ethrx.comp_index,
                   qstate_ethrx.sta.color);
            break;
        case 1:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethtx, sizeof(qstate_ethtx));
            posted = NUM_POSTED(1 << qstate_ethtx.ring_size, qstate_ethtx.p_index0,
                                qstate_ethtx.c_index0);
            printf("tx%3d: head %6u tail %6u posted %6d comp_index %6u color %d\n", qid,
                   qstate_ethtx.p_index0, qstate_ethtx.c_index0, posted, qstate_ethtx.comp_index,
                   qstate_ethtx.sta.color);
            break;
        case 2:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethaq, sizeof(qstate_ethaq));
            posted = NUM_POSTED(1 << qstate_ethaq.ring_size, qstate_ethaq.p_index0,
                                qstate_ethaq.c_index0);
            printf("aq%3d: head %6u tail %6u posted %6d comp_index %6u color %d\n", qid,
                   qstate_ethaq.p_index0, qstate_ethaq.c_index0, posted, qstate_ethaq.comp_index,
                   qstate_ethaq.sta.color);
            break;
        case 7:
            if (qid == 0) {
                sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_notifyq, sizeof(qstate_notifyq));
                posted = NUM_POSTED(1 << qstate_notifyq.ring_size, qstate_notifyq.p_index0,
                                    qstate_notifyq.c_index0);
                printf("nq%3d: head %6u tail %6u posted %6d host_pindex %6u\n", qid,
                       qstate_notifyq.p_index0, qstate_notifyq.c_index0, posted,
                       qstate_notifyq.host_pindex);
            }
            if (qid == 1) {
                sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_edmaq, sizeof(qstate_edmaq));
                posted = NUM_POSTED(1 << qstate_edmaq.ring_size, qstate_edmaq.p_index0,
                                    qstate_edmaq.c_index0);
                printf("nq%3d: head %6u tail %6u posted %6d comp_index %6u color %d\n", qid,
                       qstate_edmaq.p_index0, qstate_edmaq.c_index0, posted,
                       qstate_edmaq.comp_index, qstate_edmaq.sta.color);
            }
            break;
        default:
            assert(0);
        }
    }
}

void
eth_qstate(uint16_t lif, uint8_t qtype, uint32_t qid)
{
    struct eth_rx_qstate qstate_ethrx = {0};
    struct eth_tx_qstate qstate_ethtx = {0};
    struct admin_qstate qstate_ethaq = {0};
    struct notify_qstate qstate_notifyq = {0};
    struct edma_qstate qstate_edmaq = {0};
    queue_info_t qinfo[8] = {0};

    assert(get_lif_qstate(lif, qinfo));

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
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethrx, sizeof(qstate_ethrx));
        printf("pc_offset=0x%0x\n"
               "rsvd0=0x%0x\n"
               "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
               "eval_last=0x%0x\n"
               "host=0x%0x\ntotal=0x%0x\n"
               "pid=0x%0x\n"
               "p_index0=0x%0x\nc_index0=0x%0x\ncomp_index=0x%0x\n"
               "color=0x%0x\n"
               "enable=0x%0x\nhost_queue=0x%0x\nintr_enable=0x%0x\n"
               "ring_base=0x%0lx\nring_size=0x%0x\n"
               "cq_ring_base=0x%0lx\nintr_assert_index=0x%0x\n",
               qstate_ethrx.pc_offset, qstate_ethrx.rsvd0, qstate_ethrx.cosA, qstate_ethrx.cosB,
               qstate_ethrx.cos_sel, qstate_ethrx.eval_last, qstate_ethrx.host, qstate_ethrx.total,
               qstate_ethrx.pid, qstate_ethrx.p_index0, qstate_ethrx.c_index0,
               qstate_ethrx.comp_index, qstate_ethrx.sta.color, qstate_ethrx.cfg.enable,
               qstate_ethrx.cfg.host_queue, qstate_ethrx.cfg.intr_enable, qstate_ethrx.ring_base,
               qstate_ethrx.ring_size, qstate_ethrx.cq_ring_base, qstate_ethrx.intr_assert_index);
        break;
    case 1:
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethtx, sizeof(qstate_ethtx));
        printf("pc_offset=0x%0x\n"
               "rsvd0=0x%0x\n"
               "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
               "eval_last=0x%0x\n"
               "host=0x%0x\ntotal=0x%0x\n"
               "pid=0x%0x\n"
               "p_index0=0x%0x\nc_index0=0x%0x\n"
               "comp_index=0x%0x\nci_fetch=0x%0x\nci_miss=0x%0x\n"
               "color=0x%0x\nspec_miss=0x%0x\n"
               "enable=0x%0x\nhost_queue=0x%0x\nintr_enable=0x%0x\n"
               "ring_base=0x%0lx\nring_size=0x%0x\n"
               "cq_ring_base=0x%0lx\nintr_assert_index=0x%0x\n"
               "sg_ring_base=0x%0lx\n"
               "spurious_db_cnt=0x%lx\n",
               qstate_ethtx.pc_offset, qstate_ethtx.rsvd0, qstate_ethtx.cosA, qstate_ethtx.cosB,
               qstate_ethtx.cos_sel, qstate_ethtx.eval_last, qstate_ethtx.host, qstate_ethtx.total,
               qstate_ethtx.pid, qstate_ethtx.p_index0, qstate_ethtx.c_index0,
               qstate_ethtx.comp_index, qstate_ethtx.ci_fetch, qstate_ethtx.ci_miss,
               qstate_ethtx.sta.color, qstate_ethtx.sta.spec_miss, qstate_ethtx.cfg.enable,
               qstate_ethtx.cfg.host_queue, qstate_ethtx.cfg.intr_enable, qstate_ethtx.ring_base,
               qstate_ethtx.ring_size, qstate_ethtx.cq_ring_base, qstate_ethtx.intr_assert_index,
               qstate_ethtx.sg_ring_base, qstate_ethtx.spurious_db_cnt);
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
               "enable=0x%0x\nhost_queue=0x%0x\nintr_enable=0x%0x\n"
               "ring_base=0x%0lx\nring_size=0x%0x\n"
               "cq_ring_base=0x%0lx\nintr_assert_index=0x%0x\n"
               "nicmgr_qstate_addr=0x%0lx\n",
               qstate_ethaq.pc_offset, qstate_ethaq.rsvd0, qstate_ethaq.cosA, qstate_ethaq.cosB,
               qstate_ethaq.cos_sel, qstate_ethaq.eval_last, qstate_ethaq.host, qstate_ethaq.total,
               qstate_ethaq.pid, qstate_ethaq.p_index0, qstate_ethaq.c_index0,
               qstate_ethaq.comp_index, qstate_ethaq.ci_fetch, qstate_ethaq.sta.color,
               qstate_ethaq.cfg.enable, qstate_ethaq.cfg.host_queue, qstate_ethaq.cfg.intr_enable,
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
                   "enable=0x%0x\nhost_queue=0x%0x\nintr_enable=0x%0x\n"
                   "ring_base=0x%0lx\nring_size=0x%0x\n"
                   "host_ring_base=0x%0lx\nhost_ring_size=0x%0x\nhost_intr_assert_index=0x%0x\n",
                   qstate_notifyq.pc_offset, qstate_notifyq.rsvd0, qstate_notifyq.cosA,
                   qstate_notifyq.cosB, qstate_notifyq.cos_sel, qstate_notifyq.eval_last,
                   qstate_notifyq.host, qstate_notifyq.total, qstate_notifyq.pid,
                   qstate_notifyq.p_index0, qstate_notifyq.c_index0, qstate_notifyq.host_pindex,
                   qstate_notifyq.cfg.enable, qstate_notifyq.cfg.host_queue,
                   qstate_notifyq.cfg.intr_enable, qstate_notifyq.ring_base,
                   qstate_notifyq.ring_size, qstate_notifyq.host_ring_base,
                   qstate_notifyq.host_ring_size, qstate_notifyq.host_intr_assert_index);
        }
        if (qid == 1) {
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_edmaq, sizeof(qstate_edmaq));
            printf("pc_offset=0x%0x\n"
                   "rsvd0=0x%0x\n"
                   "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
                   "eval_last=0x%0x\n"
                   "host=0x%0x\ntotal=0x%0x\n"
                   "pid=0x%0x\n"
                   "p_index0=0x%0x\nc_index0=0x%0x\ncomp_index=0x%0x\n"
                   "color=0x%0x\n"
                   "enable=0x%0x\nintr_enable=0x%0x\n"
                   "ring_base=0x%0lx\nring_size=0x%0x\ncq_ring_base=0x%0lx\n"
                   "intr_assert_index=0x%0x\n",
                   qstate_edmaq.pc_offset, qstate_edmaq.rsvd0, qstate_edmaq.cosA,
                   qstate_edmaq.cosB, qstate_edmaq.cos_sel, qstate_edmaq.eval_last,
                   qstate_edmaq.host, qstate_edmaq.total, qstate_edmaq.pid, qstate_edmaq.p_index0,
                   qstate_edmaq.c_index0, qstate_edmaq.comp_index, qstate_edmaq.sta.color,
                   qstate_edmaq.cfg.enable, qstate_edmaq.cfg.intr_enable, qstate_edmaq.ring_base,
                   qstate_edmaq.ring_size, qstate_edmaq.cq_ring_base,
                   qstate_edmaq.intr_assert_index);
        }
        break;
    default:
        assert(0);
    }
}

void
eth_stats(uint16_t lif)
{
    sdk::platform::utils::mpartition *mp_ =
        sdk::platform::utils::mpartition::factory("/nic/conf/iris/hbm_mem.json", CAPRI_HBM_BASE);
    assert(mp_);

    uint64_t addr = mp_->start_addr("lif_stats") + (lif << 10);
    printf("\naddr: 0x%lx\n\n", addr);

    struct ionic_lif_stats stats;
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
nicmgr_qstate(uint16_t lif, uint8_t qtype, uint32_t qid)
{
    nicmgr_req_qstate_t qstate_req = {0};
    nicmgr_resp_qstate_t qstate_resp = {0};
    queue_info_t qinfo[8] = {0};

    assert(get_lif_qstate(lif, qinfo));

    if (qinfo[qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", qtype, lif);
        return;
    }

    if (qid >= qinfo[qtype].length) {
        printf("Invalid qid %u for lif %u qtype %u\n", qid, lif, qtype);
        return;
    }

    uint64_t addr = qinfo[qtype].base + qid * qinfo[qtype].size;
    printf("addr: %lx\n\n", addr);

    switch (qtype) {
    case 0:
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
               "enable=0x%0x\nintr_enable=0x%0x\n"
               "ring_base=0x%0lx\nring_size=0x%0x\n"
               "cq_ring_base=0x%0lx\nintr_assert_index=0x%0x\n",
               qstate_req.pc_offset, qstate_req.rsvd0, qstate_req.cosA, qstate_req.cosB,
               qstate_req.cos_sel, qstate_req.eval_last, qstate_req.host, qstate_req.total,
               qstate_req.pid, qstate_req.p_index0, qstate_req.c_index0, qstate_req.comp_index,
               qstate_req.ci_fetch, qstate_req.sta.color, qstate_req.cfg.enable,
               qstate_req.cfg.intr_enable, qstate_req.ring_base, qstate_req.ring_size,
               qstate_req.cq_ring_base, qstate_req.intr_assert_index);
        break;
    case 1:
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
               "enable=0x%0x\nintr_enable=0x%0x\n"
               "ring_base=0x%0lx\nring_size=0x%0x\n"
               "cq_ring_base=0x%0lx\nintr_assert_index=0x%0x\n",
               qstate_resp.pc_offset, qstate_resp.rsvd0, qstate_resp.cosA, qstate_resp.cosB,
               qstate_resp.cos_sel, qstate_resp.eval_last, qstate_resp.host, qstate_resp.total,
               qstate_resp.pid, qstate_resp.p_index0, qstate_resp.c_index0, qstate_resp.comp_index,
               qstate_resp.ci_fetch, qstate_resp.sta.color, qstate_resp.cfg.enable,
               qstate_resp.cfg.intr_enable, qstate_resp.ring_base, qstate_resp.ring_size,
               qstate_resp.cq_ring_base, qstate_resp.intr_assert_index);
        break;
    default:
        assert(0);
    }
}

void
nicmgr_qpoll(uint16_t lif, uint8_t qtype)
{
    nicmgr_req_qstate_t qstate_req = {0};
    nicmgr_resp_qstate_t qstate_resp = {0};
    queue_info_t qinfo[8] = {0};

    assert(get_lif_qstate(lif, qinfo));

    if (qinfo[qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", qtype, lif);
        return;
    }

    for (uint32_t qid = 0; qid < qinfo[qtype].length; qid++) {
        uint64_t addr = qinfo[qtype].base + qid * qinfo[qtype].size;
        uint32_t posted = 0;
        switch (qtype) {
        case 0:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_req, sizeof(qstate_req));
            posted =
                NUM_POSTED(1 << qstate_req.ring_size, qstate_req.p_index0, qstate_req.c_index0);
            printf("req%3d: head %6u tail %6u posted %6d comp_index %6u color %d\n", qid,
                   qstate_req.p_index0, qstate_req.c_index0, posted, qstate_req.comp_index,
                   qstate_req.sta.color);
            break;
        case 1:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_resp, sizeof(qstate_resp));
            posted =
                NUM_POSTED(1 << qstate_resp.ring_size, qstate_resp.p_index0, qstate_resp.c_index0);
            printf("resp%3d: head %6u tail %6u posted %6d comp_index %6u color %d\n", qid,
                   qstate_resp.p_index0, qstate_resp.c_index0, posted, qstate_resp.comp_index,
                   qstate_resp.sta.color);
            break;
        default:
            assert(0);
        }
    }
}

void
usage()
{
    printf("Usage:\n");
    printf("   qinfo          <lif>\n");
    printf("   qstate         <lif> <qtype> <qid>\n");
    printf("   qpoll          <lif> <qtype>\n");
    printf("   stats          <lif>\n");
    printf("   nicmgr_qstate  <lif> <qtype> <qid>\n");
    printf("   nicmgr_qpoll   <lif> <qtype>\n");
    printf("   memrd          <addr> <size_in_bytes>\n");
    printf("   memwr          <addr> <size_in_bytes> <bytes> ...\n");
    printf("   memdump        <addr> <size_in_bytes>\n");
    printf("   bzero          <addr> <size_in_bytes>\n");
    printf("   find           <addr> <size_in_bytes> <pattern>\n");
    printf("   nfind          <addr> <size_in_bytes> <pattern>\n");
    exit(1);
}

int
debug_logger(const char *format, ...)
{
    return 0;
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
    }

    sdk::lib::logger::init(&debug_logger, &debug_logger);

#ifdef __x86_64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM) ==
           sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HW) ==
           sdk::lib::PAL_RET_OK);
#endif

    if (strcmp(argv[1], "qinfo") == 0) {
        if (argc != 3) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        queue_info_t qinfo[8] = {0};
        assert(get_lif_qstate(lif, qinfo));
        for (int qtype = 0; qtype < 8; qtype++) {
            printf("qtype: %d    base: %12lx    size: %6u    count: %6u\n", qtype,
                   qinfo[qtype].base, qinfo[qtype].size, qinfo[qtype].length);
        }
    } else if (strcmp(argv[1], "qstate") == 0) {
        if (argc != 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        eth_qstate(lif, qtype, qid);
    } else if (strcmp(argv[1], "qpoll") == 0) {
        if (argc != 4) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        eth_qpoll(lif, qtype);
    } else if (strcmp(argv[1], "stats") == 0) {
        if (argc != 3) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        eth_stats(lif);
    } else if (strcmp(argv[1], "nicmgr_qstate") == 0) {
        if (argc != 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        nicmgr_qstate(lif, qtype, qid);
    } else if (strcmp(argv[1], "nicmgr_qpoll") == 0) {
        if (argc != 4) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        nicmgr_qpoll(lif, qtype);
    } else if (strcmp(argv[1], "memrd") == 0) {
        if (argc != 4) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint8_t *buf = (uint8_t *)calloc(1, size);
        assert(buf != NULL);
        sdk::lib::pal_mem_read(addr, buf, size);
        for (uint32_t i = 0; i < size / 16; i++) {
            printf("%0lx : ", addr + (i * 16));
            for (uint32_t j = 0; j < 16; j++) {
                printf("%02x ", buf[(i * 16) + j]);
            }
            printf("\n");
        }
        printf("\n");
        free(buf);
    } else if (strcmp(argv[1], "memwr") == 0) {
        if (argc <= 4) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        assert(argc == (int)(4 + size));
        uint8_t *buf = (uint8_t *)calloc(1, size);
        assert(buf != NULL);
        for (uint32_t i = 0; i < size; i++) {
            buf[i] = strtoul(argv[4 + i], NULL, 16);
        }
        sdk::lib::pal_mem_write(addr, buf, size);
        free(buf);
    } else if (strcmp(argv[1], "memdump") == 0) {
        if (argc != 5) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint8_t *buf = (uint8_t *)calloc(1, size);
        assert(buf != NULL);
        sdk::lib::pal_mem_read(addr, buf, size);
        FILE *fp = fopen(argv[4], "wb");
        assert(fp != NULL);
        fwrite(buf, sizeof(buf[0]), size, fp);
        fclose(fp);
    } else if (strcmp(argv[1], "bzero") == 0) {
        if (argc != 4) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint8_t *buf = (uint8_t *)calloc(1, size);
        assert(buf != NULL);
        sdk::lib::pal_mem_write(addr, buf, size);
        free(buf);
    } else if (strcmp(argv[1], "find") == 0) {
        if (argc != 5) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint32_t pattern = strtoul(argv[4], NULL, 0);
        uint32_t val;
        for (uint32_t i = 0; i < size / 4; i++) {
            sdk::lib::pal_mem_read(addr + i * 4, (uint8_t *)&val, sizeof(val));
            if (val == pattern) {
                printf("0x%lx\n", addr + i * 4);
                return 0;
            }
        }
        printf("Pattern 0x%x not found in region 0x%lx - 0x%lx\n", pattern, addr, addr + size);
        return -1;
    } else if (strcmp(argv[1], "nfind") == 0) {
        if (argc != 5) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint32_t pattern = strtoul(argv[4], NULL, 0);
        uint32_t val;
        for (uint32_t i = 0; i < size / 4; i++) {
            sdk::lib::pal_mem_read(addr + i * 4, (uint8_t *)&val, sizeof(val));
            if (val != pattern) {
                printf("0x%lx\n", addr + i * 4);
                return 0;
            }
        }
        printf("Pattern 0x%x not found in region 0x%lx - 0x%lx\n", pattern, addr, addr + size);
        return -1;
    } else {
        usage();
    }

    return 0;
}
