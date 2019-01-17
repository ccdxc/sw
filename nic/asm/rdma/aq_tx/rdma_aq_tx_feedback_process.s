#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aq_tx_s7_t0_k k;
struct aqcb0_t d;
    
#define IN_TO_S_P to_s7_fb_stats_info
    
#define K_COMMON_GLOBAL_QID CAPRI_KEY_RANGE(phv_global_common, qid_sbit0_ebit4, qid_sbit21_ebit23)
#define K_COMMON_GLOBAL_QTYPE CAPRI_KEY_FIELD(phv_global_common, qtype)
#define K_WQE_ID  CAPRI_KEY_RANGE(IN_TO_S_P, wqe_id_sbit0_ebit5, wqe_id_sbit14_ebit15)
#define K_CB_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, cb_addr_sbit0_ebit31, cb_addr_sbit32_ebit33)

#define K_AQ_CMD_DONE CAPRI_KEY_FIELD(IN_TO_S_P, aq_cmd_done)
%%

    .param      rdma_aq_tx_stats_process
    
.align
rdma_aq_tx_feedback_process:

    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_7
    bcf         [!c1], bubble_to_next_stage

    // OP_TYPE, ERROR and STATUS is set in prior stages

    seq         c2, K_AQ_CMD_DONE, 1

    bcf         [!c2], multi_pass
    add         r2, r0, K_WQE_ID  //BD slot
    
    phvwr       p.rdma_feedback.feedback_type, RDMA_AQ_FEEDBACK  
    phvwr       p.rdma_feedback.aq_completion.wqe_id, K_WQE_ID

    mincr       r2, d.log_num_wqes, 1
    phvwr       p.proxy_cindex, r2[15:0].hx
    
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
    seq         c1, r1[4:2], STAGE_6
    bcf         [!c1], exit

    CAPRI_GET_TABLE_0_K(aq_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, K_CB_ADDR)

    //Get AQCB1 addr
    add r1, K_CB_ADDR, 1, LOG_CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_stats_process, r1)
    
exit:
    nop.e
    nop

multi_pass:
    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_RDMA_BUSY)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)
    nop.e
    nop
    
