#include "aq_rx.h"
#include "aqcb.h"

struct aq_rx_phv_t p;
struct aqcb_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define PHV_GLOBAL_COMMON_P phv_global_common

#define AQCB_TO_CQ_P t2_s2s_aqcb_to_cq_info

%%

    .param    rdma_aq_rx_cqcb_process

.align
rdma_aq_rx_aqcb_process:

    add            r1, r0, CAPRI_APP_DATA_RAW_FLAGS
    bnei           r1, AQ_RX_FLAG_RDMA_FEEDBACK, exit
    CAPRI_SET_TABLE_0_VALID(0) //BD Slot

process_feedback:

    seq            c1, k.rdma_aq_feedback_feedback_type, RDMA_AQ_FEEDBACK
    bcf            [!c1], exit

aq_feedback:
    
    #CQCB is loaded by req_rx and resp_rx flows in stage-5, table-2 for mutual
    #exclusive access. Do the same for aq_rx also

    phvwrpair      p.cqe.status[7:0], k.rdma_aq_feedback_status, p.cqe.error, k.rdma_aq_feedback_error
    phvwr          CAPRI_PHV_FIELD(AQCB_TO_CQ_P, cq_id), d.cq_id
    
    CAPRI_RESET_TABLE_2_ARG() //BD Slot


    phvwr       CAPRI_PHV_RANGE(AQCB_TO_CQ_P, cq_id, status), CAPRI_KEY_RANGE(rdma_aq_feedback,cq_num_sbit0_ebit7, status)
    phvwr       CAPRI_PHV_FIELD(AQCB_TO_CQ_P, error), k.rdma_aq_feedback_error

    add         r2, d.cqcb_addr, r0
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_rx_cqcb_process, r2)

    //          populate LIF
    add            r1, r0, offsetof(struct phv_, common_global_global_data)    
    phvwr CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, lif), CAPRI_RXDMA_INTRINSIC_LIF
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(AQCB_ADDR_SHIFT))

    //set dma_cmd_ptr in phv
    RXDMA_DMA_CMD_PTR_SET(AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_START_FLIT_CMD_ID) //Exit Slot


exit:
    phvwr.e       p.common.p4_intr_global_drop, 1   
    nop //Exit Slot

