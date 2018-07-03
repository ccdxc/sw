#ifndef __AQCB_H__
#define __AQCB_H__

#include "capri.h"

#define MAX_AQ_RINGS          1
#define MAX_AQ_DOORBELL_RINGS 1
#define AQ_RING_ID            0

#define AQ_P_INDEX  d.ring0.pindex
#define AQ_P_INDEX_HX  d.{ring0.pindex}.hx
#define AQ_C_INDEX  d.ring0.cindex
#define AQ_C_INDEX_HX  d.{ring0.cindex}.hx

#define AQ_COLOR    d.color

struct aqcb_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;

    proxy_pindex: 16;

    log_aq_page_size: 5;
    log_wqe_size: 5;
    log_num_wqes: 5;
    rsvd2: 1;

    phy_base_addr: 64;
    
    aq_id: 24;
    rsvd1: 8;

    pad: 256;
};

#endif //__AQCB_H__
