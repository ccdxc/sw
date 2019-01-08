#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_s4_t2_k k;
struct sqcb1_t d;

#define IN_P t2_s2s_sqcb1_to_timer_expiry_info

#define K_REXMIT_PSN CAPRI_KEY_RANGE(IN_P, rexmit_psn_sbit0_ebit7, rexmit_psn_sbit16_ebit23)
%%

.align
req_rx_timer_expiry_process:
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_4
    bcf            [!c1], bubble_to_next_stage

    // If bktrack is in progress already, then no more bktracking
    // is needed. If rexmit_psn from TxDMA is not same in RxDMA,
    // then one or more acks have been processed by RxDMA after TxDMA
    // posted feedback Phv and before it was processed by RxDMA. So the
    // timeout is not valid anymore and no requests have to be retransmitted.
    // Drop the feedback phv and since timer was restarted in TxDMA upon expiry
    // (if responses are expected), there is no need to restart timer here
    seq            c1, d.bktrack_in_progress, 1 // Branch Delay Slot
    seq            c2, d.rexmit_psn, K_REXMIT_PSN
    bcf            [c1 | !c2], drop_feedback
   
post_bktrack_ring:
     // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_BKTRACK_DB) // Branch Delay Slot

    // dma_cmd - bktrack_ring db data
    PREPARE_DOORBELL_INC_PINDEX(K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, SQ_BKTRACK_RING_ID, r1, r2)
    phvwr          p.db_data2, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, db_data2, db_data2, r1)
    //DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

    // Set bktrack_in_progress in sqcb1 so that any response received while
    // bktracking can be dropped. This way Qstate is not updated by RxDMA
    // while TxDMA is updating as part of bktrack logic.
    tblwr          d.bktrack_in_progress, 1

end:
     CAPRI_SET_TABLE_2_VALID(0)

     nop.e
     nop

bubble_to_next_stage:
     seq           c1, r1[4:2], STAGE_3
     bcf           [!c1], exit
     SQCB1_ADDR_GET(r1)
     CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
     CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

exit:
     nop.e
     nop

drop_feedback:
     phvwr.e       p.common.p4_intr_global_drop, 1
     CAPRI_SET_TABLE_2_VALID(0)
