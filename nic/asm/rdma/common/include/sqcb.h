#ifndef __SQCB_H
#define __SQCB_H
#include "capri.h"

#define MAX_SQ_RINGS	6
#define SQ_RING_ID 	0
#define FC_RING_ID 	1
#define RRQ_RING_ID	(MAX_SQ_RINGS - 1)	

#define SQ_P_INDEX  d.{ring0.pindex}.hx
#define SQ_C_INDEX  d.{ring0.cindex}.hx
#define RRQ_P_INDEX d.ring5.pindex
#define RRQ_C_INDEX d.ring5.cindex

#define SQCB_T struct sqcb_t
#define SQCB0_T struct sqcb0_t
#define SQCB1_T struct sqcb1_t

struct sqcb0_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;
    struct capri_intrinsic_ring_t ring1;
    struct capri_intrinsic_ring_t ring2;
    struct capri_intrinsic_ring_t ring3;
    struct capri_intrinsic_ring_t ring4;
    struct capri_intrinsic_ring_t ring5;
    pt_base_addr: 32;
    log_pmtu: 5;
    log_sq_page_size: 5;
    log_wqe_size: 5;
    log_num_wqes: 5;
    service: 4;
    curr_op_type: 8;
    curr_wqe_ptr: 64;
    current_sge_offset: 32;
    current_sge_id: 8;
    num_sges: 8;
    rsvd: 7;
    busy: 1;
    in_progress: 1;
    signalled_completion: 1;
    disable_e2e_fc: 1;
    fast_reg_enable: 1;
    fence: 1;
    li_fence: 1;
    need_credits: 1;
    bktrack_in_progress: 1;
    pd: 32;
    rsvd1: 32;
};

struct sqcb1_t {
    rrq_base_addr                  : 32;
    imm_data                       : 32;
    inv_key                        : 32;
    rrq_size                       : 8;
    cq_id                          : 24;
    dst_qp                         : 24;
    tx_psn                         : 24;
    ssn                            : 24;
    lsn                            : 24;
    msn                            : 24;
    credits                        : 5;
    in_progress                    : 1;
    service                        : 4;
    timer_active                   : 1;
    local_ack_timeout              : 5;
    rrqwqe_num_sges                : 8;
    rrqwqe_cur_sge_id              : 8;
    rrqwqe_cur_sge_offset          : 32;
    header_template_addr           : 32;
    nxt_to_go_token_id             : 8;
    token_id                       : 8;
    e_rsp_psn                      : 24;
    rexmit_psn                     : 24;
    wqe_start_psn                  : 24;
    nak_retry_ctr                  : 3;
    err_retry_ctr                  : 3;
    rsvd1                          : 2;
    p4plus_to_p4_flags             : 8;
    rvsd2                          : 64;
};

struct sqcb_t {
    struct sqcb0_t sqcb0;
    struct sqcb1_t sqcb1;
};
#endif //__SQCB_H
