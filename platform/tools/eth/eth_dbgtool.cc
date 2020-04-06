
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "nic/include/adminq.h"
#include "nic/include/edmaq.h"
#include "nic/include/eth_common.h"
#include "nic/include/notify.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "lib/table/directmap/directmap.hpp"
#include "lib/table/common/table.hpp"
#include "platform/src/lib/nicmgr/include/eth_if.h"
#include "platform/src/lib/nicmgr/include/virtio_if.h"
#include "platform/utils/mpartition.hpp"
#include "nic/sdk/lib/device/device.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "platform/src/lib/nicmgr/include/rdma_dev.hpp"
#include "asic/pd/pd.hpp"

#include "gen/platform/mem_regions.hpp"

#include "third-party/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_wa_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_pics_c_hdr.h"

#if defined(APOLLO) || defined(ARTEMIS) || defined(APULU) || defined(ATHENA)
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd_table.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd_table.h"
#else
#include "gen/platform/mem_regions.hpp"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd_table.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd_table.h"
#endif

#define ENTRY_TRACE_EN                      true

// Maximum number of queue per LIF
#define ETH_RSS_MAX_QUEUES                  (128)
// Number of entries in a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_LEN           ETH_RSS_MAX_QUEUES
// Size of each LIF indirection table entry
#define ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ      (sizeof(eth_rx_rss_indir_eth_rx_rss_indir_t))
// Size of a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_SZ            (ETH_RSS_LIF_INDIR_TBL_LEN * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ)
// Max number of LIFs supported
#define MAX_LIFS                            (2048)
// Size of the entire LIF indirection table
#define ETH_RSS_INDIR_TBL_SZ                (MAX_LIFS * ETH_RSS_LIF_INDIR_TBL_SZ)
// Memory bar should be multiple of 8 MB
#define MEM_BARMAP_SIZE_SHIFT               (23)

#define QTYPE_MAX                           (8)

directmap    **p4plus_rxdma_dm_tables_;
directmap    **p4plus_txdma_dm_tables_;

using namespace sdk::asic::pd;
using namespace sdk::platform::utils;

typedef struct {
    uint64_t base;
    uint32_t size;
    uint32_t length;
} queue_info_t;

static void *
memrev(void *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = (uint8_t *)block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

bool
get_lif_qstate(uint16_t lif, queue_info_t qinfo[QTYPE_MAX])
{
    uint32_t cnt[4] = {0};
    uint32_t size[QTYPE_MAX] = {0};
    uint32_t length[QTYPE_MAX] = {0};

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

    for (uint8_t qtype = 0; qtype < QTYPE_MAX && size[qtype] != 0; qtype++) {

        qinfo[qtype].base = base;
        qinfo[qtype].size = 32 << size[qtype];
        qinfo[qtype].length = 1 << length[qtype];

        base += qinfo[qtype].size * qinfo[qtype].length;
    }

    return true;
}

#define RESET_HEADER() do { hdr_done = false; } while (0)
#define PRINT_HEADER(owner)                                             \
    do {                                                                \
        if (!hdr_done) {                                                \
            printf("\t" #owner "\n");                                   \
            hdr_done = true;                                            \
        }                                                               \
    } while (0);

#define PRINT_FLD16(owner, field, fmt)                                  \
    do {                                                                \
        if (owner->field) {                                             \
            PRINT_HEADER(owner);                                        \
            printf("\t\t%16s " fmt "\n", #field, owner->field);         \
        }                                                               \
    } while (0)

#define PRINT_FLD24(owner, field, fmt)                                  \
    do {                                                                \
        if (owner->field) {                                             \
            PRINT_HEADER(owner);                                        \
            printf("\t%24s " fmt "\n", #field, owner->field);           \
        }                                                               \
    } while (0)

#define PRINT_FLD32(owner, field, fmt)                                  \
    do {                                                                \
        if (owner->field) {                                             \
            PRINT_HEADER(owner);                                        \
            printf("%32s " fmt "\n", #field, owner->field);             \
        }                                                               \
    } while (0)

static void
rdma_qstate_one(queue_info_t *qinfo, uint8_t qtype, uint32_t qid)
{
    union {
        aqcb_t aq;
        sqcb_t sq;
        rqcb_t rq;
        cqcb0_t cq;
        eqcb0_t eq;
    } qstate;
    uint64_t addr = qinfo[qtype].base + qid * qinfo[qtype].size;

    aqcb0_t *aqcb0;
    aqcb1_t *aqcb1;
    sqcb0_t *sqcb0;
    sqcb1_t *sqcb1;
    sqcb2_t *sqcb2;
    // There is nothing in sqcb3 at present
    sqcb4_t *sqcb4;
    sqcb5_t *sqcb5;
    rqcb0_t *rqcb0;
    rqcb1_t *rqcb1;
    rqcb2_t *rqcb2;
    rqcb3_t *rqcb3;
    rqcb4_t *rqcb4;
    rqcb5_t *rqcb5;
    cqcb0_t *cqcb0;
    eqcb0_t *eqcb0;
    bool hdr_done;

    /*
     * SQ, RQ, and CQ must have total_rings set to be printed.
     * AQ and EQ must have log_num_wqes set to be printed.
     * Queue ID 0 may only be printed in the EQ case.
     */
    if (!qid && qtype != 6 /* EQ */)
        return;

    switch (qtype) {
    case 2: // AQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.aq, sizeof(qstate.aq));

        aqcb0 = (aqcb0_t *)memrev(&qstate.aq.aqcb0, sizeof *aqcb0);
        if (!aqcb0->log_num_wqes)
            break;

        printf("\naq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.aq));

        RESET_HEADER();
        PRINT_FLD24(aqcb0, map_count_completed, "%0x");
        PRINT_FLD16(aqcb0, first_pass, "%0x");
        PRINT_FLD16(aqcb0, token_id, "%0x");
        PRINT_FLD16(aqcb0, rsvd1[0], "%0x");
        PRINT_FLD16(aqcb0, rsvd1[1], "%0x");
        PRINT_FLD16(aqcb0, uplink_num, "%0x");
        PRINT_FLD16(aqcb0, debug, "%0x");
        PRINT_FLD16(aqcb0, log_wqe_size, "%0x");
        PRINT_FLD16(aqcb0, log_num_wqes, "%0x");
        PRINT_FLD32(aqcb0, ring_empty_sched_eval_done, "%0x");
        PRINT_FLD16(aqcb0, rsvd2, "%0x");
        PRINT_FLD16(aqcb0, phy_base_addr, "%0lx");
        PRINT_FLD16(aqcb0, next_token_id, "%0x");
        PRINT_FLD16(aqcb0, aq_id, "%0x");
        PRINT_FLD16(aqcb0, cq_id, "%0x");
        PRINT_FLD16(aqcb0, error, "%0x");
        PRINT_FLD16(aqcb0, cqcb_addr, "%0lx");

        aqcb1 = (aqcb1_t *)memrev(&qstate.aq.aqcb1, sizeof *aqcb1);
        RESET_HEADER();
        PRINT_FLD16(aqcb1, num_nop, "%0x");
        PRINT_FLD16(aqcb1, num_create_cq, "%0x");
        PRINT_FLD16(aqcb1, num_create_qp, "%0x");
        PRINT_FLD16(aqcb1, num_reg_mr, "%0x");
        PRINT_FLD16(aqcb1, num_stats_hdrs, "%0x");
        PRINT_FLD16(aqcb1, num_stats_vals, "%0x");
        PRINT_FLD16(aqcb1, num_dereg_mr, "%0x");
        PRINT_FLD16(aqcb1, num_resize_cq, "%0x");
        PRINT_FLD16(aqcb1, num_destroy_cq, "%0x");
        PRINT_FLD16(aqcb1, num_modify_qp, "%0x");
        PRINT_FLD16(aqcb1, num_query_qp, "%0x");
        PRINT_FLD16(aqcb1, num_destroy_qp, "%0x");
        PRINT_FLD16(aqcb1, num_stats_dump, "%0x");
        PRINT_FLD16(aqcb1, num_create_ah, "%0x");
        PRINT_FLD16(aqcb1, num_query_ah, "%0x");
        PRINT_FLD16(aqcb1, num_destroy_ah, "%0x");
        PRINT_FLD16(aqcb1, num_any, "%0lx");
        break;

    case 3: // SQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.sq, sizeof(qstate.sq));

        sqcb0 = (sqcb0_t *)memrev(&qstate.sq.sqcb0, sizeof *sqcb0);
        if (!sqcb0->total_rings)
            break;

        printf("\nsq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.sq));

        RESET_HEADER();
        PRINT_FLD16(sqcb0, pc, "%0x");
        PRINT_FLD16(sqcb0, rsvd, "%0x");
        PRINT_FLD16(sqcb0, cosA, "%0x");
        PRINT_FLD16(sqcb0, cosB, "%0x");
        PRINT_FLD16(sqcb0, cos_selector, "%0x");
        PRINT_FLD16(sqcb0, eval_last, "%0x");
        PRINT_FLD16(sqcb0, host_rings, "%0x");
        PRINT_FLD16(sqcb0, total_rings, "%0x");
        PRINT_FLD16(sqcb0, pid, "%0x");
        PRINT_FLD16(sqcb0, p_index0, "%0x");
        PRINT_FLD16(sqcb0, c_index0, "%0x");
        PRINT_FLD16(sqcb0, p_index1, "%0x");
        PRINT_FLD16(sqcb0, c_index1, "%0x");
        PRINT_FLD16(sqcb0, p_index2, "%0x");
        PRINT_FLD16(sqcb0, c_index2, "%0x");
        PRINT_FLD16(sqcb0, p_index3, "%0x");
        PRINT_FLD16(sqcb0, c_index3, "%0x");
        PRINT_FLD16(sqcb0, sqd_cindex, "%0lx");
        PRINT_FLD16(sqcb0, rsvd1, "%0lx");
        PRINT_FLD16(sqcb0, base_addr, "%0lx");
        PRINT_FLD24(sqcb0, header_template_addr, "%0x");
        PRINT_FLD16(sqcb0, pd, "%0x");
        PRINT_FLD16(sqcb0, poll_in_progress, "%0x");
        PRINT_FLD16(sqcb0, log_pmtu, "%0x");
        PRINT_FLD16(sqcb0, log_sq_page_size, "%0x");
        PRINT_FLD16(sqcb0, log_wqe_size, "%0x");
        PRINT_FLD16(sqcb0, log_num_wqes, "%0x");
        PRINT_FLD16(sqcb0, poll_for_work, "%0x");
        PRINT_FLD24(sqcb0, signalled_completion, "%0x");
        PRINT_FLD16(sqcb0, dcqcn_rl_failure, "%0x");
        PRINT_FLD16(sqcb0, service, "%0x");
        PRINT_FLD16(sqcb0, flush_rq, "%0x");
        PRINT_FLD16(sqcb0, state, "%0x");
        PRINT_FLD16(sqcb0, sq_in_hbm, "%0x");
        PRINT_FLD16(sqcb0, rsvd5, "%0x");
        PRINT_FLD24(sqcb0, local_ack_timeout, "%0x");
        PRINT_FLD32(sqcb0, ring_empty_sched_eval_done, "%0x");
        PRINT_FLD16(sqcb0, spec_sq_cindex, "%0x");
        PRINT_FLD16(sqcb0, curr_wqe_ptr, "%0lx");
        PRINT_FLD16(sqcb0, spec_msg_psn, "%0x");
        PRINT_FLD16(sqcb0, msg_psn, "%0x");
        PRINT_FLD16(sqcb0, sq_drained, "%0x");
        PRINT_FLD16(sqcb0, rsvd_state_flags, "%0x");
        PRINT_FLD16(sqcb0, state_flags, "%0x");
        PRINT_FLD16(sqcb0, spec_enable, "%0x");
        PRINT_FLD16(sqcb0, skip_pt, "%0x");
        PRINT_FLD32(sqcb0, bktrack_marker_in_progress, "%0x");
        PRINT_FLD24(sqcb0, congestion_mgmt_type, "%0x");
        PRINT_FLD16(sqcb0, rsvd2, "%0x\n");

        sqcb1 = (sqcb1_t *)memrev(&qstate.sq.sqcb1, sizeof *sqcb1);
        RESET_HEADER();
        PRINT_FLD16(sqcb1, pc, "%0x");
        PRINT_FLD16(sqcb1, cq_id, "%0x");
        PRINT_FLD16(sqcb1, rrq_pindex, "%0x");
        PRINT_FLD16(sqcb1, rrq_cindex, "%0x");
        PRINT_FLD16(sqcb1, rrq_base_addr, "%0x");
        PRINT_FLD16(sqcb1, log_rrq_size, "%0x");
        PRINT_FLD16(sqcb1, service, "%0x");
        PRINT_FLD16(sqcb1, rsvd8, "%0x");
        PRINT_FLD16(sqcb1, log_pmtu, "%0x");
        PRINT_FLD16(sqcb1, err_retry_count, "%0x");
        PRINT_FLD16(sqcb1, rnr_retry_count, "%0x");
        PRINT_FLD32(sqcb1, work_not_done_recirc_cnt, "%0x");
        PRINT_FLD16(sqcb1, tx_psn, "%0x");
        PRINT_FLD16(sqcb1, ssn, "%0x");
        PRINT_FLD16(sqcb1, rsvd2, "%0x");
        PRINT_FLD16(sqcb1, log_sqwqe_size, "%0x");
        PRINT_FLD16(sqcb1, pkt_spec_enable, "%0x");
        PRINT_FLD16(sqcb1, rsvd3, "%0x");
        PRINT_FLD24(sqcb1, header_template_addr, "%0x");
        PRINT_FLD24(sqcb1, header_template_size, "%0x");
        PRINT_FLD16(sqcb1, nxt_to_go_token_id, "%0x");
        PRINT_FLD16(sqcb1, token_id, "%0x");
        PRINT_FLD16(sqcb1, rsvd4, "%0x");
        PRINT_FLD16(sqcb1, rexmit_psn, "%0x");
        PRINT_FLD16(sqcb1, msn, "%0x");
        PRINT_FLD16(sqcb1, credits, "%0x");
        PRINT_FLD24(sqcb1, congestion_mgmt_type, "%0x");
        PRINT_FLD16(sqcb1, rsvd5, "%0x");
        PRINT_FLD16(sqcb1, max_tx_psn, "%0x");
        PRINT_FLD16(sqcb1, max_ssn, "%0x");
        PRINT_FLD16(sqcb1, rrqwqe_num_sges, "%0x");
        PRINT_FLD16(sqcb1, rrqwqe_cur_sge_id, "%0x");
        PRINT_FLD24(sqcb1, rrqwqe_cur_sge_offset, "%0x");
        PRINT_FLD16(sqcb1, state, "%0x");
        PRINT_FLD24(sqcb1, sqcb1_priv_oper_enable, "%0x");
        PRINT_FLD16(sqcb1, sq_drained, "%0x");
        PRINT_FLD24(sqcb1, sqd_async_notify_enable, "%0x");
        PRINT_FLD16(sqcb1, rsvd6, "%0x");
        PRINT_FLD24(sqcb1, bktrack_in_progress, "%0x");
        PRINT_FLD16(sqcb1, pd, "%0x");
        PRINT_FLD16(sqcb1, rrq_spec_cindex, "%0x");
        PRINT_FLD16(sqcb1, rsvd7, "%0x\n");

        sqcb2 = (sqcb2_t *)memrev(&qstate.sq.sqcb2, sizeof *sqcb2);
        RESET_HEADER();
        PRINT_FLD16(sqcb2, dst_qp, "%0x");
        PRINT_FLD24(sqcb2, header_template_size, "%0x");
        PRINT_FLD24(sqcb2, header_template_addr, "%0x");
        PRINT_FLD16(sqcb2, rrq_base_addr, "%0x");
        PRINT_FLD16(sqcb2, log_rrq_size, "%0x");
        PRINT_FLD16(sqcb2, log_sq_size, "%0x");
        PRINT_FLD24(sqcb2, roce_opt_ts_enable, "%0x");
        PRINT_FLD24(sqcb2, roce_opt_mss_enable, "%0x");
        PRINT_FLD16(sqcb2, service, "%0x");
        PRINT_FLD16(sqcb2, lsn_tx, "%0x");
        PRINT_FLD16(sqcb2, lsn_rx, "%0x");
        PRINT_FLD16(sqcb2, rexmit_psn, "%0lx");
        PRINT_FLD24(sqcb2, last_ack_or_req_ts, "%0lx");
        PRINT_FLD16(sqcb2, err_retry_ctr, "%0lx");
        PRINT_FLD16(sqcb2, rnr_retry_ctr, "%0lx");
        PRINT_FLD16(sqcb2, rnr_timeout, "%0lx");
        PRINT_FLD16(sqcb2, rsvd1, "%0lx");
        PRINT_FLD16(sqcb2, timer_on, "%0lx");
        PRINT_FLD24(sqcb2, local_ack_timeout, "%0lx");
        PRINT_FLD16(sqcb2, tx_psn, "%0lx");
        PRINT_FLD16(sqcb2, ssn, "%0lx");
        PRINT_FLD16(sqcb2, lsn, "%0lx");
        PRINT_FLD16(sqcb2, wqe_start_psn, "%0lx");
        PRINT_FLD16(sqcb2, inv_key, "%0x");
        PRINT_FLD16(sqcb2, sq_cindex, "%0x");
        PRINT_FLD16(sqcb2, rrq_pindex, "%0x");
        PRINT_FLD16(sqcb2, rrq_cindex, "%0x");
        PRINT_FLD16(sqcb2, busy, "%0x");
        PRINT_FLD16(sqcb2, need_credits, "%0x");
        PRINT_FLD16(sqcb2, rsvd2, "%0x");
        PRINT_FLD16(sqcb2, fence, "%0x");
        PRINT_FLD16(sqcb2, li_fence, "%0x");
        PRINT_FLD16(sqcb2, fence_done, "%0x");
        PRINT_FLD16(sqcb2, curr_op_type, "%0x");
        PRINT_FLD16(sqcb2, exp_rsp_psn, "%0x");
        PRINT_FLD16(sqcb2, timestamp, "%0x");
        PRINT_FLD16(sqcb2, disable_credits, "%0x");
        PRINT_FLD16(sqcb2, rsvd3, "%0x");
        PRINT_FLD16(sqcb2, sq_msg_psn, "%0x\n");

        sqcb4 = (sqcb4_t *)memrev(&qstate.sq.sqcb4, sizeof *sqcb4);
        RESET_HEADER();
        PRINT_FLD16(sqcb4, num_bytes, "%0lx");
        PRINT_FLD16(sqcb4, num_pkts, "%0x");
        PRINT_FLD16(sqcb4, num_send_msgs, "%0x");
        PRINT_FLD16(sqcb4, num_write_msgs, "%0x");
        PRINT_FLD24(sqcb4, num_read_req_msgs, "%0x");
        PRINT_FLD24(sqcb4, num_atomic_fna_msgs, "%0x");
        PRINT_FLD24(sqcb4, num_atomic_cswap_msgs, "%0x");
        PRINT_FLD24(sqcb4, num_send_msgs_inv_rkey, "%0x");
        PRINT_FLD24(sqcb4, num_send_msgs_imm_data, "%0x");
        PRINT_FLD24(sqcb4, num_write_msgs_imm_data, "%0x");
        PRINT_FLD24(sqcb4, num_pkts_in_cur_msg, "%0x");
        PRINT_FLD24(sqcb4, max_pkts_in_any_msg, "%0x");
        PRINT_FLD16(sqcb4, num_npg_req, "%0x");
        PRINT_FLD24(sqcb4, num_npg_bindmw_t1_req, "%0x");
        PRINT_FLD24(sqcb4, num_npg_bindmw_t2_req, "%0x");
        PRINT_FLD16(sqcb4, num_npg_frpmr_req, "%0x");
        PRINT_FLD24(sqcb4, num_npg_local_inv_req, "%0x");
        PRINT_FLD16(sqcb4, num_inline_req, "%0x");
        PRINT_FLD24(sqcb4, num_timeout_local_ack, "%0x");
        PRINT_FLD16(sqcb4, num_timeout_rnr, "%0x");
        PRINT_FLD16(sqcb4, num_sq_drains, "%0x");
        PRINT_FLD16(sqcb4, np_cnp_sent, "%0x");
        PRINT_FLD24(sqcb4, rp_num_byte_threshold_db, "%0x");
        PRINT_FLD16(sqcb4, qp_err_disabled, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_flush_rq, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_ud_pmtu, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_ud_fast_reg, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_ud_priv, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_no_dma_cmds, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_lkey_inv_state, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_lkey_inv_pd, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_lkey_rsvd_lkey, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_lkey_access_violation, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_len_exceeded, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_inv_pd, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_inv_zbva, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_inv_len, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_inv_mw_state, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_type_disallowed, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_state_valid, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_no_bind, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_zero_based, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_invalid_acc_ctrl, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_invalid_va, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bktrack_inv_num_sges, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bktrack_inv_rexmit_psn, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_fast_reg_not_enabled, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_invalid_pd, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_invalid_state, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_invalid_len, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_ukey_not_enabled, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_inv_lkey_qp_mismatch, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_inv_lkey_pd_mismatch, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_inv_lkey_invalid_state, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_inv_lkey_inv_not_allowed, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_table_error, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_phv_intrinsic_error, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_table_resp_error, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_inv_optype, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_err_retry_exceed, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_rnr_retry_exceed, "%0lx");
        PRINT_FLD16(sqcb4, qp_err_dis_rsvd, "%0lx\n");

        sqcb5 = (sqcb5_t *)memrev(&qstate.sq.sqcb5, sizeof *sqcb5);
        RESET_HEADER();
        PRINT_FLD16(sqcb5, num_bytes, "%0lx");
        PRINT_FLD16(sqcb5, num_pkts, "%0x");
        PRINT_FLD24(sqcb5, num_read_resp_pkts, "%0x");
        PRINT_FLD24(sqcb5, num_read_resp_msgs, "%0x");
        PRINT_FLD16(sqcb5, num_feedback, "%0x");
        PRINT_FLD16(sqcb5, num_ack, "%0x");
        PRINT_FLD16(sqcb5, num_atomic_ack, "%0x");
        PRINT_FLD24(sqcb5, num_pkts_in_cur_msg, "%0x");
        PRINT_FLD24(sqcb5, max_pkts_in_any_msg, "%0x");
        PRINT_FLD16(sqcb5, qp_err_disabled, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_pd_mismatch, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_inv_state, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_rsvd_lkey, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_acc_no_wr_perm, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_acc_len_lower, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_acc_len_higher, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqsge_insuff_sges, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqsge_insuff_sge_len, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqsge_insuff_dma_cmds, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqwqe_remote_inv_req_err_rcvd, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqwqe_remote_acc_err_rcvd, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqwqe_remote_oper_err_rcvd, "%0x");
        PRINT_FLD24(sqcb5, qp_err_dis_table_error, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_phv_intrinsic_error, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_table_resp_error, "%0x");
        PRINT_FLD16(sqcb5, qp_err_dis_rsvd, "%0x");
        PRINT_FLD16(sqcb5, recirc_bth_psn, "%0x");
        PRINT_FLD24(sqcb5, recirc_bth_opcode, "%0x");
        PRINT_FLD16(sqcb5, recirc_reason, "%0x");
        PRINT_FLD24(sqcb5, max_recirc_cnt_err, "%0x");
        PRINT_FLD16(sqcb5, rsvd1, "%0x");
        break;

    case 4: // RQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.rq, sizeof(qstate.rq));

        rqcb0 = (rqcb0_t *)memrev(&qstate.rq.rqcb0, sizeof *rqcb0);
        if (!rqcb0->total_rings)
            break;

        printf("\nrq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.rq));

        RESET_HEADER();
        PRINT_FLD16(rqcb0, log_pmtu, "%0x");
        PRINT_FLD16(rqcb0, serv_type, "%0x");
        PRINT_FLD16(rqcb0, bt_rsq_cindex, "%0x");
        PRINT_FLD16(rqcb0, rsvd1, "%0x");
        PRINT_FLD16(rqcb0, dcqcn_cfg_id, "%0x");
        PRINT_FLD32(rqcb0, ring_empty_sched_eval_done, "%0x");
        PRINT_FLD24(rqcb0, header_template_size, "%0x");
        PRINT_FLD24(rqcb0, curr_read_rsp_psn, "%0x");
        PRINT_FLD16(rqcb0, rsvd0, "%0x");
        PRINT_FLD16(rqcb0, drain_done, "%0x");
        PRINT_FLD16(rqcb0, bt_in_progress, "%0x");
        PRINT_FLD16(rqcb0, bt_lock, "%0x");
        PRINT_FLD16(rqcb0, rq_in_hbm, "%0x");
        PRINT_FLD24(rqcb0, read_rsp_in_progress, "%0x");
        PRINT_FLD16(rqcb0, curr_color, "%0x");
        PRINT_FLD16(rqcb0, dst_qp, "%0x");
        PRINT_FLD24(rqcb0, header_template_addr, "%0x");
        PRINT_FLD16(rqcb0, rsvd, "%0x");
        PRINT_FLD16(rqcb0, prefetch_en, "%0x");
        PRINT_FLD16(rqcb0, skip_pt, "%0x");
        PRINT_FLD16(rqcb0, drain_in_progress, "%0x");
        PRINT_FLD16(rqcb0, spec_color, "%0x");
        PRINT_FLD24(rqcb0, spec_read_rsp_psn, "%0x");
        PRINT_FLD16(rqcb0, rsq_base_addr, "%0x");
        PRINT_FLD16(rqcb0, phy_base_addr, "%0lx");
        PRINT_FLD16(rqcb0, log_rsq_size, "%0lx");
        PRINT_FLD16(rqcb0, state, "%0lx");
        PRINT_FLD24(rqcb0, congestion_mgmt_enable, "%0lx");
        PRINT_FLD16(rqcb0, log_num_wqes, "%0lx");
        PRINT_FLD16(rqcb0, log_wqe_size, "%0lx");
        PRINT_FLD16(rqcb0, log_rq_page_size, "%0lx");
        PRINT_FLD16(rqcb0, c_index5, "%0x");
        PRINT_FLD16(rqcb0, p_index5, "%0x");
        PRINT_FLD16(rqcb0, c_index4, "%0x");
        PRINT_FLD16(rqcb0, p_index4, "%0x");
        PRINT_FLD16(rqcb0, c_index3, "%0x");
        PRINT_FLD16(rqcb0, p_index3, "%0x");
        PRINT_FLD16(rqcb0, c_index2, "%0x");
        PRINT_FLD16(rqcb0, p_index2, "%0x");
        PRINT_FLD16(rqcb0, c_index1, "%0x");
        PRINT_FLD16(rqcb0, p_index1, "%0x");
        PRINT_FLD16(rqcb0, c_index0, "%0x");
        PRINT_FLD16(rqcb0, p_index0, "%0x");
        PRINT_FLD16(rqcb0, pid, "%0x");
        PRINT_FLD16(rqcb0, host_rings, "%0x");
        PRINT_FLD16(rqcb0, total_rings, "%0x");
        PRINT_FLD16(rqcb0, eval_last, "%0x");
        PRINT_FLD16(rqcb0, cos_selector, "%0x");
        PRINT_FLD16(rqcb0, cosA, "%0x");
        PRINT_FLD16(rqcb0, cosB, "%0x");
        PRINT_FLD16(rqcb0, intrinsic_rsvd, "%0x");
        PRINT_FLD16(rqcb0, pc, "%0x\n");

        rqcb1 = (rqcb1_t *)memrev(&qstate.rq.rqcb1, sizeof *rqcb1);
        RESET_HEADER();
        PRINT_FLD16(rqcb1, log_pmtu, "%0x");
        PRINT_FLD16(rqcb1, serv_type, "%0x");
        PRINT_FLD16(rqcb1, srq_id, "%0x");
        PRINT_FLD16(rqcb1, proxy_pindex, "%0x");
        PRINT_FLD16(rqcb1, proxy_cindex, "%0x");
        PRINT_FLD16(rqcb1, num_sges, "%0x");
        PRINT_FLD16(rqcb1, current_sge_id, "%0x");
        PRINT_FLD24(rqcb1, current_sge_offset, "%0x");
        PRINT_FLD16(rqcb1, curr_wqe_ptr, "%0lx");
        PRINT_FLD16(rqcb1, rsq_pindex, "%0x");
        PRINT_FLD16(rqcb1, bt_in_progress, "%0x");
        PRINT_FLD24(rqcb1, header_template_size, "%0x");
        PRINT_FLD16(rqcb1, msn, "%0x");
        PRINT_FLD16(rqcb1, rsvd3, "%0x");
        PRINT_FLD16(rqcb1, access_flags, "%0x");
        PRINT_FLD16(rqcb1, spec_en, "%0x");
        PRINT_FLD16(rqcb1, next_pkt_type, "%0x");
        PRINT_FLD16(rqcb1, next_op_type, "%0x");
        PRINT_FLD16(rqcb1, e_psn, "%0x");
        PRINT_FLD16(rqcb1, spec_cindex, "%0x");
        PRINT_FLD16(rqcb1, rsvd2, "%0x");
        PRINT_FLD16(rqcb1, in_progress, "%0x");
        PRINT_FLD16(rqcb1, rsvd1, "%0x");
        PRINT_FLD16(rqcb1, busy, "%0x");
        PRINT_FLD24(rqcb1, work_not_done_recirc_cnt, "%0x");
        PRINT_FLD24(rqcb1, nxt_to_go_token_id, "%0x");
        PRINT_FLD16(rqcb1, token_id, "%0x");
        PRINT_FLD24(rqcb1, header_template_addr, "%0x");
        PRINT_FLD16(rqcb1, pd, "%0x");
        PRINT_FLD16(rqcb1, cq_id, "%0x");
        PRINT_FLD16(rqcb1, prefetch_en, "%0x");
        PRINT_FLD16(rqcb1, skip_pt, "%0x");
        PRINT_FLD16(rqcb1, priv_oper_enable, "%0x");
        PRINT_FLD16(rqcb1, nak_prune, "%0x");
        PRINT_FLD16(rqcb1, rq_in_hbm, "%0x");
        PRINT_FLD16(rqcb1, immdt_as_dbell, "%0x");
        PRINT_FLD16(rqcb1, cache, "%0x");
        PRINT_FLD16(rqcb1, srq_enabled, "%0x");
        PRINT_FLD16(rqcb1, log_rsq_size, "%0lx");
        PRINT_FLD16(rqcb1, state, "%0lx");
        PRINT_FLD24(rqcb1, congestion_mgmt_enable, "%0lx");
        PRINT_FLD16(rqcb1, log_num_wqes, "%0lx");
        PRINT_FLD16(rqcb1, log_wqe_size, "%0lx");
        PRINT_FLD16(rqcb1, log_rq_page_size, "%0lx");
        PRINT_FLD16(rqcb1, phy_base_addr, "%0lx");
        PRINT_FLD16(rqcb1, rsq_base_addr, "%0x");
        PRINT_FLD16(rqcb1, pc, "%0x\n");

        rqcb2 = (rqcb2_t *)memrev(&qstate.rq.rqcb2, sizeof *rqcb2);
        RESET_HEADER();
        PRINT_FLD24(rqcb2, prefetch_init_done, "%0x");
        PRINT_FLD16(rqcb2, checkout_done, "%0x");
        PRINT_FLD24(rqcb2, prefetch_buf_index, "%0x");
        PRINT_FLD16(rqcb2, rsvd3, "%0x");
        PRINT_FLD24(rqcb2, log_num_prefetch_wqes, "%0x");
        PRINT_FLD24(rqcb2, prefetch_base_addr, "%0x");
        PRINT_FLD24(rqcb2, prefetch_proxy_cindex, "%0x");
        PRINT_FLD16(rqcb2, prefetch_pindex, "%0x");
        PRINT_FLD16(rqcb2, proxy_cindex, "%0x");
        PRINT_FLD16(rqcb2, pd, "%0x");
        PRINT_FLD16(rqcb2, rsvd2, "%0x");
        PRINT_FLD16(rqcb2, rnr_timeout, "%0x");
        PRINT_FLD16(rqcb2, len, "%0x");
        PRINT_FLD16(rqcb2, r_key, "%0x");
        PRINT_FLD16(rqcb2, va, "%0lx");
        PRINT_FLD16(rqcb2, psn, "%0x");
        PRINT_FLD16(rqcb2, rsvd, "%0x");
        PRINT_FLD16(rqcb2, read_or_atomic, "%0x");
        PRINT_FLD16(rqcb2, rsvd1, "%0x");
        PRINT_FLD16(rqcb2, credits, "%0x");
        PRINT_FLD16(rqcb2, syndrome, "%0x");
        PRINT_FLD16(rqcb2, msn, "%0x");
        PRINT_FLD16(rqcb2, ack_nak_psn, "%0x");
        PRINT_FLD16(rqcb2, rsvd0, "%0x\n");

        rqcb3 = (rqcb3_t *)memrev(&qstate.rq.rqcb3, sizeof *rqcb3);
        RESET_HEADER();
        PRINT_FLD24(rqcb3, resp_rx_timestamp, "%0lx");
        PRINT_FLD24(rqcb3, resp_tx_timestamp, "%0lx");
        PRINT_FLD16(rqcb3, dma_len, "%0x");
        PRINT_FLD24(rqcb3, num_pkts_in_curr_msg, "%0x");
        PRINT_FLD16(rqcb3, rsvd1, "%0x");
        PRINT_FLD16(rqcb3, roce_opt_mss, "%0x");
        PRINT_FLD16(rqcb3, roce_opt_ts_echo, "%0x");
        PRINT_FLD24(rqcb3, roce_opt_ts_value, "%0x");
        PRINT_FLD16(rqcb3, r_key, "%0x");
        PRINT_FLD16(rqcb3, len, "%0x");
        PRINT_FLD16(rqcb3, va, "%0lx");
        PRINT_FLD16(rqcb3, wrid, "%0lx");

        rqcb4 = (rqcb4_t *)memrev(&qstate.rq.rqcb4, sizeof *rqcb4);
        RESET_HEADER();
        PRINT_FLD24(rqcb4, qp_err_dis_resp_rx, "%0x");
        PRINT_FLD32(rqcb4, qp_err_dis_type2a_mw_qp_mismatch, "%0x");
        PRINT_FLD24(rqcb4, qp_err_dis_rkey_va_err, "%0x");
        PRINT_FLD32(rqcb4, qp_err_dis_rkey_acc_ctrl_err, "%0x");
        PRINT_FLD32(rqcb4, qp_err_dis_rkey_pd_mismatch, "%0x");
        PRINT_FLD32(rqcb4, qp_err_dis_rkey_state_err, "%0x");
        PRINT_FLD24(rqcb4, qp_err_dis_rsvd_rkey_err, "%0x");
        PRINT_FLD16(rqcb4, qp_err_disabled, "%0x");
        PRINT_FLD24(rqcb4, rp_num_max_rate_reached, "%0x");
        PRINT_FLD24(rqcb4, rp_num_timer_T_expiry, "%0x");
        PRINT_FLD32(rqcb4, rp_num_alpha_timer_expiry, "%0x");
        PRINT_FLD24(rqcb4, rp_num_byte_threshold_db, "%0x");
        PRINT_FLD24(rqcb4, rp_num_hyper_increase, "%0x");
        PRINT_FLD24(rqcb4, rp_num_fast_recovery, "%0x");
        PRINT_FLD24(rqcb4, rp_num_additive_increase, "%0x");
        PRINT_FLD16(rqcb4, last_msn, "%0x");
        PRINT_FLD16(rqcb4, last_syndrome, "%0x");
        PRINT_FLD16(rqcb4, last_psn, "%0x");
        PRINT_FLD16(rqcb4, num_seq_errs, "%0x");
        PRINT_FLD16(rqcb4, num_rnrs, "%0x");
        PRINT_FLD16(rqcb4, num_prefetch, "%0x");
        PRINT_FLD24(rqcb4, max_pkts_in_any_msg, "%0x");
        PRINT_FLD24(rqcb4, num_pkts_in_cur_msg, "%0x");
        PRINT_FLD24(rqcb4, num_atomic_resp_msgs, "%0x");
        PRINT_FLD24(rqcb4, num_read_resp_msgs, "%0x");
        PRINT_FLD16(rqcb4, num_acks, "%0x");
        PRINT_FLD24(rqcb4, num_read_resp_pkts, "%0x");
        PRINT_FLD16(rqcb4, num_pkts, "%0x");
        PRINT_FLD16(rqcb4, num_bytes, "%0lx\n");

        rqcb5 = (rqcb5_t *)memrev(&qstate.rq.rqcb5, sizeof *rqcb5);
        RESET_HEADER();
        PRINT_FLD24(rqcb5, max_recirc_cnt_err, "%0x");
        PRINT_FLD16(rqcb5, recirc_reason, "%0x");
        PRINT_FLD16(rqcb5, last_bth_opcode, "%0x");
        PRINT_FLD16(rqcb5, recirc_bth_psn, "%0x");
        PRINT_FLD16(rqcb5, qp_err_dis_rsvd, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_table_resp_error, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_phv_intrinsic_error, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_table_error, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_feedback, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_mr_cookie_mismatch, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_mr_state_invalid, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_mr_mw_pd_mismatch, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_type2a_mw_qp_mismatch, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_type1_mw_inv_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_inv_rkey_state_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_ineligible_mr_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_inv_rkey_rsvd_key_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_key_va_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_user_key_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_key_acc_ctrl_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_key_pd_mismatch, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_key_state_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_rsvd_key_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_max_sge_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_insuff_sge_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_dma_len_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_unaligned_atomic_va_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_wr_only_zero_len_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_access_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_opcode_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_pmtu_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_last_pkt_len_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_pyld_len_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_svc_type_err, "%0x");
        PRINT_FLD16(rqcb5, qp_err_disabled, "%0x");
        PRINT_FLD16(rqcb5, rp_cnp_processed, "%0x");
        PRINT_FLD24(rqcb5, np_ecn_marked_packets, "%0x");
        PRINT_FLD32(rqcb5, num_dup_rd_atomic_drop_pkts, "%0x");
        PRINT_FLD32(rqcb5, num_dup_rd_atomic_bt_pkts, "%0x");
        PRINT_FLD24(rqcb5, num_dup_wr_send_pkts, "%0x");
        PRINT_FLD24(rqcb5, num_mem_window_inv, "%0x");
        PRINT_FLD24(rqcb5, num_recirc_drop_pkts, "%0x");
        PRINT_FLD24(rqcb5, max_pkts_in_any_msg, "%0x");
        PRINT_FLD24(rqcb5, num_pkts_in_cur_msg, "%0x");
        PRINT_FLD16(rqcb5, num_ring_dbell, "%0x");
        PRINT_FLD16(rqcb5, num_ack_requested, "%0x");
        PRINT_FLD24(rqcb5, num_write_msgs_imm_data, "%0x");
        PRINT_FLD24(rqcb5, num_send_msgs_imm_data, "%0x");
        PRINT_FLD24(rqcb5, num_send_msgs_inv_rkey, "%0x");
        PRINT_FLD24(rqcb5, num_atomic_cswap_msgs, "%0x");
        PRINT_FLD24(rqcb5, num_atomic_fna_msgs, "%0x");
        PRINT_FLD16(rqcb5, num_read_req_msgs, "%0x");
        PRINT_FLD16(rqcb5, num_write_msgs, "%0x");
        PRINT_FLD16(rqcb5, num_send_msgs, "%0x");
        PRINT_FLD16(rqcb5, num_pkts, "%0x");
        PRINT_FLD16(rqcb5, num_bytes, "%0lx");
        break;

    case 5: //CQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.cq, sizeof(qstate.cq));

        cqcb0 = (cqcb0_t *)memrev(&qstate.cq, sizeof *cqcb0);
        if (!cqcb0->total_rings)
            break;

        printf("\ncq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.cq));

        RESET_HEADER();
        PRINT_FLD16(cqcb0, pc, "%0x");
        PRINT_FLD16(cqcb0, rsvd, "%0x");
        PRINT_FLD16(cqcb0, cosB, "%0x");
        PRINT_FLD16(cqcb0, cosA, "%0x");
        PRINT_FLD16(cqcb0, cos_selector, "%0x");
        PRINT_FLD16(cqcb0, eval_last, "%0x");
        PRINT_FLD16(cqcb0, total_rings, "%0x");
        PRINT_FLD16(cqcb0, host_rings, "%0x");
        PRINT_FLD16(cqcb0, pid, "%0x");
        PRINT_FLD16(cqcb0, p_index0, "%0x");
        PRINT_FLD16(cqcb0, c_index0, "%0x");
        PRINT_FLD16(cqcb0, p_index1, "%0x");
        PRINT_FLD16(cqcb0, c_index1, "%0x");
        PRINT_FLD16(cqcb0, p_index2, "%0x");
        PRINT_FLD16(cqcb0, c_index2, "%0x");
        PRINT_FLD16(cqcb0, proxy_pindex, "%0x");
        PRINT_FLD16(cqcb0, proxy_s_pindex, "%0x");
        PRINT_FLD16(cqcb0, pt_base_addr, "%0x");
        PRINT_FLD16(cqcb0, log_cq_page_size, "%0x");
        PRINT_FLD16(cqcb0, log_wqe_size, "%0x");
        PRINT_FLD16(cqcb0, log_num_wqes, "%0x");
        PRINT_FLD32(cqcb0, ring_empty_sched_eval_done, "%0x");
        PRINT_FLD16(cqcb0, cq_id, "%0x");
        PRINT_FLD16(cqcb0, eq_id, "%0x");
        PRINT_FLD16(cqcb0, arm, "%0x");
        PRINT_FLD16(cqcb0, sarm, "%0x");
        PRINT_FLD16(cqcb0, wakeup_dpath, "%0x");
        PRINT_FLD16(cqcb0, color, "%0x");
        PRINT_FLD16(cqcb0, wakeup_lif, "%0x");
        PRINT_FLD16(cqcb0, wakeup_qtype, "%0x");
        PRINT_FLD16(cqcb0, wakeup_qid, "%0x");
        PRINT_FLD16(cqcb0, wakeup_ring_id, "%0x");
        PRINT_FLD16(cqcb0, cq_full_hint, "%0x");
        PRINT_FLD16(cqcb0, cq_full, "%0x");
        PRINT_FLD16(cqcb0, pt_pg_index, "%0x");
        PRINT_FLD16(cqcb0, pt_next_pg_index, "%0x");
        PRINT_FLD16(cqcb0, host_addr, "%0x");
        PRINT_FLD16(cqcb0, is_phy_addr, "%0x");
        PRINT_FLD16(cqcb0, pad, "%0x");
        PRINT_FLD16(cqcb0, pt_pa, "%0lx");
        PRINT_FLD16(cqcb0, pt_next_pa, "%0lx");
        break;

    case 6: //EQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.eq, sizeof(qstate.eq));

        eqcb0 = (eqcb0_t *)memrev(&qstate.eq, sizeof *eqcb0);
        if (!eqcb0->log_num_wqes)
            break;

        printf("\neq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.eq));

        RESET_HEADER();
        PRINT_FLD16(eqcb0, pc, "%0x");
        PRINT_FLD16(eqcb0, rsvd, "%0x");
        PRINT_FLD16(eqcb0, cosb, "%0x");
        PRINT_FLD16(eqcb0, cosa, "%0x");
        PRINT_FLD16(eqcb0, cos_selector, "%0x");
        PRINT_FLD16(eqcb0, eval_last, "%0x");
        PRINT_FLD16(eqcb0, total_rings, "%0x");
        PRINT_FLD16(eqcb0, host_rings, "%0x");
        PRINT_FLD16(eqcb0, pid, "%0x");
        PRINT_FLD16(eqcb0, pindex, "%0x");
        PRINT_FLD16(eqcb0, cindex, "%0x");
        PRINT_FLD16(eqcb0, eqe_base_addr, "%0lx");
        PRINT_FLD16(eqcb0, rsvd0, "%0x");
        PRINT_FLD16(eqcb0, eq_id, "%0x");
        PRINT_FLD16(eqcb0, log_num_wqes, "%0x");
        PRINT_FLD16(eqcb0, log_wqe_size, "%0x");
        PRINT_FLD16(eqcb0, int_enabled, "%0x");
        PRINT_FLD16(eqcb0, color, "%0x");
        PRINT_FLD16(eqcb0, rsvd1, "%0x");
        PRINT_FLD16(eqcb0, int_assert_addr, "%0lx");
        PRINT_FLD16(eqcb0, rsvd2[0], "%0lx");
        PRINT_FLD16(eqcb0, rsvd2[1], "%0lx");
        PRINT_FLD16(eqcb0, rsvd2[2], "%0lx");
        break;

    default:
        break;
    }
}

static const char qnames[][16] = {
    "unknown", "unknown", "AQ", "SQ", "RQ", "CQ", "EQ", "unknown"
};

void
rdma_qstate(uint16_t lif, uint8_t qtype, uint32_t qid)
{
    queue_info_t qinfo[QTYPE_MAX] = {0};

    if (!get_lif_qstate(lif, qinfo)) {
        printf("Failed to get qinfo for lif %u\n", lif);
        return;
    }

    if (qtype >= QTYPE_MAX || qinfo[qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", qtype, lif);
        return;
    }

    if (qid >= qinfo[qtype].length) {
        printf("Invalid qid %u for lif %u qtype %u (%s)\n",
	       qid, lif, qtype, qnames[qtype]);
        return;
    }

    /* Print the state of a single queue */
    printf("RDMA queue state for lif %u qtype %u (%s) qid %u:\n",
	   lif, qtype, qnames[qtype], qid);
    rdma_qstate_one(qinfo, qtype, qid);
}

void
rdma_qstate_all(uint16_t lif, uint8_t qtype)
{
    queue_info_t qinfo[QTYPE_MAX] = {0};
    uint8_t qtype_idx;
    uint8_t start_qtype = 0;
    uint8_t end_qtype = QTYPE_MAX;
    uint32_t qid;

    if (!get_lif_qstate(lif, qinfo)) {
        printf("Failed to get qinfo for lif %u\n", lif);
        return;
    }

    // specific qtype
    if (qtype != QTYPE_MAX) {
        start_qtype = qtype;
        end_qtype = qtype + 1;
    }

    if (start_qtype >= QTYPE_MAX || qinfo[start_qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", start_qtype, lif);
        return;
    }

    /* Print the state of every queue */
    printf("RDMA queue state for lif %u:\n", lif);
    for (qtype_idx = start_qtype; qtype_idx < end_qtype; qtype_idx++) {
        if (qinfo[qtype_idx].size == 0)
            continue;

        for (qid = 0; qid < qinfo[qtype_idx].length; qid++)
            rdma_qstate_one(qinfo, qtype_idx, qid);
    }
}

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

void
eth_qpoll(uint16_t lif, uint8_t qtype)
{
    union {
        eth_rx_qstate_t ethrx;
        eth_tx_co_qstate_t ethtx;
    } qstate = {0};
    struct admin_qstate qstate_ethaq = {0};
    struct notify_qstate qstate_notifyq = {0};
    struct edma_qstate qstate_edmaq = {0};
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

void
print_eth_qstate_ring(eth_qstate_ring_t *ring, int i)
{
    printf("p_index%d=%#x\n"            "c_index%d=%#x\n",
           i, ring->p_index,            i, ring->c_index);
}

void
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

void
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

void
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

void
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

void
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
            struct notify_qstate qstate = {0};
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
            struct edma_qstate qstate = {0};
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
        case 1:
            addr += offsetof(eth_qstate_common_t, cfg);
            sdk::lib::pal_mem_read(addr, (uint8_t *)&cfg.eth, sizeof(cfg.eth));
            cfg.eth.debug = enable;
            sdk::lib::pal_mem_write(addr, (uint8_t *)&cfg.eth, sizeof(cfg.eth));
            break;
        case 2:
            sdk::lib::pal_mem_read(addr + offsetof(admin_qstate_t, cfg),
                (uint8_t *)&qstate_aq, sizeof(qstate_aq));
            qstate_aq.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(admin_qstate_t, cfg),
                (uint8_t *)&qstate_aq, sizeof(qstate_aq));
            break;
        case 7:
        if (qid == 0) {
            sdk::lib::pal_mem_read(addr + offsetof(struct notify_qstate, cfg),
                (uint8_t *)&qstate_notifyq, sizeof(qstate_notifyq));
            qstate_notifyq.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(struct notify_qstate, cfg),
                (uint8_t *)&qstate_notifyq, sizeof(qstate_notifyq));
        }
        if (qid == 1) {
            sdk::lib::pal_mem_read(addr + offsetof(struct edma_qstate, cfg),
                (uint8_t *)&qstate_edmaq, sizeof(qstate_edmaq));
            qstate_edmaq.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(struct edma_qstate, cfg),
                (uint8_t *)&qstate_edmaq, sizeof(qstate_edmaq));
        }
        if (qid == 2) {
            sdk::lib::pal_mem_read(addr + offsetof(nicmgr_req_qstate_t, cfg),
                (uint8_t *)&qstate_req, sizeof(qstate_req));
            qstate_req.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(nicmgr_req_qstate_t, cfg),
                (uint8_t *)&qstate_req, sizeof(qstate_req));
        }
        if (qid == 3) {
            sdk::lib::pal_mem_read(addr + offsetof(nicmgr_resp_qstate_t, cfg),
                (uint8_t *)&qstate_resp, sizeof(qstate_resp));
            qstate_resp.debug = enable;
            sdk::lib::pal_mem_write(addr + offsetof(nicmgr_resp_qstate_t, cfg),
                (uint8_t *)&qstate_resp, sizeof(qstate_resp));
        }
    }

    uint32_t val = 0x1;
    sdk::lib::pal_reg_write(CAP_ADDR_BASE_RPC_PICS_OFFSET + \
        CAP_PICS_CSR_PICC_BYTE_OFFSET + \
        CAP_PICC_CSR_DHS_CACHE_INVALIDATE_BYTE_OFFSET, &val, 1);
    sdk::lib::pal_reg_write(CAP_ADDR_BASE_TPC_PICS_OFFSET + \
        CAP_PICS_CSR_PICC_BYTE_OFFSET + \
        CAP_PICC_CSR_DHS_CACHE_INVALIDATE_BYTE_OFFSET, &val, 1);

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

std::string
hal_cfg_path()
{
    std::string hal_cfg_path_;
    if (std::getenv("HAL_CONFIG_PATH") == NULL) {
        hal_cfg_path_ = "/nic/conf/";
    } else {
        hal_cfg_path_ = std::string(std::getenv("HAL_CONFIG_PATH"));
    }

    return hal_cfg_path_;
}

std::string
mpart_cfg_path()
{
    std::string mpart_json;
    std::string hal_cfg_path_ = hal_cfg_path();
    sdk::lib::device *device =
        sdk::lib::device::factory("/sysconfig/config0/device.conf");
    sdk::lib::dev_forwarding_mode_t fwd_mode = device->get_forwarding_mode();

    // WARNING -- this must be picked based on profile, this is guaranteed to be
    // broken soon
#if defined(APOLLO)
    mpart_json = hal_cfg_path_ + "/apollo/hbm_mem.json";
#elif defined(ARTEMIS)
    mpart_json = hal_cfg_path_ + "/artemis/hbm_mem.json";
#elif defined(APULU)
    mpart_json = hal_cfg_path_ + "/apulu/8g/hbm_mem.json";
#elif defined(ATHENA)
    mpart_json = hal_cfg_path_ + "/athena/hbm_mem.json";
#else
    if (fwd_mode == sdk::lib::FORWARDING_MODE_HOSTPIN ||
        fwd_mode == sdk::lib::FORWARDING_MODE_SWITCH)
        mpart_json = hal_cfg_path_ + "/iris/hbm_mem.json";
    else
        mpart_json = hal_cfg_path_ + "/iris/hbm_classic_mem.json" ;
#endif

    return mpart_json;
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
port_config(uint64_t addr)
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
port_status(uint64_t addr)
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
lif_status(uint64_t addr)
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

#if defined(APOLLO) || defined(ARTEMIS) || defined(APULU) || defined(ATHENA)
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN            P4_P4PLUS_RXDMA_TBL_ID_INDEX_MIN
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX            P4_P4PLUS_RXDMA_TBL_ID_INDEX_MAX
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN            P4_P4PLUS_TXDMA_TBL_ID_INDEX_MIN
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX            P4_P4PLUS_TXDMA_TBL_ID_INDEX_MAX
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN               P4_P4PLUS_RXDMA_TBL_ID_TBLMIN
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX               P4_P4PLUS_RXDMA_TBL_ID_TBLMAX
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN               P4_P4PLUS_TXDMA_TBL_ID_TBLMIN
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX               P4_P4PLUS_TXDMA_TBL_ID_TBLMAX
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS    P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_PARAMS
#endif

void table_health_monitor(uint32_t table_id,
                          char *name,
                          table_health_state_t curr_state,
                          uint32_t capacity,
                          uint32_t usage,
                          table_health_state_t *new_state)
{
    printf("table id: %d, name: %s, capacity: %d, "
            "usage: %d, curr state: %d, new state: %d",
            table_id, name, capacity, usage, curr_state, *new_state);
}

int
p4plus_rxdma_init_tables()
{
    uint32_t                   tid;
    p4pd_table_properties_t    tinfo;
    p4pd_error_t               rc;
    std::string hal_cfg_path_ = hal_cfg_path();

#if defined(APOLLO)
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "apollo/capri_rxdma_table_map.json",
            .p4pd_pgm_name       = "apollo",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(ARTEMIS)
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "artemis/capri_rxdma_table_map.json",
            .p4pd_pgm_name       = "artemis",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(APULU)
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "apulu/capri_rxdma_table_map.json",
            .p4pd_pgm_name       = "apulu",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(ATHENA)
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "athena/capri_rxdma_table_map.json",
            .p4pd_pgm_name       = "apollo", /* FIXME - change to athena when ready */
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#else
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "iris/capri_p4_rxdma_table_map.json",
            .p4pd_pgm_name       = "iris",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#endif

    memset(&tinfo, 0, sizeof(tinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_rxdma_init(&p4pd_cfg);
    assert(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_rxdma_dm_tables_ =
            (directmap **)calloc(sizeof(directmap *),
            (P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX -
             P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    assert(p4plus_rxdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_rxdma_table_properties_get(tid, &tinfo);
        assert(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
            case P4_TBL_TYPE_INDEX:
                if (tinfo.tabledepth) {
                    p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                        directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                           false, ENTRY_TRACE_EN, table_health_monitor);
                    assert(p4plus_rxdma_dm_tables_
                           [tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
                }
                break;

            case P4_TBL_TYPE_MPU:
            default:
                break;
        }
    }

    return 0;
}

int
p4plus_txdma_init_tables()
{
    uint32_t                   tid;
    p4pd_table_properties_t    tinfo;
    p4pd_error_t               rc;
    std::string hal_cfg_path_ = hal_cfg_path();

#if defined(APOLLO)
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "apollo/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apollo",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(ARTEMIS)
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "artemis/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "artemis",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(APULU)
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "apulu/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apulu",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(ATHENA)
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "athena/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apollo", /* FIXME - change to athena when ready */
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#else
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "iris/capri_p4_txdma_table_map.json",
        .p4pd_pgm_name       = "iris",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#endif

    memset(&tinfo, 0, sizeof(tinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_txdma_init(&p4pd_cfg);
    assert(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_txdma_dm_tables_ =
        (directmap **)calloc(sizeof(directmap *),
                             (P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                              P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    SDK_ASSERT(p4plus_txdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_txdma_table_properties_get(tid, &tinfo);
        SDK_ASSERT(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
        case P4_TBL_TYPE_INDEX:
            if (tinfo.tabledepth) {
                p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                    directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                       false, ENTRY_TRACE_EN, table_health_monitor);
                assert(p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
            }
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return 0;
}

void
pd_init()
{
    int ret;
    asicpd_state_pd_init(NULL);

    ret = p4plus_rxdma_init_tables();
    assert(ret == 0);
    ret = p4plus_txdma_init_tables();
    assert(ret == 0);

#if !defined(APOLLO) || !defined(ARTEMIS) || !defined(APULU) || !defined(ATHENA)
    ret = asicpd_p4plus_table_rw_init();
    assert(ret == 0);
#endif
}

int
p4pd_common_p4plus_rxdma_rss_params_table_entry_get(uint32_t hw_lif_id,
    eth_rx_rss_params_actiondata_t *data)
{
    p4pd_error_t        pd_err;

    assert(hw_lif_id < MAX_LIFS);

    pd_init();

    pd_err = p4pd_global_entry_read(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                    hw_lif_id, NULL, NULL, data);
    if (pd_err != P4PD_SUCCESS) {
        assert(0);
    }

    return 0;
}

void
p4pd_common_p4plus_rxdma_rss_params_table_entry_show(uint32_t hw_lif_id)
{
    eth_rx_rss_params_actiondata_t data = { 0 };

    p4pd_common_p4plus_rxdma_rss_params_table_entry_get(hw_lif_id, &data);

    memrev((uint8_t *)&data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    printf("type:  %x\n", data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_type);
    printf("key:   ");
    for (uint16_t i = 0; i < sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key); i++) {
        if (i != 0)
            printf(":");
        printf("%02x", data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key[i]);
    }
    printf("\n");
    printf("debug: %x\n", data.action_u.eth_rx_rss_params_eth_rx_rss_params.debug);
}

int
p4pd_common_p4plus_rxdma_rss_params_table_entry_add(uint32_t hw_lif_id,
    uint8_t debug)
{
    p4pd_error_t        pd_err;
    eth_rx_rss_params_actiondata_t data = { 0 };

    assert(hw_lif_id < MAX_LIFS);

    p4pd_common_p4plus_rxdma_rss_params_table_entry_get(hw_lif_id, &data);

    data.action_u.eth_rx_rss_params_eth_rx_rss_params.debug = debug;

    pd_err = p4pd_global_entry_write(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                     hw_lif_id, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        assert(0);
    }

    return 0;
}

int
p4pd_common_p4plus_rxdma_rss_indir_table_entry_show(uint32_t hw_lif_id)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;
    uint8_t index;
    eth_rx_rss_indir_actiondata_t data;

    std::string mpart_json = mpart_cfg_path();
    mpartition *mp_ = mpartition::factory(mpart_json.c_str());
    assert(mp_);

    if (hw_lif_id >= MAX_LIFS) {
        printf("Invalid lif %d\n", hw_lif_id);
        return -1;
    };

    tbl_base = mp_->start_addr(MEM_REGION_RSS_INDIR_TABLE_NAME);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    tbl_base += (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ);

    printf("ind_table: 0x%lx\n\t", tbl_base);
    for (index = 0; index < ETH_RSS_MAX_QUEUES; index++) {
        tbl_index = (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
        addr = tbl_base + tbl_index;
        sdk::lib::pal_mem_read(addr, (uint8_t *)&data.action_u,
                        sizeof(data.action_u), 0);
        printf("[%3d] %3d ", index,
            data.action_u.eth_rx_rss_indir_eth_rx_rss_indir.qid);
        if ((index + 1) % 8 == 0)
            printf("\n\t");
    }

    return 0;
}

void
qinfo(uint16_t lif)
{
    queue_info_t qinfo[QTYPE_MAX] = {0};

    if (!get_lif_qstate(lif, qinfo)) {
        printf("Failed to get qinfo for lif %u\n", lif);
        return;
    }

    for (int qtype = 0; qtype < 8; qtype++) {
        printf("qtype: %d    base: %12lx    size: %6u    count: %6u\n", qtype,
                qinfo[qtype].base, qinfo[qtype].size, qinfo[qtype].length);
    }
}

int
debug_logger (uint32_t mod_id, sdk_trace_level_e trace_level,
              const char *format, ...)
{
    return 0;
}

void
usage()
{
    printf("Usage:\n");
    printf("   qinfo          <lif>\n");
    printf("   qstate         <lif> <qtype> <qid>\n");
    printf("   eqstate        <addr>\n");
    printf("   qdump          <lif> <qtype> <qid> <ring>\n");
    printf("   debug          <lif> <qtype> <qid> <enable>\n");
    printf("   nvme_qstate    <lif> <qtype> <qid>\n");
    printf("   rdma_qstate    <lif> <qtype> <qid>\n");
    printf("   virtio_qstate  <lif> <qtype> <qid>\n");
    printf("   qpoll          <lif> <qtype>\n");
    printf("   stats          <lif>\n");
    printf("   stats_reset    <lif>\n");
    printf("   memrd          <addr> <size_in_bytes>\n");
    printf("   memwr          <addr> <size_in_bytes> <bytes> ...\n");
    printf("   memdump        <addr> <size_in_bytes>\n");
    printf("   bzero          <addr> <size_in_bytes>\n");
    printf("   find           <addr> <size_in_bytes> <pattern>\n");
    printf("   nfind          <addr> <size_in_bytes> <pattern>\n");
    printf("   port_config    <addr>\n");
    printf("   port_status    <addr>\n");
    printf("   lif_status     <addr>\n");
    printf("   rss            <lif>\n");
    printf("   rss_debug      <lif> <enable>\n");
    exit(1);
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
    }

    sdk::lib::logger::init(&debug_logger);

#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) ==
           sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW) ==
           sdk::lib::PAL_RET_OK);
#endif

    if (strcmp(argv[1], "qinfo") == 0) {
        if (argc != 3) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        qinfo(lif);
    } else if (strcmp(argv[1], "qstate") == 0) {
        if (argc != 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        eth_qstate(lif, qtype, qid);
    } else if (strcmp(argv[1], "qdump") == 0) {
        if (argc != 6) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        uint8_t ring = std::strtoul(argv[5], NULL, 0);
        eth_qdump(lif, qtype, qid, ring);
    } else if (strcmp(argv[1], "debug") == 0) {
        if (argc != 6) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        uint8_t enable = std::strtoul(argv[5], NULL, 0);
        eth_debug(lif, qtype, qid, enable);
    } else if (strcmp(argv[1], "eqstate") == 0) {
        if (argc != 3) {
            usage();
        }
        uint64_t addr = std::strtoull(argv[2], NULL, 0);
        eth_eqstate(addr);
    } else if (strcmp(argv[1], "nvme_qstate") == 0) {
        if (argc != 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        nvme_qstate(lif, qtype, qid);
    } else if (strcmp(argv[1], "rdma_qstate") == 0) {
        if (argc < 3 || argc > 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);

        if (argc == 3) {
            rdma_qstate_all(lif, QTYPE_MAX);
        } else if (argc == 4) {
            uint8_t qtype = std::strtoul(argv[3], NULL, 0);
            rdma_qstate_all(lif, qtype);
        } else {
            uint8_t qtype = std::strtoul(argv[3], NULL, 0);
            uint32_t qid = std::strtoul(argv[4], NULL, 0);
            rdma_qstate(lif, qtype, qid);
        }
    } else if (strcmp(argv[1], "virtio_qstate") == 0) {
        if (argc != 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        virtio_qstate(lif, qtype, qid);
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
    } else if (strcmp(argv[1], "stats_reset") == 0) {
        if (argc != 3) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        eth_stats_reset(lif);
        eth_stats(lif);
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
        if (argc == (int)(3 + size)) {
            printf("Not enough bytes to write\n");
            usage();
        }
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
    } else if (strcmp(argv[1], "port_config") == 0) {
        if (argc != 3) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        port_config(addr);
    } else if (strcmp(argv[1], "port_status") == 0) {
        if (argc != 3) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        port_status(addr);
    } else if (strcmp(argv[1], "lif_status") == 0) {
        if (argc != 3) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        lif_status(addr);
    } else if (strcmp(argv[1], "rss") == 0) {
        if (argc != 3) {
            usage();
        }
        uint16_t lif_id = strtoul(argv[2], NULL, 0);
        printf("\n");
        p4pd_common_p4plus_rxdma_rss_params_table_entry_show(lif_id);
        p4pd_common_p4plus_rxdma_rss_indir_table_entry_show(lif_id);
        printf("\n");
    } else if (strcmp(argv[1], "rss_debug") == 0) {
        if (argc != 4) {
            usage();
        }
        uint16_t lif_id = strtoul(argv[2], NULL, 0);
        uint8_t enable = std::strtoul(argv[3], NULL, 0);
        p4pd_common_p4plus_rxdma_rss_params_table_entry_add(lif_id, enable);
        printf("\n");
        p4pd_common_p4plus_rxdma_rss_params_table_entry_show(lif_id);
        p4pd_common_p4plus_rxdma_rss_indir_table_entry_show(lif_id);
        printf("\n");
    } else {
        usage();
    }

    return 0;
}
