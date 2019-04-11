#include "capri.h"
#include "aq_rx.h"
#include "aqcb.h"
#include "sqcb.h"    
#include "common_phv.h"
#include "types.h"

struct aq_rx_phv_t p;
struct sqcb1_t d;
struct aq_rx_s4_t2_k k;

#define IN_P t2_s2s_aqcb_to_sqcb1_info
#define IN_TO_S_P to_s4_info
    
#define SQCB1_RQCB1_P t2_s2s_sqcb1_to_rqcb1_info

#define K_SQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, sqcb_base_addr_hi)    
#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, rqcb_base_addr_hi)
#define K_RQ_ID CAPRI_KEY_RANGE(IN_P, rq_id_sbit0_ebit4, rq_id_sbit21_ebit23)    

%%

    .param      rdma_aq_rx_destroy_rqcb1_process
    
.align
rdma_aq_rx_destroy_sqcb1_process:

    // Pin cqcb process to stage 4
    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_4
    bcf         [!c1], bubble_to_next_stage
    nop

    tblwr       d.state, QP_STATE_RESET 
    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_DESTROY_QP_SQCB0_STATE)
    SQCB_ADDR_GET(r2, K_RQ_ID, K_SQCB_BASE_ADDR_HI)
    add         r3, FIELD_OFFSET(sqcb0_t, service), r2
    // Repurposing mod_qp structures for destroy_qp. Writing QP_STATE_RESET to sqcb0 state
    phvwrpair   p.mod_qp.service, d.service, p.{mod_qp.flush_rq...mod_qp.state}, r0
    DMA_HBM_PHV2MEM_SETUP(r6, mod_qp.service, mod_qp.state, r3)

setup_rqcb_stages: 

    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_DESTROY_QP_RQCB0_STATE)
    RQCB_ADDR_GET(r2, K_RQ_ID, K_RQCB_BASE_ADDR_HI)
    // Repurposing mod_qp structures for destroy_qp
    add         r3, FIELD_OFFSET(rqcb0_t, state), r2
    // Setup DMA instruction now. Values in PHV will be populated in next stage.
    DMA_HBM_PHV2MEM_SETUP(r6, mod_qp.rq_state, mod_qp.log_rsq_size, r3)

    add         r2, r2, CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_rx_destroy_rqcb1_process, r2)

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_3
    bcf         [!c1], exit

    SQCB_ADDR_GET(r2, K_RQ_ID, K_SQCB_BASE_ADDR_HI)
    add         r2, r2, CB_UNIT_SIZE_BYTES
    CAPRI_GET_TABLE_2_K(aq_rx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r2) //Exit Slot

exit:
    nop.e
    nop
    
