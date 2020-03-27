#include "capri.h"
#include "req_rx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"


struct req_rx_phv_t p;
struct req_rx_s7_t3_k k;
struct sqcb5_t d;

#define IN_P    to_s7_stats_info

#define GLOBAL_FLAGS r7
#define SQCB5_ADDR   r3
#define STATS_PC     r6

#define MASK_16 16
#define MASK_32 32

#define K_LIF_CQE_ERROR_ID_VLD CAPRI_KEY_FIELD(IN_P, lif_cqe_error_id_vld)
#define K_LIF_ERROR_ID_VLD CAPRI_KEY_FIELD(IN_P, lif_error_id_vld)
#define K_LIF_ERROR_ID CAPRI_KEY_FIELD(IN_P, lif_error_id)
#define K_ERR_DIS_REASON_CODES CAPRI_KEY_RANGE(IN_P, qp_err_disabled, qp_err_dis_rsvd_sbit10_ebit15)
#define K_QP_ERR_DISABLED      CAPRI_KEY_FIELD(IN_P, qp_err_disabled)
#define K_FLAGS CAPRI_KEY_RANGE(IN_P, table_error, table_resp_error)

#define K_MAX_RECIRC_CNT_ERR CAPRI_KEY_FIELD(IN_P, max_recirc_cnt_err) 
#define K_RECIRC_REASON CAPRI_KEY_FIELD(IN_P, recirc_reason)
#define K_RECIRC_BTH_OPCODE CAPRI_KEY_RANGE(IN_P, recirc_bth_opcode_sbit0_ebit3, recirc_bth_opcode_sbit4_ebit7)
#define K_RECIRC_BTH_PSN CAPRI_KEY_RANGE(IN_P, recirc_bth_psn_sbit0_ebit3, recirc_bth_psn_sbit20_ebit23) 

%%

.param  lif_stats_base

.align
req_rx_stats_process:

    // Pin stats process to stage 7
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_7
    bcf              [!c1], bubble_to_next_stage

    add              GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS //BD Slot

    bbeq             K_MAX_RECIRC_CNT_ERR, 1, max_recirc_cnt_err
    tbladd           d.np_ecn_marked_packets, CAPRI_KEY_FIELD(to_s7_stats_info, np_ecn_marked_packets)  // BD Slot

    bbeq             K_LIF_ERROR_ID_VLD, 1, handle_error_lif_stats
    tblor            d.{qp_err_disabled...qp_err_dis_rsvd}, K_ERR_DIS_REASON_CODES //BD Slot

    bbeq             K_QP_ERR_DISABLED, 1, handle_error_stats

    crestore         [c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (REQ_RX_FLAG_RDMA_FEEDBACK | REQ_RX_FLAG_ATOMIC_AETH | REQ_RX_FLAG_ACK | REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_FIRST | REQ_RX_FLAG_ONLY) // BD Slot

    bcf              [c6], handle_lif_stats
    tblmincri.c6     d.num_feedback, MASK_16, 1 //BD Slot

    tbladd           d.num_bytes, CAPRI_KEY_FIELD(to_s7_stats_info, pyld_bytes)
    tblmincri        d.num_pkts, MASK_32, 1

    tblmincri.c5     d.num_atomic_ack, MASK_16, 1
    tblmincri.c4     d.num_ack, MASK_16, 1

    bcf              [c5 | c4], handle_lif_stats
    setcf.c3         c3, [c2 | c1] //BD Slot

    tblmincri        d.num_read_resp_pkts, MASK_32, 1
    tblmincri.c3     d.num_read_resp_msgs, MASK_16, 1

    //ignoring this update for ACK and ATOMIC_ACK and tracking only for read responses
    IS_ANY_FLAG_SET(c6, GLOBAL_FLAGS, REQ_RX_FLAG_ONLY | REQ_RX_FLAG_FIRST)
    tblwr.c6         d.num_pkts_in_cur_msg, 1
    tblmincri.!c6    d.num_pkts_in_cur_msg, MASK_16, 1

    //peak
    add              r4, r0, d.max_pkts_in_any_msg
    sslt             c6, r4, d.num_pkts_in_cur_msg, r0
    tblwr.c6         d.max_pkts_in_any_msg, d.num_pkts_in_cur_msg

handle_lif_stats:

#ifndef GFT

    addi            r1, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT

    #uc bytes and packets
    addi            r3, r2, LIF_STATS_RX_RDMA_UCAST_BYTES_OFFSET

    ATOMIC_INC_VAL_2(r1, r3, r4, r5, CAPRI_KEY_FIELD(to_s7_stats_info, pyld_bytes), 1)

#endif

done:

    nop.e
    nop


max_recirc_cnt_err:
    //if max_recirc_cnt_err bit is already set, do not overwrite the info
    bbeq            d.max_recirc_cnt_err, 1, done
    //a packet which went thru too many recirculations had to be terminated and qp had to                
    //be put into error disabled state. The recirc reason, opcode, the psn of the packet etc.            
    //are remembered for further debugging.
    tblwr           d.max_recirc_cnt_err, 1     //BD Slot
    tblwr           d.recirc_reason, K_RECIRC_REASON
    tblwr.e         d.recirc_bth_opcode, K_RECIRC_BTH_OPCODE
    tblwr           d.recirc_bth_psn, K_RECIRC_BTH_PSN  //Exit Slot 

handle_error_lif_stats:

#ifndef GFT
    bbeq            K_LIF_ERROR_ID_VLD, 0, error_done

    addi            r1, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START //BD Slot
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT

    #lif req error-id stats
    addi            r3, r2, LIF_STATS_REQ_DEBUG_ERR_START_OFFSET
    add             r3, r3, K_LIF_ERROR_ID, 3

    ATOMIC_INC_VAL_1(r1, r3, r4, r5, 1)

    bbeq            K_LIF_CQE_ERROR_ID_VLD, 0, error_done

    #lif CQE error-id stat
    addi            r3, r2, LIF_STATS_REQ_DEBUG_ERR_START_OFFSET
    add             r3, r3, LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_CQE_ERR_OFFSET), 3

    ATOMIC_INC_VAL_1(r1, r3, r4, r5, 1)

error_done:
#endif

    nop.e
    nop

handle_error_stats:
    nop.e
    nop


bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], exit

    //invoke the same routine, but with valid d[]
    //using static config, stage-7/table-3 is set as memory_only - to improve on latency
    CAPRI_GET_TABLE_3_K(req_rx_phv_t, r7) //BD Slot
    SQCB5_ADDR_GET(SQCB5_ADDR)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, SQCB5_ADDR)

exit:
    nop.e
    nop


