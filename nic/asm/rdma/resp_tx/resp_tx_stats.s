#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"


struct resp_tx_phv_t p;
struct resp_tx_s7_t3_k k;
struct rqcb4_t d;

#define IN_P    to_s7_stats_info
#define K_DCQCN_FLAGS CAPRI_KEY_RANGE(IN_P, rp_num_additive_increase, rp_num_max_rate_reached)
#define K_LAST_SYNDROME CAPRI_KEY_FIELD(IN_P, last_syndrome)
#define K_LAST_MSN CAPRI_KEY_FIELD(IN_P, last_msn)
#define K_LAST_PSN CAPRI_KEY_FIELD(IN_P, last_psn)
#define K_ERR_DIS_REASON_CODES CAPRI_KEY_RANGE(IN_P, qp_err_disabled, qp_err_dis_resp_rx)
#define K_INCR_PREFETCH_CNT CAPRI_KEY_FIELD(IN_P, incr_prefetch_cnt)
#define K_PREFETCH_ONLY CAPRI_KEY_FIELD(IN_P, prefetch_only)

#define GLOBAL_FLAGS r7
#define RQCB4_ADDR   r3
#define STATS_PC     r6

#define MASK_16 16
#define MASK_32 32

%%

.param  lif_stats_base

.align
resp_tx_stats_process:

    // Pin stats process to stage 7
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_7
    bcf              [!c1], bubble_to_next_stage
    seq              c2, K_INCR_PREFETCH_CNT, 1 // BD Slot

    bbeq             K_PREFETCH_ONLY, 1, exit
    tblmincri.c2     d.num_prefetch, MASK_16, 1 // BD Slot

    // if already error disabled, do not update the stats anymore
    bbeq             d.qp_err_disabled, 1, exit
    add              GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS //BD Slot

    // if flush rq, skip updating num_pkts and num_bytes
    bbeq             CAPRI_KEY_FIELD(IN_P, flush_rq), 1, err_dis_qp_stats
    seq              c7, CAPRI_KEY_FIELD(IN_P, dcqcn_rate), 1 // BD Slot

    // if dcqcn rate, skip updating num_pkts and num_bytes etc.
    bcf              [c7], dcqcn_rate_stats
    seq              c7, CAPRI_KEY_FIELD(IN_P, dcqcn_timer), 1 // BD Slot

    // if dcqcn timer, skip updating num_pkts and num_bytes etc.
    bcf              [c7], dcqcn_timer_stats

    seq              c7, CAPRI_KEY_FIELD(to_s7_stats_info, rp_num_byte_threshold_db), 1 // BD Slot
    tblmincri.c7     d.rp_num_byte_threshold_db, MASK_16, 1

    crestore         [c7, c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (RESP_TX_FLAG_ATOMIC_RESP | RESP_TX_FLAG_READ_RESP | RESP_TX_FLAG_ACK | RESP_TX_FLAG_ONLY | RESP_TX_FLAG_LAST | RESP_TX_FLAG_FIRST | RESP_TX_FLAG_ERR_DIS_QP) // BD Slot
    // c7 - atomic_resp, c6 - read_resp, c5 - ack, c4 - only, c3 - last, c2 - first, c1 - err_dis_qp

    tbladd           d.num_bytes, CAPRI_KEY_FIELD(to_s7_stats_info, pyld_bytes)
    tblmincri        d.num_pkts, MASK_32, 1

    bcf              [c1], err_dis_qp_stats
    tblmincri.c7     d.num_atomic_resp_msgs, MASK_16, 1 //BD Slot
    tblmincri.c6     d.num_read_resp_pkts, MASK_32, 1

    setcf            c1, [c4 | c2]  // only | last
    setcf            c1, [c6 & c1]  // read & (only | last)
    tblmincri.c1     d.num_read_resp_msgs, MASK_16, 1

    IS_ANY_FLAG_SET(c1, GLOBAL_FLAGS, RESP_TX_FLAG_ONLY | RESP_TX_FLAG_FIRST | RESP_TX_FLAG_ATOMIC_RESP | RESP_TX_FLAG_ACK)
    tblwr.c1         d.num_pkts_in_cur_msg, 1
    tblmincri.!c1    d.num_pkts_in_cur_msg, MASK_16, 1

    //peak
    add              r4, r0, d.max_pkts_in_any_msg
    sslt             c1, r4, d.num_pkts_in_cur_msg, r0
    tblwr.c1         d.max_pkts_in_any_msg, d.num_pkts_in_cur_msg

    // is there an aeth hdr ?
    IS_ANY_FLAG_SET(c1, GLOBAL_FLAGS, RESP_TX_FLAG_ONLY | RESP_TX_FLAG_FIRST | RESP_TX_FLAG_LAST | RESP_TX_FLAG_ATOMIC_RESP | RESP_TX_FLAG_ACK)
    bcf             [!c1], handle_lif_stats
    add             r2, r0, K_LAST_SYNDROME //BD Slot
 
    //aeth stats
    tblwr           d.last_syndrome, K_LAST_SYNDROME
    tblwr           d.last_psn, K_LAST_PSN
    tblwr           d.last_msn, K_LAST_MSN

    bcf             [!c5], handle_lif_stats
    seq             c1, r2[7:5], AETH_CODE_ACK

    // ack stats
    tblmincri.c1    d.num_acks, MASK_32, 1     //count only positive standalone acks
    seq             c1, r2[7:5], AETH_CODE_RNR
    tblmincri.c1    d.num_rnrs, MASK_32, 1
    seq             c2, r2, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_SEQ_ERR)
    bcf             [c1 | c2], handle_error_lif_stats
    tblmincri.c2    d.num_seq_errs, MASK_32, 1 //BD Slot

handle_lif_stats:

#ifndef GFT

    addi            r1, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT

    #uc bytes and packets
    addi            r3, r2, LIF_STATS_TX_RDMA_UCAST_BYTES_OFFSET

    ATOMIC_INC_VAL_2(r1, r3, r4, r5, CAPRI_KEY_FIELD(to_s7_stats_info, pyld_bytes), 1)

#endif

    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], exit

    //invoke the same routine, but with valid d[]
    //using static config, stage-7/table-3 is set as memory_only - to improve on latency
    CAPRI_GET_TABLE_3_K(resp_tx_phv_t, r7) //BD Slot
    RQCB4_ADDR_GET(RQCB4_ADDR)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RQCB4_ADDR)

exit:

    nop.e
    nop

dcqcn_rate_stats:
    crestore         [c7, c6, c5, c4], K_DCQCN_FLAGS, 0xFF
    //c7-rp_num_additive_increase, c1-rp_num_max_rate_reached

    tblmincri.c7     d.rp_num_additive_increase, MASK_16, 1
    tblmincri.c6     d.rp_num_fast_recovery, MASK_16, 1
    tblmincri.c5     d.rp_num_hyper_increase, MASK_16, 1
    tblmincri.c4     d.rp_num_max_rate_reached, MASK_16, 1
    nop.e
    nop

dcqcn_timer_stats:
    seq              c7, CAPRI_KEY_FIELD(to_s7_stats_info, rp_num_alpha_timer_expiry), 1
    tblmincri.c7     d.rp_num_alpha_timer_expiry, MASK_16, 1
    seq              c6, CAPRI_KEY_FIELD(to_s7_stats_info, rp_num_timer_T_expiry), 1
    tblmincri.c6     d.rp_num_timer_T_expiry, MASK_16, 1
    nop.e
    nop

err_dis_qp_stats:
    tblwr           d.last_syndrome, K_LAST_SYNDROME
    tblwr           d.last_psn, K_LAST_PSN
    tblwr           d.last_msn, K_LAST_MSN
    tblor           d.{qp_err_disabled...qp_err_dis_resp_rx}, K_ERR_DIS_REASON_CODES

handle_error_lif_stats:

#ifndef GFT

    addi            r1, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START //BD Slot
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT
 
    addi            r3, r2, LIF_STATS_RESP_DEBUG_ERR_START_OFFSET

    seq             c3, r6[7:5], AETH_CODE_ACK
    //if ACK, ignore error counting
    bcf             [c3], error_done

    seq             c3, r6[7:5], AETH_CODE_RNR //BD Slot
    //else if RNR, set up the offset and jump
    bcf             [c3], atomic_incr

    add.c3          r3, r3, LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_TX_RNR_RETRY_ERR_OFFSET), 3 //BD Slot

    seq             c3, r6[7:5], AETH_CODE_NAK
    //else if not NAK, ignore error counting
    bcf             [!c3], error_done

    seq             c3, r6, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_SEQ_ERR) //BD Slot
    bcf             [c3], atomic_incr
    add.c3          r3, r3, LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_TX_PACKET_SEQ_ERR_OFFSET), 3 //BD Slot

    seq             c3, r6, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_INV_REQ)
    bcf             [c3], atomic_incr
    add.c3          r3, r3, LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_TX_REMOTE_INV_REQ_ERR_OFFSET), 3 //BD Slot

    seq             c3, r6, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR)
    bcf             [c3], atomic_incr
    add.c3          r3, r3, LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_TX_REMOTE_ACC_ERR_OFFSET), 3 //BD Slot

    seq             c3, r6, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_OP_ERR)
    bcf             [!c3], error_done
    add.c3          r3, r3, LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_TX_REMOTE_OPER_ERR_OFFSET), 3 //BD Slot

atomic_incr:
    ATOMIC_INC_VAL_1(r1, r3, r4, r5, 1)

error_done:
#endif

    nop.e
    nop


