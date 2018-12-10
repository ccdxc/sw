#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"


struct resp_rx_phv_t p;
struct dcqcn_cb_t d;
struct resp_rx_s1_t2_k k;

#define TO_S_WB1_P to_s5_wb1_info

%%
    .param    resp_rx_rqcb1_write_back_mpu_only_process
    .param      lif_stats_base

.align
resp_rx_dcqcn_cnp_process:

    tblmincri   d.num_cnp_rcvd, 8, 1 //num_cnp_recvd is 8-bit field.
    // Ring doorbell to cut-rate and reset dcqcn params. Also drop PHV since further 
    // processing will be done in TxDMA rate-compute-ring.
    DOORBELL_INC_PINDEX(K_GLOBAL_LIF,  K_GLOBAL_QTYPE, K_GLOBAL_QID, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)
    phvwr       p.common.p4_intr_global_drop, 1
    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

handle_lif_stats:

#ifndef GFT

    addi            r1, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT

    #uc bytes and packets
    addi            r3, r2, LIF_STATS_RX_RDMA_CNP_PACKETS_OFFSET

    ATOMIC_INC_VAL_1(r1, r3, r4, r5, 1)
#endif

    nop.e
    nop

