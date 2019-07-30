#include "aq_rx.h"
#include "aqcb.h"
#include "types.h"

struct aq_rx_phv_t p;
struct aq_rx_s1_t3_k k;

#define PHV_GLOBAL_COMMON_P phv_global_common

#define IN_P t3_s2s_aqcb_to_wqe_info
#define IN_TO_S_P to_s1_info

#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, rqcb_base_addr_hi)
#define K_PREFETCH_BASE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, prefetch_pool_base_addr_page_id_sbit0_ebit15, prefetch_pool_base_addr_page_id_sbit16_ebit21)
#define K_LOG_PREFETCH_ENTRIES CAPRI_KEY_RANGE(IN_TO_S_P, log_num_prefetch_pool_entries_sbit0_ebit1, log_num_prefetch_pool_entries_sbit2_ebit4)

#define K_RQ_MAP_COUNT CAPRI_KEY_FIELD(IN_P, rq_map_count)
#define K_RQ_CMB CAPRI_KEY_FIELD(IN_P, rq_cmb)
#define K_RQ_SPEC CAPRI_KEY_FIELD(IN_P, rq_spec)
#define K_RQ_ID CAPRI_KEY_FIELD(IN_P, rq_id)
#define K_LOG_WQE_SIZE CAPRI_KEY_FIELD(IN_P, log_wqe_size)

%%

.align
rdma_aq_rx_wqe_process:

    //Setup     DMA for RQ PT

    add         r4, r0, K_RQ_MAP_COUNT 
    beqi        r4, 1, qp_skip_dma_pt
    crestore    [c1], K_RQ_CMB, 0x1 //BD Slot
    
    PT_BASE_ADDR_GET2(r3) 
    add         r3, r3, k.t3_s2s_aqcb_to_wqe_info_rq_tbl_index, CAPRI_LOG_SIZEOF_U64
    srl         r5, r3, CAPRI_LOG_SIZEOF_U64
    phvwr       p.rqcb0.pt_base_addr, r5
    phvwr       p.rqcb1.pt_base_addr, r5

    add         r4, r0, K_RQ_MAP_COUNT, CAPRI_LOG_SIZEOF_U64

    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CREATE_QP_RQPT_SRC)

    or r2, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr, 0x1, 63
    DMA_HOST_MEM2MEM_SRC_SETUP(r6, r4, r2)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CREATE_QP_RQPT_DST)
    DMA_HBM_MEM2MEM_DST_SETUP(r6, r4, r3)

    b           qp_no_skip_dma_pt
    nop

qp_skip_dma_pt:

    bcf         [!c1], qp_skip_rq_cmb
    nop
    
    phvwr    p.rqcb0.hbm_rq_base_addr, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr[33:HBM_RQ_BASE_ADDR_SHIFT]
    phvwr    p.rqcb1.hbm_rq_base_addr, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr[33:HBM_RQ_BASE_ADDR_SHIFT]
    phvwr    p.rqcb0.rq_in_hbm, 1
    phvwr    p.rqcb1.rq_in_hbm, 1

    b           qp_no_skip_dma_pt
    nop
    
qp_skip_rq_cmb: 
    //copy      the phy address of a single page directly.

    phvwr       p.rqcb0.phy_base_addr, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr[51:12] 
    phvwr       p.rqcb1.phy_base_addr, k.t3_s2s_aqcb_to_wqe_info_rq_dma_addr[51:12]
    phvwr       p.rqcb0.skip_pt, 1
    phvwr       p.rqcb1.skip_pt, 1
    
qp_no_skip_dma_pt: 
    add         r1, K_PREFETCH_BASE_ADDR, r0
    beq         r1, r0, skip_prefetch_en
    add         r1, r0, r1, HBM_PAGE_SIZE_SHIFT   // BD Slot

    sll         r3, K_RQ_ID, K_LOG_PREFETCH_ENTRIES
    add         r1, r1, r3
    sub         r2, K_LOG_PREFETCH_ENTRIES, K_LOG_WQE_SIZE

    phvwr       p.rqcb0.prefetch_en, 1
    phvwr       p.rqcb1.prefetch_en, 1
    phvwrpair   p.rqcb1.prefetch_base_addr, r1, p.rqcb1.log_num_wqes, r2
    phvwrpair   p.rqcb2.prefetch_base_addr, r1, p.rqcb2.log_num_pref_wqes, r2

skip_prefetch_en:
    phvwr       p.rqcb1.spec_en, K_RQ_SPEC
    // setup DMA for SQCB
    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CREATE_QP_CB)        
    RQCB_ADDR_GET(r1, k.t3_s2s_aqcb_to_wqe_info_rq_id, K_RQCB_BASE_ADDR_HI)

    DMA_PHV2MEM_SETUP(r6, c1, rqcb0, rqcb2, r1)

    CAPRI_SET_TABLE_3_VALID(0)          
    nop.e
    nop
