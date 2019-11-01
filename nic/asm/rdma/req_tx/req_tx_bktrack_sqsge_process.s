#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"

struct req_tx_phv_t p;
struct req_tx_s3_t0_k k;

#define SQ_BKTRACK_P t0_s2s_sq_bktrack_info
#define SQCB0_WRITE_BACK_P t0_s2s_sqcb_write_back_info
#define SQCB2_WRITE_BACK_P t1_s2s_bktrack_sqcb2_write_back_info

#define TO_S1_BT_P to_s1_bt_info
#define TO_S2_BT_P to_s2_bt_info
#define TO_S3_BT_P to_s3_bt_info
#define TO_S4_BT_P to_s4_bt_info
#define TO_S5_BT_P to_s5_bt_info
#define TO_S6_BT_P to_s6_bt_info

#define TO_S7_BT_WB_P to_s7_bt_wb_info

#define IN_P t0_s2s_sq_bktrack_info
#define IN_TO_S_P to_s3_bt_info

#define K_CURRENT_SGE_ID CAPRI_KEY_RANGE(IN_P, current_sge_id_sbit0_ebit6, current_sge_id_sbit7_ebit7)
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit6, current_sge_offset_sbit31_ebit31)
#define K_TX_PSN     CAPRI_KEY_RANGE(IN_P, tx_psn_sbit0_ebit6, tx_psn_sbit23_ebit23)
#define K_REXMIT_PSN CAPRI_KEY_RANGE(IN_TO_S_P, rexmit_psn_sbit0_ebit3, rexmit_psn_sbit20_ebit23)
#define K_LOG_PMTU   CAPRI_KEY_FIELD(IN_TO_S_P, log_pmtu)
#define K_SQ_C_INDEX CAPRI_KEY_RANGE(IN_P, sq_c_index_sbit0_ebit7, sq_c_index_sbit8_ebit15)
#define K_SQ_P_INDEX CAPRI_KEY_RANGE(IN_P, sq_p_index_or_imm_data1_or_inv_key1_sbit0_ebit2, sq_p_index_or_imm_data1_or_inv_key1_sbit11_ebit15)
#define K_SSN        CAPRI_KEY_RANGE(IN_P, ssn_sbit0_ebit6, ssn_sbit23_ebit23)
#define K_IMM_DATA   CAPRI_KEY_RANGE(IN_P, sq_p_index_or_imm_data1_or_inv_key1_sbit0_ebit2, imm_data2_or_inv_key2_sbit11_ebit15)
#define K_INV_KEY    K_IMM_DATA
#define K_NUM_SGES   CAPRI_KEY_RANGE(IN_P, num_sges_sbit0_ebit6, num_sges_sbit7_ebit7)
#define K_WQE_ADDR   CAPRI_KEY_RANGE(IN_TO_S_P, wqe_addr_sbit0_ebit31, wqe_addr_sbit56_ebit63)

%%

    .param    req_tx_bktrack_write_back_process
    .param    req_tx_bktrack_sqcb2_write_back_process

.align
req_tx_bktrack_sqsge_process:
    // sge_t are accessed from bottom to top in big-endian, hence go to
    // the bottom of the SGE_T
    //sge_t[0]
    add            r1, r0, (HBM_NUM_SGES_PER_CACHELINE - 1), LOG_SIZEOF_SGE_T_BITS
    add            r2, K_CURRENT_SGE_OFFSET, r0
    add            r3, K_TX_PSN, r0
    // set sge iterator to current_sge_id
    add            r4, K_CURRENT_SGE_ID, 0
    // To start with, set empty_rrq_backtrack to false
    setcf          c7, [!c0]

sge_loop:
    sle            c2, K_NUM_SGES, r4
    bcf            [c2], invalid_num_sges

    CAPRI_TABLE_GET_FIELD(r5, r1, SGE_T, len)
    //if (sge_p->len > current_sge_offset)
    blt            r5, r2, next_sge_start
    nop            // Branch Delay Slot

    // length = sge_p->len - current_sge_offset
    sub            r5, r5, r2 

    // num_pkts = (length + (1 << log_pmtu) - 1) >> log_pmtu
    sll            r7, 1, K_LOG_PMTU
    add            r6, r7, r5
    sub            r6, r6, 1
    srl            r6, r6, K_LOG_PMTU
    
    // if (rexmit_psn < (tx_psn + num_pkts))
    add            r7, r3, r6
    mincr          r7, 24, r0
    scwlt24        c1, K_REXMIT_PSN, r7
    bcf            [!c1], next_sge

    // current_sge_offset = ((rexmit_psn  - tx_psn) << log_pmtu) + current_sge_offset
    sub            r7, K_REXMIT_PSN, r3
    mincr          r7, 24, r0
    sll            r7, r7, K_LOG_PMTU
    add            r2, r7, r2

    // tx_psn = rexmit_psn
    add            r3, K_REXMIT_PSN, r0

    // bktrack_in_progress = false
    setcf          c6, [!c0]
    b              sqcb_writeback
    // set empty_rrq_bktrack to true on completion of bktracking
    setcf          c7, [c0] // Branch Delay Slot

next_sge:
    // current_sge_offset = partial_pkty_bytes ? ((1 << log_pmtu) - partial_pkt_bytes) : 0
    mincr          r5, K_LOG_PMTU, r0  
    seq            c1, r5, r0
    sll.!c1        r7, 1, K_LOG_PMTU
    sub.!c1        r2, r7, r5
    add.c1         r2, r5, r0

    add            r4, r4, 1
    // tx_psn = tx_psn + num_pkts
    add            r3, r6, r3
    beq            r1, r0, sge_bktrack
    mincr          r3, 24, r0 // Branch Delay Slot

    b              sge_loop
    // current_sge_id = current_sge_id + 1
    sub            r1, r1, 1, LOG_SIZEOF_SGE_T_BITS // Branch Delay Slot

next_sge_start:
    add            r4, r4, 1
    beq            r1, r0, sge_bktrack
    // current_sge_offset -= sge_p->len
    sub            r2, r2, r5 // Branch Delay Slot

    b              sge_loop
    // current_sge_id = current_sge_id + 1
    sub            r1, r1, 1, LOG_SIZEOF_SGE_T_BITS // Branch Delay Slot

sge_bktrack:
    mfspr          r5, spr_mpuid
    seq            c1, r5[4:2], CAPRI_STAGE_LAST-1
    bcf            [c1], sqcb_writeback
    // bktrack_in_progress = true
    setcf          c6, [c0] // Branch Delay Slot

    CAPRI_RESET_TABLE_0_ARG()

    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, tx_psn), r3, CAPRI_PHV_FIELD(SQ_BKTRACK_P, ssn), K_SSN
    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, sq_c_index), K_SQ_C_INDEX, CAPRI_PHV_FIELD(SQ_BKTRACK_P, sq_p_index_or_imm_data1_or_inv_key1), K_SQ_P_INDEX
    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, current_sge_id), r4, CAPRI_PHV_FIELD(SQ_BKTRACK_P, current_sge_offset), r2
    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, in_progress), 1, CAPRI_PHV_FIELD(SQ_BKTRACK_P, num_sges), K_NUM_SGES
    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, op_type), CAPRI_KEY_FIELD(IN_P, op_type), CAPRI_PHV_RANGE(SQ_BKTRACK_P, sq_p_index_or_imm_data1_or_inv_key1, imm_data2_or_inv_key2), K_IMM_DATA

    // sge_addr = wqe_addr + TXWQE_SGE_OFFSET + (sizeof(sge_t) * current_sge_id)
    add          r5,  K_WQE_ADDR, r1, LOG_SIZEOF_SGE_T
    add          r5, r5, TXWQE_SGE_OFFSET

    // label and pc cannot be same, so calculate cur pc using bal 
    // and compute start pc deducting the current instruction position
    bal     r6, calculate_raw_table_pc
    nop     //BD Slot

calculate_raw_table_pc:
    // "$" here denores releative current instruction position
    sub     r6, r6, $
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r5)

    nop.e
    nop

sqcb_writeback:
    phvwr          p.common.p4_intr_global_drop, 1

    CAPRI_RESET_TABLE_0_ARG()

    phvwr CAPRI_PHV_FIELD(SQCB0_WRITE_BACK_P, in_progress), 1
    phvwr.c6 CAPRI_PHV_FIELD(SQCB0_WRITE_BACK_P, bktrack_in_progress), 1

    phvwrpair CAPRI_PHV_FIELD(SQCB0_WRITE_BACK_P, current_sge_offset), r2, CAPRI_PHV_FIELD(SQCB0_WRITE_BACK_P, current_sge_id), r4
    //sub            r2, K_NUM_SGES, r1
    phvwrpair CAPRI_PHV_FIELD(SQCB0_WRITE_BACK_P, op_type), CAPRI_KEY_FIELD(IN_P, op_type), CAPRI_PHV_FIELD(SQCB0_WRITE_BACK_P, num_sges), K_NUM_SGES
    phvwr CAPRI_PHV_FIELD(SQCB0_WRITE_BACK_P, sq_c_index), K_SQ_C_INDEX
    phvwr.c7 CAPRI_PHV_FIELD(SQCB0_WRITE_BACK_P, empty_rrq_bktrack), 1

    SQCB0_ADDR_GET(r5)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_write_back_process, r5)

    CAPRI_RESET_TABLE_1_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQCB2_WRITE_BACK_P, tx_psn), r3, CAPRI_PHV_FIELD(SQCB2_WRITE_BACK_P, sq_cindex), K_SQ_C_INDEX
    phvwrpair CAPRI_PHV_FIELD(SQCB2_WRITE_BACK_P, ssn), K_SSN, \
              CAPRI_PHV_FIELD(SQCB2_WRITE_BACK_P, imm_data_or_inv_key), K_IMM_DATA
    phvwr     CAPRI_PHV_FIELD(SQCB2_WRITE_BACK_P, op_type), CAPRI_KEY_FIELD(IN_P, op_type)
    phvwr.c6  CAPRI_PHV_FIELD(SQCB2_WRITE_BACK_P, bktrack_in_progress), 1

    SQCB2_ADDR_GET(r5)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqcb2_write_back_process, r5)

    nop.e
    nop

invalid_num_sges:
    //phvwrpair      CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_disabled), 1, \
    //               CAPRI_PHV_FILED(TO_S7_STATS_INFO_P, qp_err_dis_bktrack_inv_num_sges), 1
    phvwrpair    p.{rdma_feedback.completion.status, rdma_feedback.completion.error,rdma_feedback.completion.err_qp_instantly}, \
                     ((CQ_STATUS_LOCAL_QP_OPER_ERR << 2) | (1 << 1) | 1), \
                 p.{rdma_feedback.completion.lif_cqe_error_id_vld, rdma_feedback.completion.lif_error_id_vld, rdma_feedback.completion.lif_error_id}, \
                     ((1 << 5) | (1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_TX_LOCAL_OPER_ERR_OFFSET))

    phvwr          CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp),  1

    SQCB0_ADDR_GET(r5)
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_write_back_process, r5)
