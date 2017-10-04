#ifndef __RQCB_H
#define __RQCB_H
#include "capri.h"

#define MAX_RQ_RINGS	6
#define RQ_RING_ID 	0
#define RSQ_RING_ID 	1
#define ACK_NAK_RING_ID 2
#define RSQ_BT_RING_ID 	3

#define RQ_P_INDEX d.{ring0.pindex}.hx
#define RQ_C_INDEX d.{ring0.cindex}.hx

#define RSQ_P_INDEX d.{ring1.pindex}.hx
#define RSQ_C_INDEX d.{ring1.cindex}.hx

#define ACK_NAK_P_INDEX d.{ring2.pindex}.hx
#define ACK_NAK_C_INDEX d.{ring2.cindex}.hx

#define PROXY_RQ_C_INDEX   d.{proxy_cindex}.hx

struct rqcb0_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;
    struct capri_intrinsic_ring_t ring1;
    struct capri_intrinsic_ring_t ring2;
    struct capri_intrinsic_ring_t ring3;
    struct capri_intrinsic_ring_t ring4;
    struct capri_intrinsic_ring_t ring5;

    pt_base_addr: 32;

    rsq_base_addr: 32;

    rsvd0: 3;
    log_rsq_size: 5;
    
    token_id: 8;
    nxt_to_go_token_id: 8;
    rsq_pindex_prime: 8;


    log_pmtu: 5;
    log_rq_page_size: 5;
    log_wqe_size: 5;
    log_num_wqes: 5;
    serv_type: 3;
    srq_enabled: 1;
    busy: 1;
    in_progress: 1;
    rsvd1: 3; 
    adjust_rsq_c_index_in_progress: 1;
    rsq_quiesce: 1;
    cache: 1;


    e_psn: 24;
    adjust_rsq_c_index: 8;

    msn:24; 
    rsvd2:8; 

    pd: 32; 

    proxy_cindex: 16; // place holder for a copy of c_index to avoid
                       // scheduler ringing RQ all the time.
    rsvd3: 16;
};

struct rqcb1_t {
    va: 64;
    len: 32;
    r_key: 32;
    wrid: 64;
    cq_id: 24;
    read_rsp_in_progress: 1;
    read_rsp_lock: 1;
    dummy1: 6;
    curr_read_rsp_psn: 24;
    ack_nak_psn: 24;
    struct rdma_aeth_t aeth;
    last_ack_nak_psn: 24;
    header_template_addr: 32;
    dst_qp: 24;
    curr_wqe_ptr: 64;
    current_sge_id: 8;
    num_sges: 8;
    current_sge_offset: 32;
    p4plus_to_p4_flags: 8;
    rsvd: 8;
};

struct rqcb_t {
    struct rqcb0_t rqcb0;
    struct rqcb1_t rqcb1;
};
#endif // __RQCB_H
