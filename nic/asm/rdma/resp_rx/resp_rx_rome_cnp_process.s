#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"


struct resp_rx_phv_t p;
struct rome_sender_cb_t d;
struct resp_rx_s1_t2_k k;


%%
    .param    resp_rx_rqcb1_write_back_mpu_only_process
    .param      lif_stats_base

.align
resp_rx_rome_cnp_process:

handle_lif_stats:

#ifndef GFT

    addi            r1, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    addi            r2, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             r2, r2, K_GLOBAL_LIF, LIF_STATS_SIZE_SHIFT

    #rx cnp packets
    addi            r3, r2, LIF_STATS_RX_RDMA_CNP_PACKETS_OFFSET

    ATOMIC_INC_VAL_1(r1, r3, r4, r5, 1)
#endif

    nop.e
    nop
