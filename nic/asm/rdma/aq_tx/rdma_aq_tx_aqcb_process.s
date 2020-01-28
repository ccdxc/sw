
#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aqcb0_t d;
struct aq_tx_s0_t0_k k;

#define TO_WQE_INFO_P t0_s2s_aqcb_to_wqe_info
#define TO_S_FB_INFO_P to_s7_fb_stats_info
#define TO_S6_INFO_P   to_s6_info
#define TO_S3_INFO_P   to_s3_info
%%

    .param      rdma_aq_tx_wqe_process
    
.align
rdma_aq_tx_aqcb_process:

    .brbegin
    br          r7[MAX_AQ_DOORBELL_RINGS-1: 0]
    nop
    
    .brcase     0

        bbeq        d.ring_empty_sched_eval_done, 1, exit
        tblwr       d.ring_empty_sched_eval_done, 1 //BD Slot

        DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r2, r3) 

        phvwr.e       p.common.p4_intr_global_drop, 1
        nop         

    .brcase     1
        seq         c1, d.error, 1
        bcf         [c1], error
        seq         c2, d.first_pass, 1 //BD slot                                      
        seq         c1, d.token_id, d.next_token_id
        bcf         [!c1], exit 
        nop

        tblmincri   d.next_token_id, 8, 1 //BD Slot
        tblwr.f     d.ring_empty_sched_eval_done, 0 //BD Slot

        //Default   values
        phvwr       p.common.p4_intr_global_debug_trace, d.debug
        phvwr       p.proxy_cindex, d.{ring0.cindex}
        phvwrpair   p.first_pass, 1, p.token_id, d.token_id
        phvwr       CAPRI_PHV_FIELD(TO_S_FB_INFO_P, aq_cmd_done), 1
     
        // copy intrinsic to global
        add            r1, r0, offsetof(struct phv_, common_global_global_data) 

        CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
        CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
        CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)

        CAPRI_SET_FIELD2(TO_S6_INFO_P, cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR)

        CAPRI_SET_FIELD2(TO_S3_INFO_P, tx_psn_or_uplink_port, d.uplink_num)

        //set       dma_cmd_ptr in phv
        TXDMA_DMA_CMD_PTR_SET(AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_START_FLIT_CMD_ID)     
        /* Setup for the next wqe stage */

        DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_RDMA_BUSY)
        mfspr       r2, spr_tbladdr
        add         r2, r2, FIELD_OFFSET(aqcb0_t, ring0.cindex)
        DMA_HBM_PHV2MEM_SETUP_F(r6, proxy_cindex, token_id, r2)    
    
        CAPRI_RESET_TABLE_0_ARG()

        phvwr.!c2   CAPRI_PHV_FIELD(TO_WQE_INFO_P, map_count_completed), d.map_count_completed
        CAPRI_SET_FIELD2(TO_WQE_INFO_P, cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR)
        phvwr       CAPRI_PHV_FIELD(TO_S_FB_INFO_P, wqe_id), AQ_C_INDEX

        // Compute WQE address & encode
        add         r3, d.phy_base_addr, AQ_C_INDEX, AQ_WQE_T_LOG_SIZE_BYTES

        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_wqe_process, r3)

        nop.e
        nop
    
    .brend
    
error:
        DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r0, r2, r3)

exit: 
    CAPRI_SET_TABLE_0_VALID(0)

    phvwr.e       p.common.p4_intr_global_drop, 1
    nop         //Exit Slot
