#ifndef __RQCB_H
#define __RQCB_H
#include "capri.h"

#define MAX_RQ_RINGS                6

#define RQ_RING_ID                  0
#define RSQ_RING_ID                 1
#define ACK_NAK_RING_ID             2
#define BT_RING_ID                  3
#define DCQCN_RATE_COMPUTE_RING_ID  4
#define DCQCN_TIMER_RING_ID         5

#define RQ_PRI                      3
#define RSQ_PRI                     4
#define ACK_NAK_PRI                 5
#define BT_PRI                      2
#define DCQCN_RATE_COMPUTE_PRI      0
#define DCQCN_TIMER_PRI             1

#define RQ_P_INDEX d.{ring0.pindex}.hx
#define RQ_C_INDEX d.{ring0.cindex}.hx

#define RSQ_P_INDEX d.{ring1.pindex}.hx
#define RSQ_C_INDEX d.{ring1.cindex}.hx

#define ACK_NAK_P_INDEX d.{ring2.pindex}.hx
#define ACK_NAK_C_INDEX d.{ring2.cindex}.hx

#define BT_P_INDEX d.{ring3.pindex}.hx
#define BT_C_INDEX d.{ring3.cindex}.hx

#define DCQCN_RATE_COMPUTE_P_INDEX d.{ring4.pindex}.hx
#define DCQCN_RATE_COMPUTE_C_INDEX d.{ring4.cindex}.hx

#define DCQCN_TIMER_P_INDEX d.{ring5.pindex}.hx
#define DCQCN_TIMER_C_INDEX d.{ring5.cindex}.hx

#define PROXY_RQ_C_INDEX   d.{proxy_cindex}.hx
#define PROXY_RQ_P_INDEX   d.{proxy_pindex}.hx

#define SPEC_RQ_C_INDEX d.{spec_cindex}.hx

#define RSQ_C_INDEX_OFFSET          FIELD_OFFSET(rqcb0_t, ring1.cindex)
#define RQCB0_CURR_READ_RSP_PSN     FIELD_OFFSET(rqcb0_t, curr_read_rsp_psn)

// Tx only cb
struct rqcb0_t {
    struct capri_intrinsic_qstate_t intrinsic;
    struct capri_intrinsic_ring_t ring0;
    struct capri_intrinsic_ring_t ring1;
    struct capri_intrinsic_ring_t ring2;
    struct capri_intrinsic_ring_t ring3;
    struct capri_intrinsic_ring_t ring4;
    struct capri_intrinsic_ring_t ring5;

    // need for prefetch
    union {
        pt_base_addr: 32;       //Ronly
        hbm_rq_base_addr: 32;   //Ronly
    };

    log_rq_page_size: 5;        //Ronly
    log_wqe_size: 5;            //Ronly
    log_num_wqes: 5;            //Ronly
    congestion_mgmt_enable:1;   //Ronly
    state: 3;                   //Ronly?
    log_rsq_size: 5;            //Ronly
    serv_type: 3;               //Ronly
    log_pmtu: 5;                //Ronly
    
    union {
        rsq_base_addr: 32;      //Ronly
        q_key : 32;             //Ronly
    };

    pd: 32;                     //Ronly

    header_template_addr: 32;   //Ronly

    dst_qp: 24;                 //Ronly
    read_rsp_lock: 1;           //Rw by S0 and S4 ?
    read_rsp_in_progress: 1;    //Rw by S0 and S4 ?
    rq_in_hbm: 1;               //Ronly
    bt_lock: 1;                 //Rw by S0 and Sx
    bt_in_progress: 1;          //RW by S0 and Sx
    rsvd0: 3;

    curr_read_rsp_psn: 24;      //Rw by S0 ?

    header_template_size: 8;    //Ronly
    ring_empty_sched_eval_done: 1;  //rw in S0
    rsvd1: 7;

    // store the current backtrack progress
    bt_rsq_cindex: 16;          //Read by S0, write by Sx

    pad: 8;   // 1B
};

//Rx only cb
struct rqcb1_t {
    pc: 8;
    union {
        pt_base_addr: 32;       //Ronly
        hbm_rq_base_addr: 32;   //Ronly
    };

    log_rq_page_size: 5;    //Ronly
    log_wqe_size: 5;        //Ronly
    log_num_wqes: 5;        //Ronly
    congestion_mgmt_enable:1;   //Ronly
    state: 3;               //Ronly ?
    log_rsq_size: 5;        //Ronly     
    serv_type: 3;           //Ronly
    log_pmtu: 5;            //Ronly
    
    union {
        rsq_base_addr: 32;  //Ronly
        q_key: 32;          //Ronly
    };

    pd: 32;                 //Ronly

    header_template_addr: 32;   //Ronly

    token_id: 8;            //rw by S0
    nxt_to_go_token_id: 8;  // written by S4, read by S0
    rsvd4 : 8;
    srq_enabled: 1;         //Ronly
    cache: 1;               //Ronly
    immdt_as_dbell: 1;      //Ronly
    rq_in_hbm: 1;           //Ronly
    rsvd0: 4;

    disable_speculation: 1; //rw by S0
    rsvd1: 7;
    in_progress: 1;         // wirtten by S4, read by S0
    rsvd2: 7;
    spec_cindex: 16;  // cindex used for speculation
                      // rw by S0
                      
    e_psn: 24;        //rw by S0
    rsvd3: 8;

    msn:24;                 //rw by S0 ?
    header_template_size: 8;    //Ronly

    // place holder for a copy of c_index/p_index to avoid
    // scheduler ringing RQ all the time.
    proxy_cindex: 16;   // written by S4, read by S0
    proxy_pindex: 16;   // written by TxDMA, read by RxDMA
                       

    cq_id: 24;                  //Ronly
    bt_in_progress: 8;  // set to 1 by rxdma, to 0 by txdma

    rsq_pindex: 8;  // written by S0

    // multi-packet send variables
    // written by S4, read by S0/S1 
    curr_wqe_ptr: 64;
    current_sge_offset: 32;
    current_sge_id: 8;
    num_sges: 8;

    srq_id: 24;
    pad: 8;  //1B
};

// rx -> tx cb. i.e., written by rxdma, read by txdma
struct rqcb2_t {
    rsvd0: 8;                    //1B
    ack_nak_psn: 24;            //3B
    struct rdma_aeth_t aeth;    //4B

    //backtrack info
    struct resp_bt_info_t  bt_info;

    // rnr
    rnr_timeout: 5;
    rsvd1: 3;

    pad: 280;   //35B
};

// Multi-packet write fields used in resp_rx
struct rqcb3_t {
    wrid: 64;
    va: 64;         
    len: 32;
    r_key: 32;

    //Temporarily storing these params here for DOL testing purpose
    roce_opt_ts_value: 32;
    roce_opt_ts_echo:  32;
    roce_opt_mss:      16;
    rsvd1:             16;

    pad: 224; //28B
};

//resp_tx stats
struct rqcb4_t {
    //TBD
    pad: 512;
};

// resp_rx stats
struct rqcb5_t {
    num_bytes: 64;
    num_pkts: 32;
    num_send_msgs: 16;
    num_write_msgs: 16;
    num_read_req_msgs: 16;
    num_atomic_fna_msgs: 16;
    num_atomic_cswap_msgs: 16;
    num_send_msgs_inv_rkey: 16;
    num_send_msgs_imm_data: 16;
    num_write_msgs_imm_data: 16;
    num_ack_requested: 16;
    num_ring_dbell: 16;
    num_pkts_in_cur_msg: 16;
    max_pkts_in_any_msg : 16;
    pad: 224;
};

struct rqcb_t {
    struct rqcb0_t rqcb0;
    struct rqcb1_t rqcb1;
    struct rqcb2_t rqcb2;
    struct rqcb3_t rqcb3;
    struct rqcb4_t rqcb4;
    struct rqcb5_t rqcb5;
};

#endif // __RQCB_H
