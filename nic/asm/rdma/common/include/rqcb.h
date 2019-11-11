#ifndef __RQCB_H
#define __RQCB_H
#include "capri.h"

#define MAX_RQ_RINGS                6
#define MAX_RQ_HOST_RINGS           1

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

#define ACK_NAK_PROXY_P_INDEX   d.{ack_nak_pindex}.hx
#define PROXY_RQ_C_INDEX   d.{proxy_cindex}.hx
#define PROXY_RQ_P_INDEX   d.{proxy_pindex}.hx

#define SPEC_RQ_C_INDEX d.{spec_cindex}.hx

// rqcb2 fields related to prefetch
#define PREFETCH_P_INDEX d.{prefetch_pindex}.hx
#define PREFETCH_PROXY_C_INDEX d.{prefetch_proxy_cindex}.hx

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

    log_rq_page_size: 5;        //Ronly
    log_wqe_size: 5;            //Ronly
    log_num_wqes: 5;            //Ronly
    rsvd2: 1;
    state: 3;                   //Ronly?
    log_rsq_size: 5;            //Ronly

    // need for prefetch
    union {
        struct {
            pad1 : 8;
            pt_base_addr: 32;       //Ronly
        };
        struct {
            pad2 : 8;
            hbm_rq_base_addr: 32;   //Ronly
        };
        phy_base_addr             : 40;            
    };

    union {
        rsq_base_addr: 32;      //Ronly
        q_key : 32;             //Ronly
    };

    spec_read_rsp_psn: 24;      //Written by S0, Read by S5
    spec_color: 1;              //Rw by S0
    drain_in_progress: 1;       //Rw by S0
    skip_pt: 1;
    prefetch_en: 1;             //Rw by S0
    // 0 - congestion_mgmt_disabled; 1 - DCQCN; 2 - ROME; 3 - RSVD
    congestion_mgmt_type:2;   //Ronly
    rsvd: 2;

    header_template_addr: 32;   //Ronly

    dst_qp: 24;                 //Ronly
    curr_color: 1;              //Written by S5, Read by S0
    read_rsp_in_progress: 1;    //Rw by S0 and S4 ?
    rq_in_hbm: 1;               //Ronly
    bt_lock: 1;                 //Rw by S0 and Sx
    bt_in_progress: 1;          //RW by S0 and Sx
    drain_done: 1;              //Written by S5, Read by S0
    rsvd0: 2;

    curr_read_rsp_psn: 24;      //Written by S5, Read by S0 and S5

    header_template_size: 8;    //Ronly
    ring_empty_sched_eval_done: 1;  //rw in S0
    dcqcn_cfg_id: 4;            //Ronly
    rsvd1: 3;

    // store the current backtrack progress
    bt_rsq_cindex: 16;          //Read by S0, write by Sx

    serv_type: 3;               //Ronly
    log_pmtu: 5;                //Ronly
    
};

//Rx only cb
struct rqcb1_t {
    pc: 8;

    union {
        rsq_base_addr: 32;  //Ronly
        q_key: 32;          //Ronly
    };

    union {
        struct {
           pad1 : 8;
           pt_base_addr: 32;       //Ronly
        };
        struct {
           pad2 : 8;
           hbm_rq_base_addr: 32;   //Ronly
        };
        struct {
           pad3 : 8;
           prefetch_base_addr: 32; //Ronly
        };
        phy_base_addr             : 40;            
    };

    log_rq_page_size: 5;    //Ronly
    log_wqe_size: 5;        //Ronly
    log_num_wqes: 5;        //Ronly
    rsvd4: 1;   //Ronly
    state: 3;               //Ronly ?
    log_rsq_size: 5;        //Ronly     
    
    srq_enabled: 1;         //Ronly
    cache: 1;               //Ronly
    immdt_as_dbell: 1;      //Ronly
    rq_in_hbm: 1;           //Ronly
    nak_prune: 1;           //rw by S0
    priv_oper_enable: 1;    //Ronly
    skip_pt :1;
    prefetch_en: 1;         //Ronly

    cq_id: 24;              //Ronly

    pd: 32;                 //Ronly

    header_template_addr: 32;   //Ronly

    token_id: 8;            //rw by S0
    nxt_to_go_token_id: 8;  // written by S4, read by S0
    work_not_done_recirc_cnt: 8; //rw by S0

    busy: 1; // set to 1 by S0, to 0 by S3
    // 0 - congestion_mgmt_disabled; 1 - DCQCN; 2 - ROME; 3 - RSVD
    congestion_mgmt_type: 2;
    rsvd1: 5;
    in_progress: 1;         // written by S4, read by S0
    rsvd2: 7;
    spec_cindex: 16;  // cindex used for speculation
                      // rw by S0

    e_psn: 24;        //rw by S0
    next_op_type: 2;  //rw by S0
    next_pkt_type: 1; //rw by S0
    spec_en: 1; //rw by S0
    access_flags: 3; // rw by S0
    rsvd3: 1;

    msn:24;                 //rw by S0 ?
    header_template_size: 8;    //Ronly

    bt_in_progress: 8;  // set to 1 by rxdma, to 0 by txdma

    rsq_pindex: 8;  // written by S0

    // multi-packet send variables
    // written by S4, read by S0/S1 
    curr_wqe_ptr: 64;
    union {
        current_sge_offset: 32;
        struct resp_rx_send_fml_t send_info; // rw 32 bits
    };
    current_sge_id: 8;
    num_sges: 8;

    // place holder for a copy of c_index/p_index to avoid
    // scheduler ringing RQ all the time.
    // proxy_pindex variable is updated using memwr from resp_tx,
    // hence it should be at 16-bit boundary.
    ack_nak_pindex: 16;   // written by S4, read by S0
    proxy_pindex: 16;   // written by TxDMA, read by RxDMA

    srq_id: 24;

    serv_type: 3;           //Ronly
    log_pmtu: 5;            //Ronly
};

// rx -> tx cb. i.e., written by rxdma, read by txdma
struct rqcb2_t {
    rsvd0: 8;                    //1B
    ack_nak_psn: 24;            //3B
    msn         : 24;           //3B
    // used for NAK
    syndrome    : 8;            //1B
    // used for ACK and read responses
    credits     : 5;
    rsvd1       : 3;

    //backtrack info
    struct resp_bt_info_t  bt_info; //20B

    // rnr
    rnr_timeout: 5;
    rsvd2: 3;

    // protection domain - moved from rqcb0_t    
    pd: 32; //4B

    // RQ prefetch related fields
    proxy_cindex: 16;
    prefetch_pindex: 16;
    prefetch_proxy_cindex: 16;
    prefetch_base_addr: 32; //Ronly
    log_num_prefetch_wqes: 5;
    rsvd3       : 3;
    prefetch_buf_index: 16;
    checkout_done: 1;
    prefetch_init_done: 1;

    pad: 134;   //17B
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

    num_pkts_in_curr_msg: 32;
    dma_len: 32;

    pad: 160; //20B
};

//resp_tx stats
struct rqcb4_t {
    num_bytes: 64;
    num_pkts: 32;
    num_read_resp_pkts: 32;
    num_acks: 32;
    num_read_resp_msgs: 16;
    num_atomic_resp_msgs: 16;
    num_pkts_in_cur_msg: 16;
    max_pkts_in_any_msg: 16;
    num_prefetch: 16;

    num_rnrs: 32;
    num_seq_errs: 32;

    last_psn: 24;
    last_syndrome: 8;
    last_msn: 24;

    //dcqcn
    rp_num_additive_increase            :   16;
    rp_num_fast_recovery                :   16;
    rp_num_hyper_increase               :   16;
    rp_num_byte_threshold_db            :   16;
    rp_num_alpha_timer_expiry           :   16;
    rp_num_timer_T_expiry               :   16;
    rp_num_max_rate_reached             :   16;

    qp_err_disabled                     :    1;
    // rsqrkey errors
    qp_err_dis_rsvd_rkey_err            :    1;
    qp_err_dis_rkey_state_err           :    1;
    qp_err_dis_rkey_pd_mismatch         :    1;
    qp_err_dis_rkey_acc_ctrl_err        :    1;
    qp_err_dis_rkey_va_err              :    1;
    qp_err_dis_type2a_mw_qp_mismatch    :    1;
    // resp_rx err
    qp_err_dis_resp_rx                  :    1;

    pad: 32;
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
    num_recirc_drop_pkts: 16;
    num_mem_window_inv: 16;
    num_dup_wr_send_pkts: 16;
    num_dup_rd_atomic_bt_pkts: 16;
    num_dup_rd_atomic_drop_pkts: 16;
    //dcqcn
    np_ecn_marked_packets: 16;
    rp_cnp_processed: 16;
    // errors
    qp_err_disabled                     :    1;
    // stage 0 errors
    qp_err_dis_svc_type_err             :    1;
    qp_err_dis_pyld_len_err             :    1;
    qp_err_dis_last_pkt_len_err         :    1;
    qp_err_dis_pmtu_err                 :    1;
    qp_err_dis_opcode_err               :    1;
    qp_err_dis_access_err               :    1;
    qp_err_dis_wr_only_zero_len_err     :    1;
    qp_err_dis_unaligned_atomic_va_err  :    1;
    // write_dummy errors
    qp_err_dis_dma_len_err              :    1;
    // rqwqe errors
    qp_err_dis_insuff_sge_err           :    1;
    qp_err_dis_max_sge_err              :    1;
    // rqlkey & rqrkey errors
    qp_err_dis_rsvd_key_err             :    1;
    qp_err_dis_key_state_err            :    1;
    qp_err_dis_key_pd_mismatch          :    1;
    qp_err_dis_key_acc_ctrl_err         :    1;
    qp_err_dis_user_key_err             :    1;
    qp_err_dis_key_va_err               :    1;
    // inv_rkey_validate_process
    qp_err_dis_inv_rkey_rsvd_key_err    :    1; 
    qp_err_dis_ineligible_mr_err        :    1; 
    qp_err_dis_inv_rkey_state_err       :    1; 
    qp_err_dis_type1_mw_inv_err         :    1; 
    qp_err_dis_type2a_mw_qp_mismatch    :    1; 
    qp_err_dis_mr_mw_pd_mismatch        :    1;
    // rqlkey_mr_cookie_process
    qp_err_dis_mr_state_invalid         :    1;
    qp_err_dis_mr_cookie_mismatch       :    1;
    // feedback
    qp_err_dis_feedback                 :    1;
    // table errors
    qp_err_dis_table_error              :    1;
    qp_err_dis_phv_intrinsic_error      :    1;
    qp_err_dis_table_resp_error         :    1;
    qp_err_dis_rsvd                     :    2;
    //a packet which went thru too many recirculations had to be terminated and qp had to 
    //be put into error disabled state. The recirc reason, opcode, the psn of the packet etc.
    //are remembered for further debugging.
    recirc_bth_psn: 24;
    last_bth_opcode: 8;
    recirc_reason: 4;
    max_recirc_cnt_err: 1;
    pad: 43;
};

struct rqcb_t {
    struct rqcb0_t rqcb0;
    struct rqcb1_t rqcb1;
    struct rqcb2_t rqcb2;
    struct rqcb3_t rqcb3;
    struct rqcb4_t rqcb4;
    struct rqcb5_t rqcb5;
};

struct prefetch_cb_t {
    p_index: 16;
    c_index: 16;
    pad: 480; // 60B
};

struct prefetch_ring_t {
    val: 16;
    rsvd: 496; // 62B
};

#endif // __RQCB_H
