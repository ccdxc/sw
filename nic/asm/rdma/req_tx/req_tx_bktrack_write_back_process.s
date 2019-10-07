#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s7_t0_k k;
struct sqcb0_t d;

#define IN_P t0_s2s_sqcb_write_back_info
#define IN_TO_S_P to_s7_bt_wb_info

#define K_NUM_SGES CAPRI_KEY_RANGE(IN_P, num_sges_sbit0_ebit2, num_sges_sbit3_ebit7)
#define K_CURRENT_SGE_ID CAPRI_KEY_RANGE(IN_P, current_sge_id_sbit0_ebit2, current_sge_id_sbit3_ebit7)
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit2, current_sge_offset_sbit27_ebit31)
#define K_SQ_C_INDEX CAPRI_KEY_RANGE(IN_P, sq_c_index_sbit0_ebit2, sq_c_index_sbit11_ebit15)
#define K_WQE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, wqe_addr_sbit0_ebit15, wqe_addr_sbit32_ebit63)
#define K_OP_TYPE         CAPRI_KEY_FIELD(IN_P, op_type)

%%

.align
req_tx_bktrack_write_back_process:
     // Pin write_back to stage 7
     mfspr         r1, spr_mpuid
     seq           c1, r1[4:2], STAGE_7
     bcf           [!c1], bubble_to_next_stage
     nop           // Branch Delay Slot

     bbeq          K_GLOBAL_FLAG(_error_disable_qp), 1, error_disable_exit
     nop

     bbeq          CAPRI_KEY_FIELD(IN_P, drop_phv), 1, exit
     tblwr         d.busy, 0 // Branch Delay Slot

     bbeq          CAPRI_KEY_FIELD(IN_P, drop_bktrack), 1, drop_bktrack
     nop           // Branch Delay Slot

     seq           c4, d.spec_enable, 1
     bcf           [c4], skip_in_progress_update
     tblwr         d.in_progress, CAPRI_KEY_FIELD(IN_P, in_progress) //BD-Slot
     tblwr         d.num_sges, K_NUM_SGES
     tblwr         d.current_sge_id, K_CURRENT_SGE_ID
     tblwr         d.current_sge_offset, K_CURRENT_SGE_OFFSET

skip_in_progress_update:
     tblwr         d.bktrack_in_progress, CAPRI_KEY_FIELD(IN_P, bktrack_in_progress)
    
     // Update read_req_adjust/msg_psn and spec_msg_psn.
     tblwr         d.read_req_adjust, K_CURRENT_SGE_OFFSET
     bbeq          CAPRI_KEY_FIELD(IN_P, in_progress), 0, skip_spec_psn_update
     tblwr.c4      d.spec_msg_psn, K_CURRENT_SGE_OFFSET //BD-slot. Reset spec_msg_psn

     // Decrement spec_msg_psn by 1. It will be incremented after bktrack during in-progress processing.
     add           r1, K_CURRENT_SGE_OFFSET, r0
     mincr         r1, 24, -1
     tblwr.c4      d.spec_msg_psn, r1

skip_spec_psn_update:
     tblwr         d.curr_wqe_ptr, K_WQE_ADDR

     seq           c2, CAPRI_KEY_FIELD(IN_P, bktrack_in_progress), 1
     bcf           [c2], update_spec_cindex
     // set SQ c_index to the backtracked value
     tblwr         SQ_C_INDEX, K_SQ_C_INDEX // Branch Delay Slot

     // Empty backtrack ring upon completion of bktracking. Also,
     // clear bktrack_in_progress in SQCB1 so that it can start
     // accepting response packets
     tblwr         SQ_BKTRACK_C_INDEX, SQ_BKTRACK_P_INDEX
     // Fake a dcqcn_rl_failure to force dcqcn stage to reset copies of
     // spec_cindex and msg_psn after backtrack.
     tblwr         d.dcqcn_rl_failure, 1

update_spec_cindex:
 
     // Set speculative cindex to next cindex so that speculative wqe 
     // processing can take place after backtrack 
     bbeq          CAPRI_KEY_FIELD(IN_P, in_progress), 0, exit
     tblwr         SPEC_SQ_C_INDEX, SQ_C_INDEX // Branch Delay Slot

     tblmincri     SPEC_SQ_C_INDEX,  d.log_num_wqes, 1

exit:
     phvwr.e  p.common.p4_intr_global_drop, 1
     CAPRI_SET_TABLE_0_VALID(0)

error_disable_exit:
#if !(defined (HAPS) || defined (HW))
    /*
     *  Empty bktrack ring  to satisfy model. Ideally, on error disabling
     *  we should just exit and be in the same state which caused the error.
     */
    tblwr          SQ_BKTRACK_C_INDEX, SQ_BKTRACK_P_INDEX
#endif

    // DMA commands for generating error-completion to RxDMA
    phvwr          p.rdma_feedback.feedback_type, RDMA_COMPLETION_FEEDBACK
    add            r1, r0, offsetof(struct req_tx_phv_t, p4_to_p4plus)
    phvwrp         r1, 0, CAPRI_SIZEOF_RANGE(struct req_tx_phv_t, p4_intr_global, p4_to_p4plus), r0
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_ERR_FEEDBACK)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, p4_intr_global, p4_to_p4plus, 2)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, rdma_feedback, rdma_feedback, 1)

    phvwrpair      p.p4_intr_global.tm_iport, TM_PORT_INGRESS, p.p4_intr_global.tm_oport, TM_PORT_DMA
    phvwrpair      p.p4_intr_global.tm_iq, 0, p.p4_intr_global.lif, K_GLOBAL_LIF
    SQCB0_ADDR_GET(r1)
    phvwrpair      p.p4_intr_rxdma.intr_qid, K_GLOBAL_QID, p.p4_intr_rxdma.intr_qstate_addr, r1
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_REQ_FEEDBACK_SPLITTER_OFFSET

    phvwrpair      p.p4_intr_rxdma.intr_qtype, K_GLOBAL_QTYPE, p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, REQ_RX_FLAG_RDMA_FEEDBACK
    phvwri.e       p.{p4_to_p4plus.table0_valid...p4_to_p4plus.table1_valid}, 0x3
    DMA_SET_END_OF_PKT_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], end

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7) // Branch Delay Slot
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS)

end:
   nop.e
   nop

drop_bktrack:
     tblwr    SQ_BKTRACK_C_INDEX, SQ_BKTRACK_P_INDEX
     tblwr    d.bktrack_in_progress, 0
     // Finally, clear bktrack_in_progress
     SQCB1_ADDR_GET(r1)
     add            r2, FIELD_OFFSET(sqcb1_t, bktrack_in_progress), r1
     memwr.b        r2, 0
     phvwr.e  p.common.p4_intr_global_drop, 1
     CAPRI_SET_TABLE_0_VALID(0)
