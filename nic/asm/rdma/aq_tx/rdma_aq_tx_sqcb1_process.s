#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "sqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct sqcb1_t d;
struct aq_tx_s3_t2_k k;

#define IN_P t2_s2s_wqe2_to_sqcb1_info
#define IN_TO_S_P to_s3_info
    
#define SQCB1_RQCB1_P t2_s2s_sqcb1_to_rqcb1_info

#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, rqcb_base_addr_hi)
#define K_TX_PSN CAPRI_KEY_RANGE(IN_TO_S_P, tx_psn_sbit0_ebit6, tx_psn_sbit23_ebit23)
#define K_TX_PSN_VALID CAPRI_KEY_FIELD(IN_TO_S_P, tx_psn_valid)
#define K_ERR_RETRY_COUNT CAPRI_KEY_RANGE(IN_TO_S_P, err_retry_count_sbit0_ebit0, err_retry_count_sbit1_ebit2)
#define K_ERR_RETRY_COUNT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, err_retry_count_valid)
    
%%

    .param      rdma_aq_tx_rqcb1_process
    
.align
rdma_aq_tx_sqcb1_process:

//TODO: Remove  redundant labels later
    
hdr_update:
    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, rrq_base
    nop
    
    tblwr     d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    tblwr     d.header_template_size, CAPRI_KEY_FIELD(IN_P, ah_len)
    
rrq_base:
    bbne        CAPRI_KEY_FIELD(IN_P, rrq_valid), 1, state
    nop
    
    tblwr       d.rrq_base_addr, CAPRI_KEY_FIELD(IN_P, rrq_base_addr)
    tblwr       d.log_rrq_size, CAPRI_KEY_FIELD(IN_P, rrq_depth_log2)
    
state:
    bbne        CAPRI_KEY_FIELD(IN_P, state_valid), 1, tx_psn
    nop
    
    tblwr       d.state, CAPRI_KEY_FIELD(IN_P, state)

tx_psn:
    bbne        K_TX_PSN_VALID, 1, retry_cnt
    nop

    tblwr       d.tx_psn, K_TX_PSN
    tblwr       d.max_tx_psn, K_TX_PSN
    tblwr       d.rexmit_psn, K_TX_PSN

retry_cnt:
    bbne        K_ERR_RETRY_COUNT_VALID, 1, pmtu
    nop

    tblwr       d.err_retry_count, K_ERR_RETRY_COUNT

pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P , pmtu_valid), 1, setup_sqcb_stages
    nop
                                        
    tblwr       d.log_pmtu, CAPRI_KEY_FIELD(IN_P, pmtu_log2)
    
setup_sqcb_stages: 

    CAPRI_RESET_TABLE_2_ARG()
    phvwr       CAPRI_PHV_RANGE(SQCB1_RQCB1_P, state, pad), CAPRI_KEY_RANGE(IN_P, state, pad_sbit1_ebit8)
    RQCB_ADDR_GET(r2, CAPRI_KEY_RANGE(IN_P, qid_sbit0_ebit0, qid_sbit17_ebit23), K_RQCB_BASE_ADDR_HI)
    add         r2, r2, CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_rqcb1_process, r2)

done:
    nop.e
    nop
    
