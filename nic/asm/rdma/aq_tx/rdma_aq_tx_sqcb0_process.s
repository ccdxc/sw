#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "sqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct sqcb0_t d;
struct aq_tx_s6_t2_k k;

#define IN_P t2_s2s_sqcb2_to_sqcb0_info
#define IN_TO_S_P to_s6_info
    
#define K_LOCAL_ACK_TIMEOUT CAPRI_KEY_RANGE(IN_TO_S_P, local_ack_timeout_or_dscp_sbit0_ebit3, local_ack_timeout_or_dscp_sbit4_ebit4)
#define K_LOCAL_ACK_TIMEOUT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, local_ack_timeout_valid)
#define K_Q_KEY_VALID CAPRI_KEY_FIELD(IN_TO_S_P, q_key_valid)
#define K_Q_KEY_TM_IQ CAPRI_KEY_RANGE(IN_TO_S_P, q_key_or_tm_iq_sbit0_ebit1, q_key_or_tm_iq_sbit26_ebit31)
#define K_TX_PSN CAPRI_KEY_RANGE(IN_TO_S_P, tx_psn_sbit0_ebit3, tx_psn_sbit20_ebit23)
#define K_CONGESTION_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_enable)

%%

.align
rdma_aq_tx_sqcb0_process:

//TODO: Remove  redundant labels later

    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, timeout
    nop

    tblwr       d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    // Update cos(tm_iq) in qstate.
    tblwr       d.intrinsic.cosB, K_Q_KEY_TM_IQ
    tblwr       d.congestion_mgmt_type, K_CONGESTION_ENABLE
    
timeout:
    bbne        K_LOCAL_ACK_TIMEOUT_VALID, 1, pmtu
    nop

    tblwr       d.local_ack_timeout, K_LOCAL_ACK_TIMEOUT

pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P, pmtu_valid), 1, q_key
    nop
    
    tblwr       d.log_pmtu, CAPRI_KEY_RANGE(IN_P, pmtu_log2_sbit0_ebit2, pmtu_log2_sbit3_ebit4)
    
q_key:
    bbne        K_Q_KEY_VALID, 1, done
    nop

    tblwr       d.q_key, K_Q_KEY_TM_IQ

done:
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop
