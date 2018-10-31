#ifndef __AQCB_H__
#define __AQCB_H__

#include "capri.h"

#define MAX_AQ_RINGS          1
#define MAX_AQ_DOORBELL_RINGS 1
#define AQ_RING_ID            0

#define AQ_P_INDEX     d.ring0.pindex
#define AQ_P_INDEX_HX  d.{ring0.pindex}.hx
#define AQ_C_INDEX     d.ring0.cindex
#define AQ_C_INDEX_HX  d.{ring0.cindex}.hx

#define PROXY_AQ_P_INDEX     d.{proxy_pindex}
#define PROXY_AQ_P_INDEX_HX  d.{proxy_pindex}.hx

#define CQ_PROXY_PINDEX      d.{proxy_pindex}.hx
#define CQ_P_INDEX           d.{ring0.pindex}.hx
#define CQ_C_INDEX           d.{ring0.cindex}.hx

#define AQ_COLOR             d.color

struct aqcb0_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;

    proxy_pindex: 16;

    log_wqe_size: 5;
    log_num_wqes: 5;
    ring_empty_sched_eval_done: 1;
    rsvd2: 5;

    phy_base_addr: 64;
    
    aq_id: 24;
    busy: 8;

    cq_id: 24;
    rsvd3:8;

    cqcb_addr: 64;
    pad: 192;
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
    
    num_any: 64;

    pad: 240;
};

struct aqcb_t {
    struct aqcb0_t aqcb0;
    struct aqcb1_t aqcb1;
};


#endif //__AQCB_H__
