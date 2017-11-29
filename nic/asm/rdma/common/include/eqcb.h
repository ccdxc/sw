#ifndef __EQCB_H
#define __EQCB_H
#include "capri.h"

#define MAX_EQ_RINGS	1
#define EQ_RING_ID 	0

#define EQ_P_INDEX  d.ring0.pindex
#define EQ_C_INDEX  d.ring0.cindex

#define EQ_COLOR    d.color

struct eqcb_t {    // 32 bytes
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;
    eqe_base_addr: 64;
    int_num: 32;
    eq_id: 24;
    log_num_wqes: 5;
    log_wqe_size: 5;
    int_enabled: 1;
    color: 1;
    rsvd: 28;
    pad: 256;  // as d-vector is loaded only 256 bits, need this to interpret it correctly
};

#endif // __EQCB_H
