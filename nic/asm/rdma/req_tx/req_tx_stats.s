#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"

struct req_tx_phv_t p;
struct req_tx_s7_t3_k k;
struct sqcb4_t d;

#define IN_P    to_s7_stats_info

#define GLOBAL_FLAGS r7
#define SQCB4_ADDR   r3
#define STATS_PC     r6

#define MASK_16 16
#define MASK_32 32

#define K_LIF_ERROR_ID_VLD CAPRI_KEY_FIELD(IN_P, lif_error_id_vld)
#define K_LIF_ERROR_ID CAPRI_KEY_FIELD(IN_P, lif_error_id)
#define K_ERR_DIS_REASON_CODES CAPRI_KEY_RANGE(IN_P, qp_err_disabled, qp_err_dis_rsvd_sbit19_ebit25)
#define K_QP_ERR_DISABLED      CAPRI_KEY_FIELD(IN_P, qp_err_disabled)

%%

.param  lif_stats_base

.align
req_tx_stats_process:

    // Pin stats process to stage 7
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_7
    bcf              [!c1], bubble_to_next_stage

    add              GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS //BD Slot

    bbeq             K_QP_ERR_DISABLED, 1, handle_error_stats

    seq              c1, CAPRI_KEY_FIELD(IN_P, sq_drain), 1 //BD Slot
    tblmincri.c1.e   d.num_sq_drains, MASK_16, 1

    // if sqcb2_cnp_process, branch to dcqcn_cnp_sent_stats .
    bbeq             CAPRI_KEY_FIELD(IN_P, dcqcn_cnp_sent), 1, dcqcn_cnp_sent_stats

    seq              c7, CAPRI_KEY_FIELD(IN_P, rp_num_byte_threshold_db), 1
    tblmincri.c7     d.rp_num_byte_threshold_db, MASK_16, 1

    bbeq             CAPRI_KEY_FIELD(IN_P, npg), 1, handle_npg_stats
    crestore         [c4, c3, c2, c1], GLOBAL_FLAGS, (REQ_TX_FLAG_INV_RKEY | REQ_TX_FLAG_ATOMIC_FNA | REQ_TX_FLAG_ATOMIC_CSWAP | REQ_TX_FLAG_READ_REQ) //BD Slot

    bbeq             CAPRI_KEY_FIELD(IN_P, timeout), 1, handle_timeout_stats
    nop

    tbladd           d.num_bytes, CAPRI_KEY_FIELD(to_s7_stats_info, pyld_bytes)
    tblmincri        d.num_pkts, MASK_32, 1

    tblmincri.c4     d.num_send_msgs_inv_rkey, MASK_16, 1
    tblmincri.c3     d.num_atomic_fna_msgs, MASK_16, 1
    tblmincri.c2     d.num_atomic_cswap_msgs, MASK_16, 1
    tblmincri.c1     d.num_read_req_msgs, MASK_16, 1

    bcf              [c3 | c2 | c1], handle_lif_stats

    ARE_ALL_FLAGS_SET(c6, GLOBAL_FLAGS, REQ_TX_FLAG_IMMDT|REQ_TX_FLAG_SEND) //BD Slot
    tblmincri.c6     d.num_send_msgs_imm_data, MASK_16, 1
    ARE_ALL_FLAGS_SET(c6, GLOBAL_FLAGS, REQ_TX_FLAG_IMMDT|REQ_TX_FLAG_WRITE)
    tblmincri.c6     d.num_write_msgs_imm_data, MASK_16, 1

    //send messages without inv_rkey and imm_data
    crestore         [c7, c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (REQ_TX_FLAG_INLINE | REQ_TX_FLAG_INV_RKEY | REQ_TX_FLAG_IMMDT | REQ_TX_FLAG_WRITE | REQ_TX_FLAG_SEND | REQ_TX_FLAG_MIDDLE | REQ_TX_FLAG_FIRST)
    tblmincri.c7     d.num_inline_req, MASK_16, 1
    //send & !inv_rkey & !imm_data & !middle & !first 
    setcf            c7, [!c6 & !c5 & c3 & !c2 & !c1]
    tblmincri.c7     d.num_send_msgs, MASK_16, 1

    //write messages without imm_data
    //write & !imm_data & !middle & !first 
    setcf            c7, [!c5 & c4 & !c2 & !c1]
    tblmincri.c7     d.num_write_msgs, MASK_16, 1

    IS_ANY_FLAG_SET(c6, GLOBAL_FLAGS, REQ_TX_FLAG_ONLY | REQ_TX_FLAG_FIRST | REQ_TX_FLAG_READ_REQ | REQ_TX_FLAG_ATOMIC_CSWAP | REQ_TX_FLAG_ATOMIC_FNA)
    tblwr.c6         d.num_pkts_in_cur_msg, 1
    tblmincri.!c6    d.num_pkts_in_cur_msg, MASK_16, 1

    //peak
    add              r4, r0, d.max_pkts_in_any_msg
    sslt             c6, r4, d.num_pkts_in_cur_msg, r0
    tblwr.c6         d.max_pkts_in_any_msg, d.num_pkts_in_cur_msg

handle_lif_stats:

#ifndef GFT

    addi            r1, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT

    #uc bytes and packets
    addi            r3, r2, LIF_STATS_TX_RDMA_UCAST_BYTES_OFFSET

    ATOMIC_INC_VAL_2(r1, r3, r4, r5, CAPRI_KEY_FIELD(to_s7_stats_info, pyld_bytes), 1)

#endif

done:

    nop.e
    nop

handle_npg_stats:
    crestore      [c5, c4, c3, c2, c1], CAPRI_KEY_RANGE(IN_P, npg, npg_local_inv), 0x1F
    tblmincri.c4  d.num_npg_bindmw_t1_req, MASK_16, 1
    tblmincri.c3  d.num_npg_bindmw_t2_req, MASK_16, 1
    tblmincri.c2  d.num_npg_frpmr_req, MASK_16, 1
    tblmincri.c1  d.num_npg_local_inv_req, MASK_16, 1
    tblmincri.e   d.num_npg_req, MASK_32, 1
    nop
 
handle_timeout_stats:
    crestore      [c3, c2, c1], CAPRI_KEY_RANGE(IN_P, timeout, timeout_rnr), 0x7
    tblmincri.c2  d.num_timeout_local_ack, MASK_16, 1
    tblmincri.c1  d.num_timeout_rnr, MASK_16, 1

    nop.e
    nop   

handle_error_stats:
    tblor.e          d.{qp_err_disabled...qp_err_dis_rsvd}, K_ERR_DIS_REASON_CODES
    nop

dcqcn_cnp_sent_stats:
    seq              c2, CAPRI_KEY_FIELD(IN_P, np_cnp_sent), 1
    tblmincri.c2     d.np_cnp_sent, MASK_16, 1

    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], exit

    //invoke the same routine, but with valid d[]
    //using static config, stage-7/table-3 is set as memory_only - to improve on latency
    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7) //BD Slot
    SQCB4_ADDR_GET(SQCB4_ADDR)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, SQCB4_ADDR)

exit:
    nop.e
    nop

