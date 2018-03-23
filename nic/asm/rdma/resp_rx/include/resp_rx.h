#ifndef __RESP_RX_H
#define __RESP_RX_H
#include "capri.h"
#include "types.h"
#include "resp_rx_args.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"

#define RESP_RX_MAX_DMA_CMDS        16

#define RESP_RX_CQCB_ADDR_GET(_r, _cqid) \
    CQCB_ADDR_GET(_r, _cqid, CAPRI_KEY_RANGE(to_s3_wb1_info, cqcb_base_addr_page_id_sbit0_ebit15, cqcb_base_addr_page_id_sbit16_ebit21));

#define RESP_RX_EQCB_ADDR_GET(_r, _tmp_r, _eqid) \
    EQCB_ADDR_GET(_r, _tmp_r, _eqid, k.{to_s5_cqpt_info_cqcb_base_addr_page_id_sbit0_ebit15...to_s5_cqpt_info_cqcb_base_addr_page_id_sbit16_ebit21}, k.{to_s5_cqpt_info_log_num_cq_entries_sbit0_ebit1...to_s5_cqpt_info_log_num_cq_entries_sbit2_ebit3});

// currently PYLD_BASE starts at 2, each PTSEG can generate upto 3
// dma instructions. so, (2,3,4),(5,6,7),(8,9,10),(11,12,13) will
// accommodate a packet spaning upto 4 SGEs where each SGE can generate
// upto 3 DMA instructions.
// NOTE: 13th DMA command is overlapping with IMMDT_AS_DBELL instruction
// below. We are leaving it that way assuming that IMMDT_AS_DBELL is
// used only by storage folks, their case wouldn't be needing 4 SGEs
// anyway.
// In cases of error (KEY permissions, VA offset mismatch etc.,) where
// one do not want to transfer payload, skip to eop should be marked thru
// a DMA instruction. Using DMA_CMD 12 for that, as it is mutually exclusive
// to PYLD transfer
#define RESP_RX_DMA_CMD_PYLD_BASE   2
#define RESP_RX_DMA_CMD_RSQWQE      2
#define RESP_RX_DMA_CMD_ATOMIC_RESOURCE_WR          3
#define RESP_RX_DMA_CMD_ATOMIC_RESOURCE_RD          4
#define RESP_RX_DMA_CMD_ATOMIC_RESOURCE_TO_RSQWQE   5
#define RESP_RX_DMA_CMD_RELEASE_ATOMIC_RESOURCE     6
#define RESP_RX_DMA_CMD_RSQ_DB                      7

#define RESP_RX_DMA_CMD_PYLD_BASE_END 14

#define RESP_RX_DMA_CMD_START       0
#define RESP_RX_DMA_CMD_ACK         0
#define RESP_RX_DMA_CMD_SKIP_PLD_ON_ERROR (RESP_RX_MAX_DMA_CMDS - 4)
#define RESP_RX_DMA_CMD_IMMDT_AS_DBELL (RESP_RX_MAX_DMA_CMDS - 3)
#define RESP_RX_DMA_CMD_CQ          (RESP_RX_MAX_DMA_CMDS - 2)
#define RESP_RX_DMA_CMD_EQ          (RESP_RX_MAX_DMA_CMDS - 1)
//wakeup dpath and EQ are mutually exclusive
#define RESP_RX_DMA_CMD_WAKEUP_DPATH RESP_RX_DMA_CMD_EQ

#define RESP_RX_DMA_CMD_START_FLIT_ID   8 // flits 8-11 are used for dma cmds

// for read and atomic operations, use flit id 9 as the start flit id
// of DMA operations
#define RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID 9

//TODO: put ack_info.aeth, ack_info.psn adjacent to each other in PHV and also
//      adjacent to each other in rqcb1, in right order. This will eliminate
//      one DMA instruction
#define RESP_RX_POST_ACK_INFO_TO_TXDMA_NO_DB(_dma_base_r, _rqcb2_addr_r, _tmp_r) \
    add         _tmp_r, _rqcb2_addr_r, FIELD_OFFSET(rqcb2_t, ack_nak_psn); \
    DMA_HBM_PHV2MEM_SETUP(_dma_base_r, ack_info.psn, ack_info.aeth.msn, _tmp_r); \

#define RESP_RX_POST_ACK_INFO_TO_TXDMA_DB_ONLY(_dma_base_r, \
                                       _lif, _qtype, _qid, \
                                       _db_addr_r, _db_data_r) \
    DMA_NEXT_CMD_I_BASE_GET(_dma_base_r, 1); \
    PREPARE_DOORBELL_INC_PINDEX(_lif, _qtype, _qid, ACK_NAK_RING_ID, _db_addr_r, _db_data_r);\
    phvwr       p.db_data1, _db_data_r.dx; \
    DMA_HBM_PHV2MEM_SETUP(_dma_base_r, db_data1, db_data1, _db_addr_r); \
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, _dma_base_r); \
    
#define RESP_RX_POST_ACK_INFO_TO_TXDMA(_dma_base_r, _rqcb2_addr_r, _tmp_r, \
                                       _lif, _qtype, _qid, \
                                       _db_addr_r, _db_data_r) \
    RESP_RX_POST_ACK_INFO_TO_TXDMA_NO_DB(_dma_base_r, _rqcb2_addr_r, _tmp_r); \
    RESP_RX_POST_ACK_INFO_TO_TXDMA_DB_ONLY(_dma_base_r, \
                                       _lif, _qtype, _qid, \
                                       _db_addr_r, _db_data_r); \
    
#ring-id assumed as 0
#define RESP_RX_POST_IMMDT_AS_DOORBELL_INCR_PINDEX(_dma_base_r, \
                                       _lif, _qtype, _qid, \
                                       _db_addr_r, _db_data_r) \
    PREPARE_DOORBELL_INC_PINDEX(_lif, _qtype, _qid, 0 /*ring-id*/, _db_addr_r, _db_data_r);\
    phvwr       p.immdt_as_dbell_data, _db_data_r.dx; \
    DMA_HBM_PHV2MEM_SETUP(_dma_base_r, immdt_as_dbell_data, immdt_as_dbell_data, _db_addr_r); \
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, _dma_base_r); \

#ring-id assumed as 0
#define RESP_RX_POST_IMMDT_AS_DOORBELL_SET_PINDEX(_dma_base_r, \
                                       _lif, _qtype, _qid, \
                                       _db_addr_r, _db_data_r) \
    PREPARE_DOORBELL_WRITE_PINDEX(_lif, _qtype, _qid, 0 /*ring-id*/, 0 /*pindex*/, _db_addr_r, _db_data_r);\
    phvwr       p.immdt_as_dbell_data, _db_data_r.dx; \
    DMA_HBM_PHV2MEM_SETUP(_dma_base_r, immdt_as_dbell_data, immdt_as_dbell_data, _db_addr_r); \
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, _dma_base_r); \

//immdt_as_dbell and wakeup_dpath are used mutually exclusively. Hence using the same phv field.
//dma_cmd_idx is not the same though
#define RESP_RX_POST_WAKEUP_DPATH_INCR_PINDEX(_dma_base_r, \
                                       _lif, _qtype, _qid, _ring_id, \
                                       _db_addr_r, _db_data_r) \
    PREPARE_DOORBELL_INC_PINDEX(_lif, _qtype, _qid, _ring_id, _db_addr_r, _db_data_r);\
    phvwr       p.immdt_as_dbell_data, _db_data_r.dx; \
    DMA_HBM_PHV2MEM_SETUP(_dma_base_r, immdt_as_dbell_data, immdt_as_dbell_data, _db_addr_r); \
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, _dma_base_r); \

      
#define RESP_RX_UPDATE_IMM_AS_DB_DATA_WITH_PINDEX(_pindex) \
    phvwr       p.immdt_as_dbell_data[63:48], _pindex;

#define RESP_RX_RING_ACK_NAK_DB(_dma_base_r,  \
                                _lif, _qtype, _qid, \
                                _db_addr_r, _db_data_r) \
    PREPARE_DOORBELL_INC_PINDEX(_lif, _qtype, _qid, ACK_NAK_RING_ID, _db_addr_r, _db_data_r);\
    phvwr       p.db_data1, _db_data_r.dx; \
    DMA_HBM_PHV2MEM_SETUP(_dma_base_r, db_data1, db_data1, _db_addr_r);

#define RSQ_EVAL_MIDDLE     0
#define RSQ_WALK_FORWARD    1
#define RSQ_WALK_BACKWARD   2

struct resp_rx_dma_cmds_flit_t {
    dma_cmd0 : 128;
    dma_cmd1 : 128;
    dma_cmd2 : 128;
    dma_cmd3 : 128;
};

    
// phv 
struct resp_rx_phv_t {
    /* flit 11 */
    struct resp_rx_dma_cmds_flit_t flit_11;

    /* flit 10 */
    
    struct resp_rx_dma_cmds_flit_t  flit_10;

    /* flit 9 */
    struct resp_rx_dma_cmds_flit_t flit_9;
 
    /* flit 8 */
    union {
        struct resp_rx_dma_cmds_flit_t  flit_8;
        struct rdma_pcie_atomic_reg_t   pcie_atomic;
    };
    
    // scratch (flit 7):
    // size: 42 =  2 + 2 + 8 + 8 + 1 + 7 + 4 + 1 + 8 + 1
    spec_cindex: 16;
    eq_int_num: 16;
    db_data1: 64;
    db_data2: 64;
    adjust_rsq_c_index: 8;
    struct ack_info_t ack_info;
    struct eqwqe_t eqwqe;
    my_token_id: 8;
    immdt_as_dbell_data: 64;
    atomic_release_byte: 8;
    pad: 176;   //22B

    // scratch (flit 6)
    // size: 64  = 32 + 32
    struct rsqwqe_t rsqwqe;
    struct cqwqe_t cqwqe;

    // common rx (flit 0 - 5)
    struct phv_ common;
};

// we still need to retain these structures due to the special nature of wqe/pt 
// functions which deal with Table 0 and/or Table 1.
// TBD: either auto-generate this structure from P4 or migrate to regular phvwr
//20
struct resp_rx_key_info_t {
    va: 64;
    //keep len...tbl_id in the same order
    //aligning with resp_rx_lkey_to_pt_info_t
    len: 32;
    dma_cmd_start_index: 8;
    tbl_id: 8;
    acc_ctrl: 8;
    dma_cmdeop: 1;
    nak_code: 8;
    incr_nxt_to_go_token_id: 1;
    incr_c_index: 1;
    skip_pt: 1;
    invoke_writeback: 1;
    pad:27;
};

struct resp_rx_lkey_to_pt_info_t {
    pt_offset: 32;
    //keep pt_bytes...sge_index in the same order
    //aligning with resp_rx_key_info_t
    pt_bytes: 32;
    dma_cmd_start_index: 8;
    sge_index: 8;
    log_page_size: 5;
    dma_cmdeop: 1;
    rsvd: 2;
    override_lif_vld: 1;
    override_lif: 12;
    pad: 59;
};

#if 0
struct resp_rx_phv_global_t {
    struct phv_global_common_t common;
};

// stage to stage argument structures

// 20
struct resp_rx_rqcb_to_pt_info_t {
    in_progress: 1;
    page_seg_offset: 3;
    tbl_id: 3;
    rsvd: 1;
    page_offset: 16;
    remaining_payload_bytes: 16;
    pad: 120;
};

struct resp_rx_rqpt_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_pt_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

// 20
struct resp_rx_rqcb_to_wqe_info_t {
    rsvd: 6;
    recirc_path:1;
    in_progress:1;
    remaining_payload_bytes: 16;
    curr_wqe_ptr: 64;
    current_sge_offset: 32;
    current_sge_id: 8;
    num_valid_sges: 8;
    dma_cmd_index: 8;
    pad: 16;
};

struct resp_rx_rqwqe_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_wqe_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_key_info_t {
    va: 64;
    //keep len...tbl_id in the same order
    //aligning with resp_rx_lkey_to_pt_info_t
    len: 32;
    dma_cmd_start_index: 8;
    tbl_id: 8;
    acc_ctrl: 8;
    dma_cmdeop: 1;
    nak_code: 8;
    incr_nxt_to_go_token_id: 1;
    incr_c_index: 1;
    skip_pt: 1;
    invoke_writeback: 1;
    pad:27;
};

struct resp_rx_key_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_key_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_rqcb1_write_back_info_t {
    in_progress: 1;
    incr_nxt_to_go_token_id: 1;
    incr_c_index: 1;
    tbl_id: 3;
    update_wqe_ptr: 1;
    update_num_sges: 1;
    curr_wqe_ptr: 64;
    current_sge_offset: 32;
    current_sge_id: 8;
    num_sges: 8;
    pad: 40;
};

struct resp_rx_rqcb1_write_back_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb1_write_back_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_lkey_to_pt_info_t {
    pt_offset: 32;
    //keep pt_bytes...sge_index in the same order
    //aligning with resp_rx_key_info_t
    pt_bytes: 32;
    dma_cmd_start_index: 8;
    sge_index: 8;
    log_page_size: 5;
    dma_cmdeop: 1;
    rsvd: 2;
    override_lif_vld: 1;
    override_lif: 12;
    pad: 59;
};

struct resp_rx_ptseg_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_lkey_to_pt_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_rqcb_to_cq_info_t {
    pad: 160;
};

struct resp_rx_cqcb_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_cq_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_cqcb_to_pt_info_t {
    page_offset: 16;
    page_seg_offset: 8;
    cq_id: 24;
    eq_id: 24;
    arm: 1;
    wakeup_dpath: 1;
    no_translate: 1;
    no_dma: 1;
    rsvd1: 4;
    cqcb_addr: 34;
    rsvd2: 6;
    pa_next_index: 16;
    pad: 24;
};

struct resp_rx_cqpt_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_cqcb_to_pt_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_cqcb_to_eq_info_t {
    cq_id: 24;
    pad: 136;
};

struct resp_rx_eqcb_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_cqcb_to_eq_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_rqcb_to_rqcb1_info_t {
    rsvd: 7;
    in_progress: 1;
    remaining_payload_bytes: 16;
    curr_wqe_ptr: 64;
    current_sge_offset: 32;
    current_sge_id: 8;
    num_sges: 8;
    pad: 24;
};

struct resp_rx_rqcb1_in_progress_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_rqcb1_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_rqcb_to_write_rkey_info_t {
    va: 64;
    len: 32;
    r_key: 32;
    remaining_payload_bytes: 16;
    load_reth: 1;
    incr_c_index: 1;
    rsvd: 14;
};

struct resp_rx_write_dummy_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_write_rkey_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_rqcb_to_read_atomic_rkey_info_t {
    va: 64;
    len: 32;
    r_key: 32;
    rsq_p_index: 16;
    skip_rsq_dbell: 1;
    read_or_atomic: 1;
    pad: 14;
};

struct resp_rx_read_atomic_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rqcb_to_read_atomic_rkey_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_inv_rkey_info_t {
    pad: 160;
};

struct resp_rx_inv_rkey_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_inv_rkey_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_rsq_backtrack_info_t {                         
    log_pmtu: 5;
    read_or_atomic:1;                                             
    walk:2;
    hi_index: 16;
    lo_index: 16;
    index: 8;
    log_rsq_size: 5;
    rsvd: 3;
    search_psn:24;
    rsq_base_addr: 32;
    pad: 48;
};

struct resp_rx_rsq_backtrack_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rsq_backtrack_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_rsq_backtrack_adjust_info_t {
    adjust_c_index: 1;
    adjust_p_index: 1;
    rsvd: 6;
    index: 8;
    pad: 144;
};

struct resp_rx_rsq_backtrack_adjust_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_rsq_backtrack_adjust_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_stats_info_t {
    pad : 160;
};


struct resp_rx_stats_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_stats_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

//20
struct resp_rx_ecn_info_t {
    p_key: 16;
    pad : 144;
};


struct resp_rx_ecn_process_k_t {
    struct capri_intrinsic_raw_k_t intrinsic;
    struct resp_rx_ecn_info_t args;
    struct resp_rx_to_stage_t to_stage;
    struct phv_global_common_t global;
};
#endif

#endif //__RESP_RX_H
