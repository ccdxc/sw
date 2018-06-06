#ifndef __CQCB_H
#define __CQCB_H
#include "capri.h"

#define MAX_CQ_RINGS	3
#define MAX_CQ_DOORBELL_RINGS	3
#define CQ_RING_ID 	0
#define CQ_ARM_RING_ID 1
#define CQ_SARM_RING_ID 2

#define CQ_PRI 0 //highest prio
#define CQ_ARM_PRI 1 
#define CQ_SARM_PRI 2

#define CQ_P_INDEX  d.ring0.pindex
#define CQ_P_INDEX_HX  d.{ring0.pindex}.hx
#define CQ_C_INDEX  d.ring0.cindex
#define CQ_C_INDEX_HX  d.{ring0.cindex}.hx

#define CQ_COLOR    d.color

#define CQ_ARM_C_INDEX d.{ring1.cindex}
#define CQ_ARM_P_INDEX d.{ring1.pindex}
#define CQ_ARM_C_INDEX_HX d.{ring1.cindex}.hx
#define CQ_ARM_P_INDEX_HX d.{ring1.pindex}.hx

#define CQ_SARM_C_INDEX d.{ring2.cindex}
#define CQ_SARM_P_INDEX d.{ring2.pindex}
#define CQ_SARM_C_INDEX_HX d.{ring2.cindex}.hx
#define CQ_SARM_P_INDEX_HX d.{ring2.pindex}.hx

struct cqcb_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;
    struct capri_intrinsic_ring_t ring1;
    struct capri_intrinsic_ring_t ring2;

    proxy_pindex: 16;
    //recent solicited CQE index
    proxy_s_pindex: 16;

    pt_base_addr: 32;

    log_cq_page_size: 5;
    log_wqe_size: 5;
    log_num_wqes: 5;
    ring_empty_sched_eval_done : 1;

    cq_id: 24;
    eq_id: 24;

    arm: 1;
    sarm: 1;
    wakeup_dpath:1;
    color: 1;

    wakeup_lif:12;
    wakeup_qtype:3;
    wakeup_qid:24;
    wakeup_ring_id:3;
    rsvd4: 2;

    pt_pg_index: 16;
    pt_next_pg_index: 16;
    pad: 16;

    pt_pa: 64;
    pt_next_pa: 64;
};

#endif // __CQCB_H
