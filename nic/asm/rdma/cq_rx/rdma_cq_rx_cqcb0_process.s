#include "cq_rx.h"
#include "cqcb.h"

struct cq_rx_phv_t p;
struct cqcb_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define PHV_GLOBAL_COMMON_P phv_global_common

#define CQCB0_TO_CQ_P t2_s2s_cqcb0_to_cq_info

%%

    .param    rdma_cq_rx_cqcb_process

.align
rdma_cq_rx_cqcb0_process:

    add            r1, r0, CAPRI_APP_DATA_RAW_FLAGS
    bnei           r1, REQ_RX_FLAG_RDMA_FEEDBACK, exit
    CAPRI_SET_TABLE_0_VALID(0) //BD Slot

process_feedback:

    seq            c1, k.rdma_cq_feedback_feedback_type, RDMA_CQ_ARM_FEEDBACK
    bcf            [!c1], exit

cq_feedback:
    
    CAPRI_RESET_TABLE_2_ARG() //BD Slot

    phvwr          CAPRI_PHV_RANGE(CQCB0_TO_CQ_P, cindex, sarm), k.{rdma_cq_feedback_cindex_sbit0_ebit7...rdma_cq_feedback_sarm}
    phvwr          CAPRI_PHV_FIELD(CQCB0_TO_CQ_P, cq_id), CAPRI_RXDMA_INTRINSIC_QID

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, rdma_cq_rx_cqcb_process, r0)

    // populate global fields
    phvwrpair.e CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, lif), \
                CAPRI_RXDMA_INTRINSIC_LIF, \
                CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, cb_addr), \
                CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(CQCB_ADDR_SHIFT)

    //set dma_cmd_ptr in phv
    RXDMA_DMA_CMD_PTR_SET(CQ_RX_DMA_CMD_START_FLIT_ID, CQ_RX_DMA_CMD_START_FLIT_CMD_ID) //Exit Slot


exit:
    phvwr.e       p.common.p4_intr_global_drop, 1   
    nop

