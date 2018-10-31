#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "sqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct sqcb2_t d;
struct aq_tx_s3_t3_k k;

#define IN_P t3_s2s_wqe2_to_sqcb2_info
#define IN_TO_S_P to_s3_info
    
#define SQCB2_RQCB2_P t3_s2s_sqcb2_to_rqcb2_info

#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, rqcb_base_addr_hi)    
#define K_TX_PSN CAPRI_KEY_RANGE(IN_TO_S_P, tx_psn_sbit0_ebit6, tx_psn_sbit23_ebit23)
#define K_TX_PSN_VALID CAPRI_KEY_FIELD(IN_TO_S_P, tx_psn_valid)
#define K_ERR_RETRY_COUNT CAPRI_KEY_RANGE(IN_TO_S_P, err_retry_count_sbit0_ebit0, err_retry_count_sbit1_ebit2)
#define K_ERR_RETRY_COUNT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, err_retry_count_valid)
#define K_LOCAL_ACK_TIMEOUT CAPRI_KEY_FIELD(IN_TO_S_P, local_ack_timeout)
#define K_LOCAL_ACK_TIMEOUT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, local_ack_timeout_valid)

%%

    .param      rdma_aq_tx_rqcb2_process
    
.align
rdma_aq_tx_sqcb2_process:

//TODO: Remove  redundant labels later
    
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
    sub         r3, d.rexmit_psn, 1
    tblwr       d.exp_rsp_psn, r3
    tblwr       d.rexmit_psn, K_TX_PSN

timeout:
    bbne        K_LOCAL_ACK_TIMEOUT_VALID, 1, retry_cnt
    nop

    tblwr       d.local_ack_timeout, K_LOCAL_ACK_TIMEOUT

retry_cnt:
    bbne        K_ERR_RETRY_COUNT_VALID, 1, done
    nop

    tblwr       d.err_retry_ctr, K_ERR_RETRY_COUNT

done:
    CAPRI_SET_TABLE_3_VALID(0)
    CAPRI_RESET_TABLE_3_ARG()    
    nop.e
    nop

