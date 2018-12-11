#include "aq_rx.h"
#include "aqcb.h"
#include "types.h"

struct aq_rx_phv_t p;
struct aq_rx_s1_t3_k k;

#define PHV_GLOBAL_COMMON_P phv_global_common

#define IN_P t3_s2s_aqcb_to_wqe_info
#define IN_TO_S_P to_s1_info

#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, rqcb_base_addr_hi)

#define K_RQ_MAP_COUNT CAPRI_KEY_FIELD(IN_P, rq_map_count)
#define K_RQ_CMB CAPRI_KEY_FIELD(IN_P, rq_cmb)

%%

.align
rdma_aq_rx_wqe_process:

    //Setup     DMA for RQ PT

    PT_BASE_ADDR_GET2(r3) 
    add         r3, r3, k.t3_s2s_aqcb_to_wqe_info_rq_tbl_index, CAPRI_LOG_SIZEOF_U64
    srl         r5, r3, CAPRI_LOG_SIZEOF_U64
    phvwr       p.rqcb0.pt_base_addr, r5
    phvwr       p.rqcb1.pt_base_addr, r5

    add         r4, r0, K_RQ_MAP_COUNT, CAPRI_LOG_SIZEOF_U64
    beqi        r4, 1<<CAPRI_LOG_SIZEOF_U64, qp_skip_dma_pt
    crestore    [c1], K_RQ_CMB, 0x1 //BD Slot

    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CREATE_QP_RQPT_SRC)

    or r2, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr, 0x1, 63
    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r4, r2)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CREATE_QP_RQPT_DST)
    DMA_HBM_MEM2MEM_DST_SETUP(r6, r4, r3)

    b           qp_no_skip_dma_pt
    nop

qp_skip_dma_pt:

    phvwr.c1    p.rqcb0.hbm_rq_base_addr, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr[33:HBM_RQ_BASE_ADDR_SHIFT]
    phvwr.c1    p.rqcb1.hbm_rq_base_addr, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr[33:HBM_RQ_BASE_ADDR_SHIFT]
    phvwr.c1    p.rqcb0.rq_in_hbm, 1
    phvwr.c1    p.rqcb1.rq_in_hbm, 1
    //copy      the phy address of a single page directly.
    //TODO: how     do we ensure this memwr is completed by the time we generate CQ for admin cmd.
    //or          r2, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr, 0x1, 63    
    memwr.!c1.dx    r3, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr

qp_no_skip_dma_pt: 
    
    // setup DMA for SQCB
    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CREATE_QP_CB)        
    RQCB_ADDR_GET(r1, k.t3_s2s_aqcb_to_wqe_info_rq_id, K_RQCB_BASE_ADDR_HI)

    DMA_PHV2MEM_SETUP(r6, c1, rqcb0, rqcb2, r1)

    CAPRI_SET_TABLE_3_VALID(0)          
    nop.e
    nop
