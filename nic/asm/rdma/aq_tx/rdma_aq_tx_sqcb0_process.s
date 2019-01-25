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
    
#define K_LOCAL_ACK_TIMEOUT CAPRI_KEY_FIELD(IN_TO_S_P, local_ack_timeout)
#define K_LOCAL_ACK_TIMEOUT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, local_ack_timeout_valid)
#define K_Q_KEY_VALID CAPRI_KEY_FIELD(IN_TO_S_P, q_key_valid)
#define K_Q_KEY CAPRI_KEY_RANGE(IN_TO_S_P, q_key_sbit0_ebit2, q_key_sbit27_ebit31)

%%

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

    seq         c2, CAPRI_KEY_FIELD(IN_P, state), QP_STATE_INIT
    bcf         [!c2], timeout
    tblwr       d.state, CAPRI_KEY_FIELD(IN_P, state)  //BD slot

    DMA_CMD_STATIC_BASE_GET(r6, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_CLEAR_STATS_CB)
    mfspr       r2, spr_tbladdr
    add         r2, r2, 1, (3*LOG_CB_UNIT_SIZE_BYTES)

    DMA_PHV2MEM_SETUP(r6, c1, sqcb3, sqcb4, r2)

timeout:
    bbne        K_LOCAL_ACK_TIMEOUT_VALID, 1, pmtu
    nop

    tblwr       d.local_ack_timeout, K_LOCAL_ACK_TIMEOUT

pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P, pmtu_valid), 1, q_key
    nop
    
    tblwr       d.log_pmtu, CAPRI_KEY_FIELD(IN_P, pmtu_log2)
    
q_key:
    bbne        K_Q_KEY_VALID, 1, done
    nop

    tblwr       d.q_key, K_Q_KEY

done:
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop

    
