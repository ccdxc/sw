#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"


struct resp_rx_phv_t p;
struct dcqcn_cb_t d;
struct resp_rx_s3_t2_k k;

#define IN_TO_S_P   to_s3_dcqcn_info
#define TO_S_WB1_P to_s5_wb1_info
#define TO_S_STATS_INFO_P to_s7_stats_info

#define K_MIN_TIME_BTWN_CNPS CAPRI_KEY_RANGE(IN_TO_S_P, min_time_btwn_cnps_sbit0_ebit3, min_time_btwn_cnps_sbit28_ebit31)

// r4 is pre-loaded with cur timestamp. Use r4 for CUR_TIMESTAMP.
// NOTE: Feeding timestamp from dcqcn_cb on model since model doesn't have timestamps.

#if defined (HAPS) || defined (HW)
#define CUR_TIMESTAMP r4
#else
#define CUR_TIMESTAMP d.cur_timestamp
#endif

%%
    .param    resp_rx_rqcb1_write_back_mpu_only_process
    .param    lif_stats_base
    .param    rdma_num_clock_ticks_per_us

.align
resp_rx_dcqcn_cnp_process:

    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_3
    bcf         [!c1], bubble_to_next_stage
    nop    // BD Slot

    // r1: min ticks between cnps
    mul         r1, K_MIN_TIME_BTWN_CNPS, rdma_num_clock_ticks_per_us
    sub         r2, CUR_TIMESTAMP, d.last_cnp_timestamp
    blt         r2, r1, handle_lif_stats
    phvwr       CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, rp_cnp_processed), 1     // BD Slot

    // Store cur-timestamp in cb
    tblwr       d.last_cnp_timestamp, CUR_TIMESTAMP
    tblmincri   d.num_cnp_rcvd, 8, 1 //num_cnp_recvd is 8-bit field
    // Ring doorbell to cut-rate and reset dcqcn params. Also drop PHV since further 
    // processing will be done in TxDMA rate-compute-ring.
    DOORBELL_INC_PINDEX(K_GLOBAL_LIF,  K_GLOBAL_QTYPE, K_GLOBAL_QID, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)

handle_lif_stats:
    phvwr       p.common.p4_intr_global_drop, 1
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)
    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1

#ifndef GFT

    addi            r1, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT

    #rx cnp packets
    addi            r3, r2, LIF_STATS_RX_RDMA_CNP_PACKETS_OFFSET

    ATOMIC_INC_VAL_1(r1, r3, r4, r5, 1)
#endif

exit:
    nop.e
    nop

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_2
    bcf         [!c1], exit
    CAPRI_GET_TABLE_2_K(resp_rx_phv_t, r7)  // BD Slot
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_E(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS)
    nop
