#include "aq_rx.h"
#include "aqcb.h"

struct aq_rx_phv_t p;
struct aq_rx_s1_t3_k k;

#define PHV_GLOBAL_COMMON_P phv_global_common

#define IN_P t3_s2s_aqcb_to_wqe_info
#define IN_TO_S_P to_s1_info

#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_RANGE(IN_TO_S_P, rqcb_base_addr_hi_sbit0_ebit2, rqcb_base_addr_hi_sbit19_ebit23)

#define K_RQ_MAP_COUNT CAPRI_KEY_FIELD(IN_P, rq_map_count)

%%

    .param      dummy
    .param      rdma_rq_tx_stage0
    
.align
rdma_aq_rx_wqe_process:

    //Setup     DMA for RQ PT

    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CREATE_QP_RQPT)
    add         r4, r0, K_RQ_MAP_COUNT, CAPRI_LOG_SIZEOF_U64

    PT_BASE_ADDR_GET3(r4) 
    add         r3, r4, k.t3_s2s_aqcb_to_wqe_info_rq_tbl_index, CAPRI_LOG_SIZEOF_U64
    phvwr       p.rqcb0.pt_base_addr, r3
    phvwr       p.rqcb1.pt_base_addr, r3

    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r4, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr)
    DMA_HBM_MEM2MEM_DST_SETUP(r6, r4, r3)

    // setup DMA for SQCB
    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CREATE_QP_CB)        
    CAPRI_GET_TABLE_0_K(aq_rx_phv_t, r7)
    RQCB_ADDR_GET(r1, k.t3_s2s_aqcb_to_wqe_info_rq_id, K_RQCB_BASE_ADDR_HI)

    DMA_PHV2MEM_SETUP(r6, c1, rqcb0, rqcb2, r1)
    
