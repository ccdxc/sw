#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"

struct resp_rx_phv_t p;
struct resp_rx_s7_t3_k k;
struct rqcb5_t d;

#define IN_P    to_s7_stats_info

#define GLOBAL_FLAGS r7
#define RQCB5_ADDR   r3
#define STATS_PC     r6

#define MASK_16 16
#define MASK_32 32

#define K_FLAGS CAPRI_KEY_RANGE(IN_P, incr_recirc_drop, dup_rd_atomic_drop)
#define K_LIF_ERROR_ID_VLD CAPRI_KEY_FIELD(IN_P, lif_error_id_vld)
#define K_LIF_ERROR_ID CAPRI_KEY_RANGE(IN_P, lif_error_id_sbit0_ebit1, lif_error_id_sbit2_ebit3)

%%

.param  lif_stats_base

.align
resp_rx_stats_process:

    // Pin stats process to stage 7
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_7
    bcf              [!c1], bubble_to_next_stage

    crestore         [c7, c6, c5, c4], K_FLAGS, 0xf //BD Slot
    // c7-recirc_drop, c6-dup_wr_send, c5-dup_rd_atomic_bt, c4-dup_rd_atomic_drop

    tblmincri.c7     d.num_recirc_drop_pkts, MASK_16, 1
    tblmincri.c6     d.num_dup_wr_send_pkts, MASK_16, 1
    tblmincri.c5     d.num_dup_rd_atomic_bt_pkts, MASK_16, 1
    tblmincri.c4     d.num_dup_rd_atomic_drop_pkts, MASK_16, 1

    bcf              [c7 | c6 | c5 | c4], handle_error_lif_stats
    add              GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS //BD Slot

    tbladd           d.num_bytes, CAPRI_KEY_FIELD(to_s7_stats_info, pyld_bytes)
    tblmincri        d.num_pkts, MASK_32, 1

    crestore         [c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (RESP_RX_FLAG_RING_DBELL | RESP_RX_FLAG_ACK_REQ | RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA | RESP_RX_FLAG_READ_REQ)

    tblmincri.c6     d.num_ring_dbell, MASK_16, 1
    tblmincri.c5     d.num_ack_requested, MASK_16, 1
    tblmincri.c4     d.num_send_msgs_inv_rkey, MASK_16, 1
    tblmincri.c3     d.num_atomic_cswap_msgs, MASK_16, 1
    tblmincri.c2     d.num_atomic_fna_msgs, MASK_16, 1
    tblmincri.c1     d.num_read_req_msgs, MASK_16, 1

    // incr number of memory window invalidations
    // via send with invalidate
    seq              c7, CAPRI_KEY_FIELD(to_s7_stats_info, incr_mem_window_inv), 1
    tblmincri.c7     d.num_mem_window_inv, MASK_16, 1
    bcf              [c4 | c3 | c2 | c1], handle_lif_stats

    ARE_ALL_FLAGS_SET(c6, GLOBAL_FLAGS, RESP_RX_FLAG_IMMDT|RESP_RX_FLAG_SEND) //BD Slot
    tblmincri.c6     d.num_send_msgs_imm_data, MASK_16, 1
    ARE_ALL_FLAGS_SET(c6, GLOBAL_FLAGS, RESP_RX_FLAG_IMMDT|RESP_RX_FLAG_WRITE)
    tblmincri.c6     d.num_write_msgs_imm_data, MASK_16, 1

    //send messages without inv_rkey and imm_data
    crestore         [c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_SEND | RESP_RX_FLAG_MIDDLE | RESP_RX_FLAG_FIRST)
    //send & & !inv_rkey !imm_data & !middle & !first 
    setcf            c7, [!c6 & !c5 & c3 & !c2 & !c1]
    tblmincri.c7     d.num_send_msgs, MASK_16, 1

    //write messages without imm_data
    //write & !imm_data & !middle & !first 
    setcf            c7, [!c5 & c4 & !c2 & !c1]
    tblmincri.c7     d.num_write_msgs, MASK_16, 1

    IS_ANY_FLAG_SET(c6, GLOBAL_FLAGS, RESP_RX_FLAG_ONLY | RESP_RX_FLAG_FIRST | RESP_RX_FLAG_READ_REQ | RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA)
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

handle_error_lif_stats:

#ifndef GFT
    bbeq            K_LIF_ERROR_ID_VLD, 0, error_done

    addi            r1, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START //BD Slot
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT

    #lif resp error-id stats
    addi            r3, r2, LIF_STATS_RESP_DEBUG_ERR_START_OFFSET
    add             r3, r3, K_LIF_ERROR_ID, 3

    ATOMIC_INC_VAL_1(r1, r3, r4, r5, 1)

error_done:

#endif

    nop.e
    nop



bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], exit

    //invoke the same routine, but with valid d[]
    //using static config, stage-7/table-3 is set as memory_only - to improve on latency
    CAPRI_GET_TABLE_3_K(resp_rx_phv_t, r7) //BD Slot
    RQCB5_ADDR_GET(RQCB5_ADDR)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RQCB5_ADDR)

exit:
    nop.e
    nop


