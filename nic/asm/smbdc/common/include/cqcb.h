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

    cq_base_addr: 64;
    log_cq_page_size: 5;
    log_wqe_size: 5;
    log_num_wqes: 5;

    color: 1;

    pad: 80;
    rsvd: 256; //for d-vector alignment
};

#endif // __CQCB_H
