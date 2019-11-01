#ifndef __SQCB_H
#define __SQCB_H
#include "capri.h"

#define MAX_SQ_RINGS            5
#define MAX_SQ_DOORBELL_RINGS   (MAX_SQ_RINGS - 1)
#define MAX_SQ_HOST_RINGS       1

#define SQ_RING_ID              0
#define SQ_BKTRACK_RING_ID      1
#define TIMER_RING_ID           2
#define CNP_RING_ID             3
#define RRQ_RING_ID	        (MAX_SQ_RINGS - 1)

#define CNP_PRI                 0
#define SQ_BKTRACK_PRI          1
#define TIMER_PRI               2
#define SQ_PRI                  3

#define SQ_RING_ID_BITMAP            0x01 // (1 << SQ_RING_ID)
#define SQ_BKTRACK_RING_ID_BITMAP    0x02 // (1 << SQ_BACKTRACK_RING_ID)
#define TIMER_RING_ID_BITMAP         0x04 // (1 << TIMER_RING_ID)
#define CNP_RING_ID_BITMAP           0x08 // (1 << CNP_RING_ID)
#define RRQ_RING_ID_BITMAP           0x10 // (1 << RRQ_RING_ID)

#define SQ_P_INDEX                   d.{ring0.pindex}.hx
#define SQ_C_INDEX                   d.{ring0.cindex}.hx
#define SQ_BKTRACK_P_INDEX           d.{ring1.pindex}.hx
#define SQ_BKTRACK_C_INDEX           d.{ring1.cindex}.hx
#define SQ_TIMER_P_INDEX             d.{ring2.pindex}.hx
#define SQ_TIMER_C_INDEX             d.{ring2.cindex}.hx
#define CNP_P_INDEX                  d.{ring3.pindex}.hx
#define CNP_C_INDEX                  d.{ring3.cindex}.hx
#define RRQ_P_INDEX                  d.{rrq_pindex}
#define RRQ_C_INDEX                  d.{rrq_cindex}

#define SPEC_SQ_C_INDEX              d.spec_sq_cindex

#define RRQ_P_INDEX_OFFSET           FIELD_OFFSET(sqcb1_t, rrq_pindex)
#define RRQ_C_INDEX_OFFSET           FIELD_OFFSET(sqcb1_t, rrq_cindex)
#define SQCB2_LSN_RX_OFFSET          FIELD_OFFSET(sqcb2_t, lsn_rx)
#define SQCB2_REXMIT_PSN_OFFSET      FIELD_OFFSET(sqcb2_t, rexmit_psn)
#define SQCB2_MSN_CREDITS_BYTES      4
#define SQCB2_RNR_TIMEOUT_OFFSET     FIELD_OFFSET(sqcb2_t, rnr_timeout)

#define SQCB_T struct sqcb_t
#define SQCB0_T struct sqcb0_t
#define SQCB1_T struct sqcb1_t

#define SIZEOF_TOKEN_ID_BITS  8

#define SQCB_SQ_PINDEX_OFFSET        FIELD_OFFSET(sqcb0_t, ring0.pindex)
#define SQCB_SQ_CINDEX_OFFSET        FIELD_OFFSET(sqcb0_t, ring0.cindex)
#define SQCB_SPEC_SQ_CINDEX_OFFSET   FIELD_OFFSET(sqcb0_t, spec_sq_cindex)
#define SQCB_CURR_OPTYPE_OFFSET      FIELD_OFFSET(sqcb0_t, curr_op_type)
#define SQCB_CURR_WQE_PTR_OFFSET     FIELD_OFFSET(sqcb0_t, curr_wqe_ptr)
#define SQCB_CURRENT_SGE_OFFSET      FIELD_OFFSET(sqcb0_t, current_sge_offset)

#define SQCB0_FRPMR_IN_PROGRESS_BIT_OFFSET   4
#define SQCB0_FLUSH_RQ_BIT_OFFSET            3
#define SQCB0_SERVICE_BIT_OFFSET             4
struct sqcb0_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;
    struct capri_intrinsic_ring_t ring1;
    struct capri_intrinsic_ring_t ring2;
    struct capri_intrinsic_ring_t ring3;
    sqd_cindex                    : 16; // RO S0, WO S5
    rsvd1                         :  8;

    union {
        struct {
            rsvd3                 : 8;
            pt_base_addr          : 32; // RO
        };
        struct {
            rsvd4                 : 8;
            hbm_sq_base_addr      : 32; // RO
        };
        phy_base_addr             : 40;
    };
    union {
        header_template_addr      : 32; // RO
        q_key                     : 32; // RO S0
    };
    pd                            : 32; // RO

    poll_in_progress              : 1;  // WO S5, RW S0
    log_pmtu                      : 5;  // RO
    log_sq_page_size              : 5;  // RO
    log_wqe_size                  : 5;  // RO
    log_num_wqes                  : 5;  // RO
    poll_for_work                 : 1;  // RO
    signalled_completion          : 1;  // RO
    dcqcn_rl_failure              : 1;  // RW S0, RW S5

    service                       : 4;  // RO
    flush_rq                      : 1;  // RW S0 (W0 RXDMA)
    state                         : 3;  // RO S0 (WO RXDMA)

    sq_in_hbm                     : 1;  // RO
    rsvd5                         : 1;  // RO
    local_ack_timeout             : 5;  // RO
    ring_empty_sched_eval_done    : 1;  // RW S0

    spec_sq_cindex                : 16; // RW S0

    curr_wqe_ptr                  : 64; // WO S5, RO S0
    
    union {
        struct {
            current_sge_id        : 8;  // WO S5, RO S0
            num_sges              : 8;  // WO S5, RO S0
            current_sge_offset    : 32; // WO S5, RO S0
        };
        struct {
            spec_msg_psn          : 24; // RW S0 
            union {
                msg_psn           : 24; // RW S5
                read_req_adjust   : 24; // RO S0, WO S5
            };
        };
    };
        
    sq_drained                    : 1; // RW S5, RW S0
    rsvd_state_flags              : 7;

    union {
        state_flags               : 8;
        struct {
            priv_oper_enable      : 1;  // RO
            in_progress           : 1;  // WO S5, RO S0
            bktrack_in_progress   : 1;  // RW S5, RW S0
            frpmr_in_progress     : 1;  // RW S0
            color                 : 1;  // WO S5, R0 S0
            fence                 : 1;  // WO S5, RO S0
            li_fence              : 1;  // WO S5, RO S0
            busy                  : 1;
        };
    };

    spec_enable                   : 1;
    skip_pt                       : 1;
    bktrack_marker_in_progress    : 1;
    // 0 - congestion_mgmt_disabled; 1 - DCQCN; 2 - ROME; 3 - RSVD
    congestion_mgmt_type          : 2;  // RO
    rsvd2                         : 3;

};

struct sqcb1_t {
    pc                             : 8;
    cq_id                          : 24; // RO S0
    rrq_pindex                     : 8;
    rrq_cindex                     : 8;
    rsvd1                          : 16;

    rrq_base_addr                  : 32; // RO S0
    log_rrq_size                   : 8;  // RO S0
    service                        : 4;  // RO S0
    rsvd8                          : 1;  // RO S0
    log_pmtu                       : 5;  // RO S0 
    err_retry_count                : 3;  // RO SO
    rnr_retry_count                : 3;  // RO S0
    work_not_done_recirc_cnt       : 8;  // RW S0

    tx_psn                         : 24; // R0 S0 (WO S5 TXDMA)
    ssn                            : 24; // R0 S0 (WO S5 TXDMA)
    rsvd2                          : 16; // rsvd for TXDMA memwr
    log_sqwqe_size                 :  5;
    pkt_spec_enable                :  1; // RO S0
    rsvd3                          :  2;

    header_template_addr           : 32; // RO SO // DCQCN ???
    header_template_size           : 8;  // RO SO

    nxt_to_go_token_id             : 8;  // RW S3
    token_id                       : 8;  // RW S0

    rsvd4                          : 24; // RW S0
    rexmit_psn                     : 24; // RW S0
    msn                            : 24; // RW S0

    credits                        : 5;  // RW S0 
    // 0 - congestion_mgmt_disabled; 1 - DCQCN; 2 - ROME; 3 - RSVD
    congestion_mgmt_type           : 2;  // RO S0
    rsvd5                          : 1;

    max_tx_psn                     : 24; // RW S0
    max_ssn                        : 24; // RW S0

    rrqwqe_num_sges                : 8;  // RW S3
    rrqwqe_cur_sge_id              : 8;  // RW S3
    rrqwqe_cur_sge_offset          : 32; // RW S3
    state                          : 3;  // RW S3
    sqcb1_priv_oper_enable         : 1;  // RO
    sq_drained                     : 1;  // RW S5
    sqd_async_notify_enable        : 1;  // RO S5
    rsvd6                          : 2;

    bktrack_in_progress            : 8; // RW S3 (W0 S5 TXDMA)
    pd                             : 32; // RO
    rrq_spec_cindex                : 16;
    rsvd7                          : 16;
};

struct sqcb2_t {
    dst_qp                         : 24; // RO S5
    header_template_size           : 8;  // RO S5
    header_template_addr           : 32; // RO S5
    rrq_base_addr                  : 32; // RO S5
    log_rrq_size                   : 5;  // RO S5
    log_sq_size                    : 5;  // RO S5
    roce_opt_ts_enable             : 1;  // RO S5
    roce_opt_mss_enable            : 1;  // RO S5
    service                        : 4;  // RO S5

    lsn_tx                         : 24; // RW S5
    lsn_rx                         : 24; // RO S1 (WO RXDMA)
    rexmit_psn                     : 24; // RO S1 (WO RXDMA)

    last_ack_or_req_ts             : 48; // RW S5 (WO RXDMA)

    err_retry_ctr                  : 4; // RW S1 (WO RXDMA)
    rnr_retry_ctr                  : 4; // RW S1 (WO RXDMA)
    rnr_timeout                    : 8; // RW S1 (WO RXDMA)
    rsvd1                          : 2; // TODO disable_credits
    timer_on                       : 1; // RW S5
    local_ack_timeout              : 5; // RO S5

    tx_psn                         : 24; // RW S5
    ssn                            : 24; // RW S5
    lsn                            : 24; // RW S5
    wqe_start_psn                  : 24; // RW S5

    union {
        imm_data                       : 32; // RW S5
        inv_key                        : 32; // RW S5
    };

    sq_cindex                      : 16; // RW S5
    rrq_pindex                     : 8;  // RW S5
    rrq_cindex                     : 8;  // RO S1, S5 (WO RXDMA)
    busy                           : 1; // RW S1, WO S3-7
    need_credits                   : 1; // RW S5, WO S3-4,S6-7
    rsvd2                          : 14;
    fence                          : 1;  // WO S5, RO S1
    li_fence                       : 1;  // WO S5, RO S1
    fence_done                     : 1;  // RW S1, WO S5
    curr_op_type                   : 5;  // RW S5

    exp_rsp_psn                    : 24; // RW S5
    //Temporary use for DOL - ROCE UDP options
    timestamp                      : 16;

    disable_credits                : 1; // TODO move it up
    rsvd3                          : 7;

    sq_msg_psn                     : 24; // RW S5
};

struct sqcb3_t {
    pad: 512;
};

// req_tx stats
struct sqcb4_t {
    num_bytes: 64;
    num_pkts: 32;
    num_send_msgs: 16;
    num_write_msgs: 16;
    num_read_req_msgs: 16;
    num_atomic_fna_msgs: 16;
    num_atomic_cswap_msgs: 16;
    num_send_msgs_inv_rkey: 16;
    num_send_msgs_imm_data: 16;
    num_write_msgs_imm_data: 16;
    num_pkts_in_cur_msg: 16;
    max_pkts_in_any_msg : 16;
    num_npg_req: 32;
    num_npg_bindmw_t1_req:16;
    num_npg_bindmw_t2_req:16;
    num_npg_frpmr_req:16;
    num_npg_local_inv_req:16;
    num_inline_req:16;
    num_timeout_local_ack:16;
    num_timeout_rnr:16;
    num_sq_drains:16;
    np_cnp_sent: 16;
    rp_num_byte_threshold_db: 16;
    qp_err_disabled                  :    1;
    qp_err_dis_flush_rq              :    1;
    qp_err_dis_ud_pmtu               :    1;
    qp_err_dis_ud_fast_reg           :    1;
    qp_err_dis_ud_priv               :    1;
    qp_err_dis_no_dma_cmds           :    1;
    qp_err_dis_lkey_inv_state        :    1;
    qp_err_dis_lkey_inv_pd           :    1;
    qp_err_dis_lkey_rsvd_lkey        :    1;
    qp_err_dis_lkey_access_violation :    1;
    qp_err_dis_bind_mw_len_exceeded          : 1;
    qp_err_dis_bind_mw_rkey_inv_pd           : 1;
    qp_err_dis_bind_mw_rkey_inv_zbva         : 1;
    qp_err_dis_bind_mw_rkey_inv_len          : 1;
    qp_err_dis_bind_mw_rkey_inv_mw_state     : 1;
    qp_err_dis_bind_mw_rkey_type_disallowed  : 1;
    qp_err_dis_bind_mw_lkey_state_valid      : 1;
    qp_err_dis_bind_mw_lkey_no_bind          : 1;
    qp_err_dis_bind_mw_lkey_zero_based       : 1;
    qp_err_dis_bind_mw_lkey_invalid_acc_ctrl : 1;
    qp_err_dis_bind_mw_lkey_invalid_va       : 1;
    qp_err_dis_bktrack_inv_num_sges          : 1;
    qp_err_dis_bktrack_inv_rexmit_psn        : 1;
    qp_err_dis_frpmr_fast_reg_not_enabled    : 1;
    qp_err_dis_frpmr_invalid_pd              : 1;
    qp_err_dis_frpmr_invalid_state           : 1;
    qp_err_dis_frpmr_invalid_len             : 1;
    qp_err_dis_frpmr_ukey_not_enabled        : 1;
    qp_err_dis_inv_lkey_qp_mismatch          : 1;
    qp_err_dis_inv_lkey_pd_mismatch          : 1;
    qp_err_dis_inv_lkey_invalid_state        : 1;
    qp_err_dis_inv_lkey_inv_not_allowed      : 1;
    qp_err_dis_table_error                   : 1;
    qp_err_dis_phv_intrinsic_error           : 1;
    qp_err_dis_table_resp_error              : 1;
    qp_err_dis_inv_optype                    : 1;
    qp_err_dis_err_retry_exceed              : 1;
    qp_err_dis_rnr_retry_exceed              : 1;
    qp_err_dis_rsvd                          : 26;
};

struct sqcb5_t {
    num_bytes: 64;
    num_pkts: 32;
    num_read_resp_pkts:32;
    num_read_resp_msgs:16;
    num_feedback:16;
    num_ack:16;
    num_atomic_ack:16;
    num_pkts_in_cur_msg: 16;
    max_pkts_in_any_msg : 16;
    qp_err_disabled                  :    1;
    qp_err_dis_rrqlkey_pd_mismatch   :    1;
    qp_err_dis_rrqlkey_inv_state     :    1;
    qp_err_dis_rrqlkey_rsvd_lkey     :    1;
    qp_err_dis_rrqlkey_acc_no_wr_perm:    1;
    qp_err_dis_rrqlkey_acc_len_lower :    1;
    qp_err_dis_rrqlkey_acc_len_higher:    1;
    qp_err_dis_rrqsge_insuff_sges    :    1;
    qp_err_dis_rrqsge_insuff_sge_len :    1;
    qp_err_dis_rrqsge_insuff_dma_cmds:    1;
    qp_err_dis_rrqwqe_remote_inv_req_err_rcvd :    1;
    qp_err_dis_rrqwqe_remote_acc_err_rcvd     :    1;
    qp_err_dis_rrqwqe_remote_oper_err_rcvd    :    1;
    qp_err_dis_table_error           :    1;
    qp_err_dis_phv_intrinsic_error   :    1;
    qp_err_dis_table_resp_error      :    1;
    qp_err_dis_rsvd                  :    16;
    //a packet which went thru too many recirculations in req_rx had to be terminated and qp had to 
    //be put into error disabled state. The recirc reason, opcode, the psn of the packet etc.
    //are remembered for further debugging.
    recirc_bth_psn: 24;
    recirc_bth_opcode: 8;
    recirc_reason: 4;
    max_recirc_cnt_err: 1;
    pad: 219;
};

struct sqcb_t {
    struct sqcb0_t sqcb0;
    struct sqcb1_t sqcb1;
    struct sqcb2_t sqcb2;
    struct sqcb3_t sqcb3;
    struct sqcb4_t sqcb4;
    struct sqcb5_t sqcb5;
};
#endif //__SQCB_H
