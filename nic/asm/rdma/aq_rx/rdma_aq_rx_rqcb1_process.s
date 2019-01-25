#include "capri.h"
#include "aq_rx.h"
#include "aqcb.h"
#include "rqcb.h"    
#include "common_phv.h"
#include "types.h"

struct aq_rx_phv_t p;
struct rqcb1_t d;
struct aq_rx_s5_t2_k k;

#define IN_P t2_s2s_sqcb1_to_rqcb1_info
#define IN_TO_S_P to_s5_info

%%

.align
rdma_aq_rx_rqcb1_process:

//TODO: Remove  redundant labels later
    
hdr_update:
    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, rsq_base
    nop
    
    tblwr     d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    tblwr     d.header_template_size, CAPRI_KEY_FIELD(IN_P, ah_len)
    
rsq_base:
    bbne        CAPRI_KEY_FIELD(IN_TO_S_P, rsq_valid), 1, state
    nop
    
    tblwr       d.rsq_base_addr, CAPRI_KEY_FIELD(IN_TO_S_P, rsq_base_addr)
    tblwr       d.log_rsq_size, CAPRI_KEY_FIELD(IN_TO_S_P, rsq_depth_log2)
    
state:
    bbne        CAPRI_KEY_FIELD(IN_P, state_valid), 1, pmtu
    nop
    
    seq         c2, CAPRI_KEY_FIELD(IN_P, state), QP_STATE_INIT
    bcf         [!c2], pmtu
    tblwr       d.state, CAPRI_KEY_FIELD(IN_P, state)  //BD slot

    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CLEAR_STATS_CB)
    mfspr       r2, spr_tbladdr
    add         r2, r2, 1, (2*LOG_CB_UNIT_SIZE_BYTES)

    DMA_PHV2MEM_SETUP(r6, c1, rqcb3, rqcb4, r2)
    
pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P , pmtu_valid), 1, q_key
    nop
    
    tblwr       d.log_pmtu, CAPRI_KEY_RANGE(IN_P, pmtu_log2_sbit0_ebit2, pmtu_log2_sbit3_ebit4)

q_key:
    bbne        CAPRI_KEY_FIELD(IN_TO_S_P , q_key_valid), 1, done
    nop

    tblwr       d.q_key, CAPRI_KEY_RANGE(IN_TO_S_P, q_key_sbit0_ebit1, q_key_sbit26_ebit31)

done:
    nop.e
    nop
    
