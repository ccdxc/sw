#include "capri.h"
#include "cq_tx.h"
#include "cqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "types.h"

struct cq_tx_phv_t p;
struct cqcb_t d;
struct cq_tx_s0_t0_k k;

#define CQCB_TO_PT_P t0_s2s_cqcb_to_pt_info

#define TO_S5_P to_s5_sq_to_stage

%%

.align
rdma_cq_tx_cqcb_process:

    .brbegin
    brpri         r7[MAX_CQ_DOORBELL_RINGS-1: 0], [CQ_SARM_PRI, CQ_ARM_PRI, CQ_PRI]
    //set dma_cmd_ptr in phv
    TXDMA_DMA_CMD_PTR_SET(CQ_TX_DMA_CMD_START_FLIT_ID, CQ_TX_DMA_CMD_START_FLIT_CMD_ID) //BD Slot

    .brcase       CQ_RING_ID

        #set pindex = cindex to shut the ring
        tblwr     CQ_P_INDEX, CQ_C_INDEX
        b exit
        tblwr     d.ring_empty_sched_eval_done, 0 //BD Slot

    .brcase       CQ_ARM_RING_ID

        b         prepare_feedback
        tblwr     d.ring_empty_sched_eval_done, 0 //BD Slot


    .brcase       CQ_SARM_RING_ID

        b         prepare_feedback
        tblwr     d.ring_empty_sched_eval_done, 0 //BD Slot

    .brcase       MAX_CQ_DOORBELL_RINGS

        bbeq      d.ring_empty_sched_eval_done, 1, exit
        tblwr     d.ring_empty_sched_eval_done, 1 //BD Slot
    
        // ring doorbell to re-evaluate scheduler
        DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r2, r3)

        phvwr.e   p.common.p4_intr_global_drop, 1
        nop //Exit Slot
    
    .brend

prepare_feedback:

        #set arm cindex = pindex
        tblwr     CQ_ARM_C_INDEX, CQ_ARM_P_INDEX
        #set sarm cindex = pindex
        tblwr     CQ_SARM_C_INDEX, CQ_SARM_P_INDEX

        phvwr     p.rdma_feedback.feedback_type, RDMA_CQ_ARM_FEEDBACK 
        phvwr     p.rdma_feedback.arm.cindex, CQ_C_INDEX
        phvwr     p.rdma_feedback.arm.arm, r7[CQ_ARM_RING_ID:CQ_ARM_RING_ID] 
        phvwr     p.rdma_feedback.arm.sarm, r7[CQ_SARM_RING_ID:CQ_SARM_RING_ID]

        // get DMA cmd entry based on dma_cmd_index
        DMA_CMD_STATIC_BASE_GET(r6, CQ_TX_DMA_CMD_START_FLIT_ID, CQ_TX_DMA_CMD_RDMA_FEEDBACK)

        // plan is to go directly from txdma to rxdma
        // order of headers: p4_intr_global, p4_intr, p4_intr_rxdma, p4_to_p4plus, rdma_feedback
        // dma_cmd[0] : addr1 - p4_intr_global
        DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type, 2)
        phvwrpair p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_DMA

        // dma_cmd[0] : addr2 - p4_intr, p4_rxdma_intr, rdma_feedback
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, p4_intr, rdma_feedback, 1)

        phvwrpair p.p4_intr_rxdma.intr_qid, CAPRI_TXDMA_INTRINSIC_QID, p.p4_intr_rxdma.intr_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
        phvwr     p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
        phvwr     p.p4_to_p4plus.raw_flags, REQ_RX_FLAG_RDMA_FEEDBACK
        phvwri    p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET
        #phvwri    p.p4_to_p4plus.table0_valid, 1

        DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
        DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)

        nop.e
        nop //Exit Slot

exit:
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop //Exit Slot
