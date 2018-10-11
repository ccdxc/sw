#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aqcb_t d;
struct aq_tx_s0_t0_k k;

#define TO_FEEDBACK_P to_s6_info
    
%%

    .param      rdma_aq_tx_wqe_process
    
.align
rdma_aq_tx_aqcb_process:

    /*
     * Initially the plan is to implement only one admin command with busy flag
     * and later will be optimized further.
     * Not setting ci = pi so there will be many spurious phvs generated and get
     * dropped if busy flag set.
     */

    .brbegin
    br          r7[MAX_AQ_DOORBELL_RINGS-1: 0]
        nop         
    
    .brcase     0
        DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r2, r3) 
        phvwr       p.common.p4_intr_global_drop, 1
        nop.e       
        nop         

    .brcase     1
        seq         c1, d.busy, 1           
        bcf         [c1], drop              
        tblwr       d.busy, 1 //BD Slot

        // copy intrinsic to global
        add            r1, r0, offsetof(struct phv_, common_global_global_data) 

        CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
        CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
        CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)

        //set       dma_cmd_ptr in phv
        TXDMA_DMA_CMD_PTR_SET(AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_START_FLIT_CMD_ID)     
        /* Setup for the next wqe stage */

        CAPRI_RESET_TABLE_0_ARG()

        phvwr       CAPRI_PHV_FIELD(TO_FEEDBACK_P, cq_num), d.cq_id

        // Compute WQE address & encode
        add         r3, d.phy_base_addr, AQ_C_INDEX_HX, AQ_WQE_T_LOG_SIZE_BYTES

        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_wqe_process, r3)

        /* increment the cindex */
        tblmincri   AQ_C_INDEX_HX, d.log_num_wqes, 1

        nop.e
        nop

    .brend
    
busy:
drop:
    CAPRI_SET_TABLE_0_VALID(0)

    phvwr       p.common.p4_intr_global_drop, 1
    nop.e       
    nop         //Exit Slot
    
