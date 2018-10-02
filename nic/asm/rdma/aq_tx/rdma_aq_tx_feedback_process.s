#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aq_tx_s6_t0_k k;

#define IN_TO_S_P to_s6_info
    
#define K_COMMON_GLOBAL_QID CAPRI_KEY_RANGE(phv_global_common, qid_sbit0_ebit4, qid_sbit21_ebit23)
#define K_COMMON_GLOBAL_QTYPE CAPRI_KEY_FIELD(phv_global_common, qtype)
#define K_CQ_NUM CAPRI_KEY_FIELD(IN_TO_S_P, cq_num)
    
%%

    .param      dummy
    
.align
rdma_aq_tx_feedback_process:

    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], bubble_to_next_stage

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

    //Needed these as slit 5 overlaps with t3_s2s
    phvwri      p.p4_to_p4plus.table0_valid, 0
    phvwri      p.p4_intr.recirc, 0

    phvwrpair   p.p4_intr_rxdma.intr_qid, K_COMMON_GLOBAL_QID, p.p4_intr_rxdma.intr_qtype, K_COMMON_GLOBAL_QTYPE
    phvwr       p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwr       p.p4_to_p4plus.raw_flags, AQ_RX_FLAG_RDMA_FEEDBACK
    phvwri      p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_AQ_FEEDBACK_SPLITTER_OFFSET

    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)

    CAPRI_SET_TABLE_0_VALID(0)
    nop.e
    nop

bubble_to_next_stage:

    CAPRI_GET_TABLE_0_K(aq_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, r0)

exit:
    nop.e
    nop
