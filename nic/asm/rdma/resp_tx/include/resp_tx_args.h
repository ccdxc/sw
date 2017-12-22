#ifndef __RESP_TX_ARGS_H
#define __RESP_TX_ARGS_H

#include "capri.h"
#include "common_phv.h"


struct resp_tx_to_stage_dcqcn_info_t {
    dcqcn_cb_addr: 32; // used by dcqcn-mpu-only in stage3.
    congestion_mgmt_enable: 1;
    packet_len: 14;
    new_cindex: 16; // Used by DCQCN algorithm and DCQCN timer.
    rsvd: 65;
};

struct resp_tx_to_stage_rqcb1_wb_info_t {
    new_c_index: 16;
    ack_nack_serv_type: 3;
    ack_nak_process: 1;
    rsvd: 108;
};

struct resp_tx_to_stage_rsq_rkey_info_t {
    dcqcn_cb_addr: 32;
    congestion_mgmt_enable: 1;
    rsvd: 95;
};

struct resp_tx_to_stage_wqe_info_t {
    dcqcn_cb_addr: 32;
    rsvd: 96;
};


struct resp_tx_s0_info_t {
    rsvd: 128;
};

struct resp_tx_s1_info_t {
    rsvd: 128;
};

struct resp_tx_s2_info_t {
    union {
       struct resp_tx_to_stage_wqe_info_t wqe;
    };
};

struct resp_tx_s3_info_t {
    union {
       struct resp_tx_to_stage_rsq_rkey_info_t rsq_rkey;
       struct resp_tx_to_stage_dcqcn_info_t dcqcn;
    };
};

struct resp_tx_s4_info_t {
    union {
       struct resp_tx_to_stage_dcqcn_info_t dcqcn; 
    };
};

struct resp_tx_s5_info_t {
    union {
       struct resp_tx_to_stage_rqcb1_wb_info_t rqcb1_wb;
    };
};

struct resp_tx_s6_info_t {
    rsvd: 128;
};

struct resp_tx_s7_info_t {
    rsvd: 128;
};


struct resp_tx_to_stage_t {
    union {
        struct resp_tx_s0_info_t s0; // rqcb0
        struct resp_tx_s1_info_t s1; // ack-nack / CNP / backtrack
        struct resp_tx_s2_info_t s2; // wqe / dcqcn-mpu-only
        struct resp_tx_s3_info_t s3; // rsq-rkey / dcqcn-mpu-only
        struct resp_tx_s4_info_t s4; // DCQCN Rate-enforce/ DCQCN Algo/ DCQCN timer/ ptseg
        struct resp_tx_s5_info_t s5; // rqcb1-write-back
        struct resp_tx_s6_info_t s6;
        struct resp_tx_s7_info_t s7;
    };
};

#endif //__RESP_TX_ARGS_H
