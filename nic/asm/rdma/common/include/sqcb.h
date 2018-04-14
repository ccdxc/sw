#ifndef __SQCB_H
#define __SQCB_H
#include "capri.h"

#define MAX_SQ_RINGS            6
#define MAX_SQ_HOST_RINGS (MAX_SQ_RINGS - 1)

#define SQ_RING_ID              0
#define FC_RING_ID              1
#define SQ_BKTRACK_RING_ID      2
#define TIMER_RING_ID           3
#define CNP_RING_ID             4
#define RRQ_RING_ID	        (MAX_SQ_RINGS - 1)

#define CNP_PRI                 0
#define FC_PRI                  1 
#define SQ_BKTRACK_PRI          2
#define TIMER_PRI               3
#define SQ_PRI                  4

#define SQ_RING_ID_BITMAP            0x01 // (1 << SQ_RING_ID)
#define FC_RING_ID_BITMAP            0x02 // (1 << FC_RING_ID)
#define SQ_BKTRACK_RING_ID_BITMAP    0x04 // (1 << SQ_BACKTRACK_RING_ID)
#define TIMER_RING_ID_BITMAP         0x08 // (1 << TIMER_RING_ID)

#define SQ_P_INDEX                   d.{ring0.pindex}.hx
#define SQ_C_INDEX                   d.{ring0.cindex}.hx
#define FC_P_INDEX                   d.{ring1.pindex}.hx
#define FC_C_INDEX                   d.{ring1.cindex}.hx
#define SQ_BKTRACK_P_INDEX           d.{ring2.pindex}.hx
#define SQ_BKTRACK_C_INDEX           d.{ring2.cindex}.hx
#define SQ_TIMER_P_INDEX             d.{ring3.pindex}.hx
#define SQ_TIMER_C_INDEX             d.{ring3.cindex}.hx
#define SQCB0_RRQ_P_INDEX            d.{rrq_pindex}.hx
#define RRQ_P_INDEX                  d.{ring5.pindex}
#define RRQ_C_INDEX                  d.{ring5.cindex}
#define CNP_P_INDEX                  d.{ring4.pindex}.hx
#define CNP_C_INDEX                  d.{ring4.cindex}.hx

#define SPEC_SQ_C_INDEX              d.spec_sq_cindex

#define RRQ_P_INDEX_OFFSET           FIELD_OFFSET(sqcb1_t, ring5.pindex)
#define RRQ_C_INDEX_OFFSET           FIELD_OFFSET(sqcb1_t, ring5.cindex)
#define SQCB2_MSN_OFFSET             FIELD_OFFSET(sqcb2_t, msn)
#define SQCB2_REXMIT_PSN_OFFSET      FIELD_OFFSET(sqcb2_t, rexmit_psn)
#define SQCB2_MSN_CREDITS_BYTES      4

#define SQCB_T struct sqcb_t
#define SQCB0_T struct sqcb0_t
#define SQCB1_T struct sqcb1_t

#define SIZEOF_TOKEN_ID_BITS  8

#define SQCB0_NEED_CREDITS_FLAG 0x40
#define SQCB0_CB1_BUSY_FLAG 0x80

#define SQCB_SQ_PINDEX_OFFSET        FIELD_OFFSET(sqcb0_t, ring0.pindex)
#define SQCB_SQ_CINDEX_OFFSET        FIELD_OFFSET(sqcb0_t, ring0.cindex)
#define SQCB_SPEC_SQ_CINDEX_OFFSET   FIELD_OFFSET(sqcb0_t, spec_sq_cindex)
#define SQCB_CURR_OPTYPE_OFFSET      FIELD_OFFSET(sqcb0_t, curr_op_type)
#define SQCB_CURR_WQE_PTR_OFFSET     FIELD_OFFSET(sqcb0_t, curr_wqe_ptr)
#define SQCB_CURRENT_SGE_OFFSET      FIELD_OFFSET(sqcb0_t, current_sge_offset)

#define SQCB0_IN_PROGRESS_BIT_OFFSET        6
#define SQCB0_NEED_CREDITS_BIT_OFFSET       5

struct sqcb0_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;
    struct capri_intrinsic_ring_t ring1;
    struct capri_intrinsic_ring_t ring2;
    struct capri_intrinsic_ring_t ring3;
    struct capri_intrinsic_ring_t ring4;

    union {
        pt_base_addr              : 32; // RO
        hbm_sq_base_addr          : 32; // RO
    };
    header_template_addr          : 32; // RO
    pd                            : 32; // RO
    log_pmtu                      : 5;  // RO
    log_sq_page_size              : 5;  // RO
    log_wqe_size                  : 5;  // RO
    log_num_wqes                  : 5;  // RO
    service                       : 4;  // RO

    rsvd_cfg_flags                : 4;
    poll_for_work                 : 1;  // RO
    signalled_completion          : 1;  // RO
    disable_e2e_fc                : 1;  // RO
    fast_reg_enable               : 1;  // RO
    sq_in_hbm                     : 1;  // RO
    congestion_mgmt_enable        : 1;  // RO
    local_ack_timeout             : 5;  // RO
    ring_empty_sched_eval_done    : 1;  // RW S0

    spec_sq_cindex                : 16; // RW S0

    curr_wqe_ptr                  : 64; // WO S5, RO S0
    union {
        current_sge_offset        : 32; // WO S5, RO S0
        read_req_adjust           : 32; // RO S0
    };
    current_sge_id                : 8;  // WO S5, RO S0
    num_sges                      : 8;  // WO S5, RO S0

    rsvd_state_flags              : 5;
    dcqcn_rl_failure              : 1;  // RW S0, RW S5
    bktrack_in_progress           : 1;  // RW S5, RW S0
    retry_timer_on                : 1;  // RW S0

    poll_in_progress              : 1;  // WO S5, RW S0
    color                         : 1;  // WO S5, R0 S0
    fence                         : 1;  // WO S5, RW S0
    li_fence                      : 1;  // WO S5, RW S0
    state                         : 3;
    busy                          : 1;

    union {
        struct {
            cb1_busy              : 1;  // WO S5, R0 S0
            in_progress           : 1;  // WO S5, RO S0
            need_credits          : 1;  // WO S5, RO S0
            rsvd_cb1_flags        : 5;
        };
        cb1_byte                  : 8;
    };
};

struct sqcb1_t {
    pc                             : 8;
    cq_id                          : 24; // RO S0
    struct capri_intrinsic_ring_t  ring5; // RRQ Ring

    rrq_base_addr                  : 32; // RO S0
    log_rrq_size                   : 8;  // RO S0
    service                        : 4;  // RO S0
    congestion_mgmt_enable         : 1;  // RO S0
    log_pmtu                       : 5;  // RO S0 
    rsvd                           : 14;

    tx_psn                         : 24; // R0 S0 (WO S5 TXDMA)
    ssn                            : 24; // R0 S0 (WO S5 TXDMA)
    lsn                            : 24; // R0 S0 (WO S5 TXDMA)

    header_template_addr           : 32; // RO SO // DCQCN ???
    header_template_size           : 8;  // RO SO

    nxt_to_go_token_id             : 8;  // RW 
    token_id                       : 8;  // RW S0

    e_rsp_psn                      : 24; // RW S0
    rexmit_psn                     : 24; // RW S0
    msn                            : 24; // RW S0

    credits                        : 5;  // RW S0 
    rnr_retry_ctr                  : 3;  // RW S0
    err_retry_ctr                  : 3;  // RW S0
    rsvd1                          : 5;

    max_tx_psn                     : 24; // RW S0
    max_ssn                        : 24; // RW S0

    rrqwqe_num_sges                : 8;  // RW S5
    rrqwqe_cur_sge_id              : 8;  // RW S5
    rrqwqe_cur_sge_offset          : 32; // RW S5
    rrq_in_progress                : 1;  // RW S5
    rsvd2                          : 7;

    pad                            : 64;
};

struct sqcb2_t {
    dst_qp                         : 24; // RO S5
    header_template_size           : 8;  // RO S5
    union {
        header_template_addr       : 32; // RO S5
        q_key                      : 32; // privileged q_key
    };
    rrq_base_addr                  : 32; // RO S5
    log_rrq_size                   : 5;  // RO S5
    log_sq_size                    : 5;  // RO S5
    roce_opt_ts_enable             : 1;  // RO S5
    roce_opt_mss_enable            : 1;  // RO S5
    service                        : 4;  // RO S5

    msn                            : 24; // RO S1 (WO RXDMA)
    credits                        : 8;  // RO S1 (WO RXDMA)
    rexmit_psn                     : 24; // RO S1 (WO RXDMA)

    last_ack_or_req_ts             : 48; // RW S5 (WO RXDMA)

    in_progress                    : 1; // RW S5
    need_credits                   : 1; // RW S5
    timer_on                       : 1; // RW S5
    local_ack_timeout              : 5; // RO S5

    tx_psn                         : 24; // RW S5
    ssn                            : 24; // RW S5
    lsn                            : 24; // RW S5
    wqe_start_psn                  : 24; // RW S5
    curr_op_type                   : 8;  // RW S5

    imm_data                       : 32; // RW S5
    inv_key                        : 32; // RW S5

    sq_cindex                      : 16; // RW S5
    rrq_pindex                     : 16; // RW S5

    p4plus_to_p4_flags             : 8; // Not needed ???
    exp_rsp_psn                    : 24; // RW S5

    //Temporary use for DOL - ROCE UDP options
    timestamp                      : 16;
    timestamp_echo                 : 16;
    mss                            : 16;
   
    pad                            : 8;
};

struct sqcb_t {
    struct sqcb0_t sqcb0;
    struct sqcb1_t sqcb1;
    struct sqcb2_t sqcb2;
};
#endif //__SQCB_H
