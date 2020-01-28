#ifndef __AQCB_H__
#define __AQCB_H__

#include "capri.h"

#define MAX_AQ_RINGS          1
#define MAX_AQ_DOORBELL_RINGS 1
#define AQ_RING_ID            0

#define AQ_P_INDEX     d.{ring0.pindex}.hx
#define AQ_C_INDEX     d.{ring0.cindex}.hx
#define AQ_PROXY_C_INDEX     d.{proxy_cindex}.hx

#define CQ_PROXY_PINDEX      d.{proxy_pindex}.hx
#define CQ_P_INDEX           d.{ring0.pindex}.hx
#define CQ_C_INDEX           d.{ring0.cindex}.hx

#define AQ_COLOR             d.color

struct aqcb0_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;

    map_count_completed: 32;
    first_pass: 8;
    token_id: 8;

    rsvd1: 16;
    uplink_num: 8;
    debug: 8;

    log_wqe_size: 5;
    log_num_wqes: 5;
    ring_empty_sched_eval_done: 1;
    rsvd2: 5;

    phy_base_addr: 64;

    next_token_id: 8;
    aq_id: 24;

    cq_id: 24;
    error: 8;

    cqcb_addr: 64;

    pad: 128;
};


struct aqcb1_t {
    num_nop: 16;
    num_create_cq: 16;
    num_create_qp: 16;
    num_reg_mr: 16;
    num_stats_hdrs: 16;
    num_stats_vals: 16;
    num_dereg_mr: 16;
    num_resize_cq: 16;
    num_destroy_cq: 16;
    num_modify_qp: 16;
    num_query_qp: 16;
    num_destroy_qp: 16;
    num_stats_dump: 16;
    num_create_ah: 16;
    num_query_ah: 16;
    num_destroy_ah: 16;

    num_any: 64;

    pad: 192;
};

struct aqcb_t {
    struct aqcb0_t aqcb0;
    struct aqcb1_t aqcb1;
};


#endif //__AQCB_H__
