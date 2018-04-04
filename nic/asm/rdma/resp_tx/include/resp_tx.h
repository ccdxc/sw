#ifndef __RESP_TX_H
#define __RESP_TX_H
#include "capri.h"
#include "types.h"
#include "resp_tx_args.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"
#include "defines.h"

#define RESP_TX_MAX_DMA_CMDS            16
#define RESP_TX_DMA_CMD_START           0
#define RESP_TX_DMA_CMD_INTRINSIC       0
#define RESP_TX_DMA_CMD_TXDMA_INTRINSIC 1
#define RESP_TX_DMA_CMD_COMMON_P4PLUS   2
#define RESP_TX_DMA_CMD_HDR_TEMPLATE    3
#define RESP_TX_DMA_CMD_BTH             4
#define RESP_TX_DMA_CMD_AETH            5
#define RESP_TX_DMA_CMD_CNP_RSVD        5 // CNP packets do not have AETH header. Re-using index.
#define RESP_TX_DMA_CMD_ATOMICAETH      6 
#define RESP_TX_DMA_CMD_PYLD_BASE       6 // consumes 3 DMA commands
#define RESP_TX_DMA_CMD_PAD_ICRC       (RESP_TX_MAX_DMA_CMDS - 2)
#define RESP_TX_DMA_CMD_UDP_OPTIONS    (RESP_TX_MAX_DMA_CMDS - 1)


#define RESP_TX_DMA_CMD_START_FLIT_ID   8 // flits 8-11 are used for dma cmds

// phv 
struct resp_tx_phv_t {
    // dma commands
    /* flit 11 */
    dma_cmd12 : 128;
    dma_cmd13 : 128;
    dma_cmd14 : 128;
    dma_cmd15 : 128;

    /* flit 10 */
    dma_cmd8  : 128;
    dma_cmd9  : 128;
    dma_cmd10 : 128;
    dma_cmd11 : 128;

    /* flit 9 */
    dma_cmd4 : 128;
    dma_cmd5 : 128;
    dma_cmd6 : 128;
    dma_cmd7 : 128;

    /* flit 8 */
    dma_cmd0 : 128;                               // 16B
    dma_cmd1 : 128;                               // 16B
    dma_cmd2 : 128;                               // 16B
    dma_cmd3 : 128;                               // 16B

    /* flit 7 */
    pad      :  24;                               //  3B
    //***NOTE: Keep 4 bytes after pad with in this flit, so that can be copied as ICRC data into phv
    //ICRC data does not need to be given as zero as Capri overtites icrc after computation
    icrc     :  32;                              //  4B
    rsq_c_index   : 16;                          //  2B
    rsvd2    : 280;                              // 37B
    struct p4plus_to_p4_header_t p4plus_to_p4;   // 20B

    /* flit 6 */
    rsvd1   : 16;                                //  2B
    db_data1: 64;                                //  8B
    db_data2: 64;                                //  8B
    struct rdma_atomicaeth_t atomicaeth;         //  8B
    union {                                      // 16B
        struct rdma_cnp_rsvd_t cnp_rsvd;                 // 16B
        struct rdma_aeth_t aeth;                         //  4B
    }; // CNP packets do not have aeth header.
    struct rdma_bth_t bth;                       // 12B
    rsvd3   : 80;                                // 10B

    // common tx (flit 0 - 5)
    struct phv_ common;
};

#if 0
struct resp_tx_phv_global_t {
    struct phv_global_common_t common;
};

// stage to stage argument structures

//20
#if 0
struct resp_tx_rqcb_to_rqcb1_info_t {
    rsqwqe_addr: 64;
    log_pmtu: 5;
    serv_type: 3;
    timer_event_process: 1;
    curr_read_rsp_psn: 24;
    read_rsp_in_progress: 1;
    pad: 62;
};

struct resp_tx_rqcb1_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb_to_rqcb1_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};
#endif

struct resp_tx_rqcb_to_rqcb2_info_t {
    rsqwqe_addr: 64;
    curr_read_rsp_psn: 24;
    log_pmtu: 5;
    serv_type: 3;
    header_template_addr: 32;
    header_template_size: 8;
    read_rsp_in_progress: 1;
    pad: 23;
};

struct resp_tx_rqcb2_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb_to_rqcb2_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_tx_rqcb_to_ack_info_t {
    header_template_addr: 32;
    pad: 128;
};

struct resp_tx_ack_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb_to_ack_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_tx_rqcb2_to_rsqwqe_info_t {
    curr_read_rsp_psn: 24;
    log_pmtu: 5;
    serv_type: 3;
    header_template_addr: 32;
    header_template_size: 8;
    read_rsp_in_progress: 1;
    pad: 87;
};

struct resp_tx_rsqwqe_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb2_to_rsqwqe_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_tx_rsqwqe_to_rkey_info_t {
    transfer_va: 64;
    header_template_addr: 32;
    curr_read_rsp_psn: 24;
    log_pmtu: 5;
    key_id: 1;
    send_aeth: 1;
    last_or_only: 1;
    transfer_bytes: 12;
    header_template_size: 8;
    pad: 12;
};

struct resp_tx_rsqrkey_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rsqwqe_to_rkey_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_tx_rqcb0_write_back_info_t {
    curr_read_rsp_psn: 24;
    read_rsp_in_progress: 1;
    rate_enforce_failed: 1;
    pad: 134;
};

struct resp_tx_rqcb0_write_back_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb0_write_back_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_tx_rkey_to_ptseg_info_t {
    pt_seg_offset: 32;
    pt_seg_bytes: 32;
    dma_cmd_start_index: 8;
    log_page_size: 5;
    tbl_id: 2;
    pad: 81;
};

struct resp_tx_rsqptseg_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rkey_to_ptseg_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};


//20
struct resp_tx_rsq_backtrack_adjust_info_t {
    adjust_rsq_c_index: 16;
    rsq_bt_p_index: 16;
    pad: 128;
};

struct resp_tx_rsq_backtrack_adjust_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rsq_backtrack_adjust_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

struct resp_tx_rqcb_to_cnp_info_t {
    new_c_index: 16;
    pad: 144;   
};  
    
struct resp_tx_cnp_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_tx_rqcb_to_cnp_info_t args;
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};  
#endif


#endif //__RESP_TX_H
