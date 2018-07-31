#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aqwqe_t d;
struct aq_tx_s1_t0_k k;

#define IN_P t0_s2s_aqcb_to_wqe_info

#define K_COMMON_GLOBAL_QID CAPRI_KEY_RANGE(phv_global_common, qid_sbit0_ebit4, qid_sbit21_ebit23)
#define K_COMMON_GLOBAL_QTYPE CAPRI_KEY_FIELD(phv_global_common, qtype)
#define K_CQ_NUM CAPRI_KEY_FIELD(IN_P, cq_num)
    
%%

.align
rdma_aq_tx_wqe_process:

    add         r1, r0, d.op  //BD-slot

    .brbegin
    br          r1[3:0]
    nop         //BD slot

    .brcase     AQ_OP_TYPE_NOP
        b           prepare_feedback
        nop
    .brcase     AQ_OP_TYPE_STATS_HDRS
        b           exit
        nop
    .brcase     AQ_OP_TYPE_STATS_VALS
        b           exit
        nop
    .brcase     AQ_OP_TYPE_REG_MR
        b           exit
        nop
    .brcase     AQ_OP_TYPE_DEREG_MR
        b           exit
        nop
    .brcase     AQ_OP_TYPE_CREATE_CQ
        b           exit
        nop
    .brcase     AQ_OP_TYPE_RESIZE_CQ
        b           exit
        nop
    .brcase     AQ_OP_TYPE_DESTROY_CQ
        b           exit
        nop
    .brcase     AQ_OP_TYPE_CREATE_QP
        b           exit
        nop
    .brcase     AQ_OP_TYPE_MODIFY_QP
        b           exit
        nop
    .brcase     AQ_OP_TYPE_QUERY_QP
        b           exit
        nop
    .brcase     AQ_OP_TYPE_DESTROY_QP
        b           exit
        nop
    .brcase     12
        b           exit
        nop
    .brcase     13
        b           exit
        nop
    .brcase     14
        b           exit
        nop
    .brcase     15
        b           exit
        nop

    .brend

prepare_feedback:

    phvwr       p.rdma_feedback.feedback_type, RDMA_AQ_FEEDBACK
    phvwr       p.rdma_feedback.aq_completion.status, 0
    phvwr       p.rdma_feedback.aq_completion.cq_num, K_CQ_NUM
    
    //get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_RDMA_FEEDBACK)

    /* plan is to go directly from txdma to rxdma
     * order of headers: p4_intr_global, p4_intr, p4_intr_rxdma, p4_to_p4plus,
     * rdma_feedback dma_cmd[0] : addr1 - p4_intr_global
     */
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type, 2)
    phvwrpair   p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_DMA

    // dma_cmd[0] : addr2 - p4_intr, p4_rxdma_intr, rdma_feedback
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, p4_intr, rdma_feedback, 1)

    phvwrpair   p.p4_intr_rxdma.intr_qid, K_COMMON_GLOBAL_QID, p.p4_intr_rxdma.intr_qtype, K_COMMON_GLOBAL_QTYPE
    phvwr       p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwr       p.p4_to_p4plus.raw_flags, CQ_RX_FLAG_RDMA_FEEDBACK
    phvwri      p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET
    
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)
    
    nop.e
    nop         //Exit Slot
                                        
exit: 
    phvwr       p.common.p4_intr_global_drop, 1
    nop.e       
    nop         //Exit Slot
    
