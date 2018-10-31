#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "sqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct sqcb0_t d;
struct aq_tx_s3_t1_k k;

#define IN_P t1_s2s_wqe2_to_sqcb0_info
#define IN_TO_S_P to_s3_info
    
#define SQCB0_RQCB0_P t1_s2s_sqcb0_to_rqcb0_info

#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, rqcb_base_addr_hi)    
#define K_LOCAL_ACK_TIMEOUT CAPRI_KEY_FIELD(IN_TO_S_P, local_ack_timeout)
#define K_LOCAL_ACK_TIMEOUT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, local_ack_timeout_valid)
%%

    .param      rdma_aq_tx_rqcb0_process
    
.align
rdma_aq_tx_sqcb0_process:

//TODO: Remove  redundant labels later
    
hdr_update:
    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, state
    nop

    tblwr       d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    
state:
    bbne        CAPRI_KEY_FIELD(IN_P, state_valid), 1, timeout
    nop

    tblwr       d.state, CAPRI_KEY_FIELD(IN_P, state)

timeout:
    bbne        K_LOCAL_ACK_TIMEOUT_VALID, 1, pmtu
    nop

    tblwr       d.local_ack_timeout, K_LOCAL_ACK_TIMEOUT

pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P, pmtu_valid), 1, setup_sqcb_stages
    nop
    
    tblwr       d.log_pmtu, CAPRI_KEY_FIELD(IN_P, pmtu_log2)
    
setup_sqcb_stages: 

    CAPRI_RESET_TABLE_1_ARG()
    phvwr       CAPRI_PHV_RANGE(SQCB0_RQCB0_P, state, pad), CAPRI_KEY_RANGE(IN_P, state, pad_sbit1_ebit8)
    RQCB_ADDR_GET(r2, CAPRI_KEY_RANGE(IN_P, qid_sbit0_ebit0, qid_sbit17_ebit23), K_RQCB_BASE_ADDR_HI)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_rqcb0_process, r2)

done:
    nop.e
    nop
