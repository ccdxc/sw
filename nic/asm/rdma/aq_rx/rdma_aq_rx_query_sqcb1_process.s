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

    .param      rdma_aq_rx_query_rqcb1_process
    
.align
rdma_aq_rx_query_sqcb1_process:

    // Pin cqcb process to stage 4
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], bubble_to_next_stage
    nop

    add         r2, r0, d.state
    add         r2, r0, d.log_pmtu
    phvwrpair   p.query_rq.state, d.state, p.query_rq.pmtu, r2
    phvwrpair   p.query_rq.retry_cnt, d.err_retry_count, p.query_rq.rnr_retry, d.rnr_retry_count
    phvwrpair   p.query_rq.rrq_depth, d.log_rrq_size, p.query_rq.sq_psn, d.tx_psn

setup_rqcb_stages: 

    RQCB_ADDR_GET(r2, K_RQ_ID, K_RQCB_BASE_ADDR_HI)
    add         r2, r2, CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_rx_query_rqcb1_process, r2)

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_3
    bcf         [!c1], exit

    SQCB_ADDR_GET(r2, K_RQ_ID, K_SQCB_BASE_ADDR_HI)
    add         r2, r2, CB_UNIT_SIZE_BYTES
    CAPRI_GET_TABLE_2_K(aq_rx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r2) //Exit Slot

exit:
    nop.e
    nop
    
