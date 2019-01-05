#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "sqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct sqcb2_t d;
struct aq_tx_s5_t2_k k;

#define IN_P t2_s2s_wqe2_to_sqcb2_info
#define IN_TO_S_P to_s5_info
    
#define K_TX_PSN CAPRI_KEY_RANGE(IN_TO_S_P, tx_psn_sbit0_ebit4, tx_psn_sbit21_ebit23)
#define K_TX_PSN_VALID CAPRI_KEY_FIELD(IN_TO_S_P, tx_psn_valid)
#define K_ERR_RETRY_COUNT CAPRI_KEY_FIELD(IN_TO_S_P, err_retry_count)
#define K_ERR_RETRY_COUNT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, err_retry_count_valid)
#define K_LOCAL_ACK_TIMEOUT CAPRI_KEY_FIELD(IN_TO_S_P, local_ack_timeout)
#define K_LOCAL_ACK_TIMEOUT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, local_ack_timeout_valid)
#define K_RNR_RETRY_TIMER_VALID CAPRI_KEY_FIELD(IN_P, rnr_timer_valid) 
#define K_RNR_RETRY_TIMER  CAPRI_KEY_FIELD(IN_P, rnr_min_timer) 
#define K_RNR_RETRY_COUNT CAPRI_KEY_RANGE(IN_P, rnr_retry_count_sbit0_ebit1, rnr_retry_count_sbit2_ebit2) 
#define K_RNR_RETRY_VALID CAPRI_KEY_FIELD(IN_P, rnr_retry_valid) 
    
%%

    .param      rdma_aq_tx_sqcb0_process
    
.align
rdma_aq_tx_sqcb2_process:

    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], bubble_to_next_stage
    nop
    
hdr_update:
    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, rrq_base
    nop
    
    tblwr     d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    tblwr     d.header_template_size, CAPRI_KEY_FIELD(IN_P, ah_len)

rrq_base:
    bbne        CAPRI_KEY_FIELD(IN_P, rrq_valid), 1, tx_psn
    nop
    
    tblwr       d.rrq_base_addr, CAPRI_KEY_FIELD(IN_P, rrq_base_addr)
    tblwr       d.log_rrq_size, CAPRI_KEY_FIELD(IN_P, rrq_depth_log2)

tx_psn:
    bbne        K_TX_PSN_VALID, 1, timeout
    nop

    tblwr       d.tx_psn, K_TX_PSN
    //Initialize exp_rsp_psn to be 1 less than rexmit_psn
    sub         r3, K_TX_PSN, 1
    tblwr       d.exp_rsp_psn, r3
    tblwr       d.rexmit_psn, K_TX_PSN

timeout:
    bbne        K_LOCAL_ACK_TIMEOUT_VALID, 1, retry_cnt
    nop

    tblwr       d.local_ack_timeout, K_LOCAL_ACK_TIMEOUT

retry_cnt:
    bbne        K_ERR_RETRY_COUNT_VALID, 1, rnr_retry_count
    nop

    tblwr       d.err_retry_ctr, K_ERR_RETRY_COUNT

rnr_retry_count:
    bbne        K_RNR_RETRY_VALID, 1, setup_sqcb0
    nop

    //TODO: For now keep default retry count of 7 (infinite retries) until
    //data path starts supporting.

    //tblwr     d.rnr_retry_ctr, K_RNR_RETRY_COUNT
    
setup_sqcb0:

    mfspr       r2, spr_tbladdr
    sub         r2, r2, (2 * CB_UNIT_SIZE_BYTES)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_sqcb0_process, r2)
    nop.e
    nop
    
bubble_to_next_stage:
exit:
    nop.e
    nop

