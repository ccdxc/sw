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
    color: 1;
    rsvd2: 6;
    rsvd3: 24;

    rsvd4: 32;
    pad: 256;  // as d-vector is loaded only 256 bits, need this to interpret it correctly
};

#endif // __CQCB_H
