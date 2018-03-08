#ifndef __CQCB_H
#define __CQCB_H
#include "capri.h"

#define MAX_CQ_RINGS	1
#define CQ_RING_ID 	0

#define CQ_P_INDEX  d.ring0.pindex
#define CQ_P_INDEX_HX  d.{ring0.pindex}.hx
#define CQ_C_INDEX  d.ring0.cindex

#define CQ_COLOR    d.color

struct cqcb_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;
    pt_base_addr: 32;

    log_cq_page_size: 5;
    log_wqe_size: 5;
    log_num_wqes: 5;
    rsvd1: 1;

    cq_id: 24;
    eq_id: 24;

    arm: 1;
    wakeup_dpath:1;
    color: 1;

    wakeup_lif:12;
    wakeup_qtype:3;
    wakeup_qid:24;
    wakeup_ring_id:3;
    rsvd4: 19;

    pt_pa: 64;
    pt_next_pa: 64;
    pt_pa_index: 16;
    pt_next_pa_index: 16;
    pad: 96;  // as d-vector is loaded only 256 bits, need this to interpret it correctly
};

#endif // __CQCB_H
