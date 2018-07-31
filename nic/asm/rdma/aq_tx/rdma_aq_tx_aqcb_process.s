#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aqcb_t d;
struct aq_tx_s0_t0_k k;

#define AQCB_TO_WQE_P t0_s2s_aqcb_to_wqe_info
    
%%

    .param      rdma_aq_tx_wqe_process
    
.align
rdma_aq_tx_aqcb_process:

    /*
     * Initially the plan is to implement only one admin command with busy flag
     * and later will be optimized further.
     * Not setting ci = pi so there will be many spurious phvs generated and get
     * dropped if busy fag set.
     */

    seq         c1, d.busy, 1
    bcf         [c1], drop
    nop

    tblwr       d.busy, 1

    // copy intrinsic to global
    add            r1, r0, offsetof(struct phv_, common_global_global_data) 

    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)

    //set dma_cmd_ptr in phv
    TXDMA_DMA_CMD_PTR_SET(AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_START_FLIT_CMD_ID)     
    /* Setup for the next wqe stage */

    CAPRI_RESET_TABLE_0_ARG()
    phvwr       CAPRI_PHV_FIELD(AQCB_TO_WQE_P, cq_num), d.cq_id

    // Compute WQE address
    add         r3, d.phy_base_addr, AQ_C_INDEX, AQ_WQE_T_LOG_SIZE_BYTES
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_wqe_process, r3)
    
    /* increment the cindex */
    tblmincri   AQ_C_INDEX, 24, 1

    nop.e
    nop
    
busy:
    
drop: 
    phvwr       p.common.p4_intr_global_drop, 1
    nop.e       
    nop         //Exit Slot
    
