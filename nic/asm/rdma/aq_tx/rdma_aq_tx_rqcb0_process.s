#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "rqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct rqcb0_t d;
struct aq_tx_s5_t1_k k;

#define IN_P t1_s2s_wqe2_to_rqcb0_info
#define IN_TO_S_P to_s5_info

#define K_Q_KEY CAPRI_KEY_RANGE(IN_TO_S_P, q_key_sbit0_ebit2, q_key_sbit27_ebit31)
%%

.align
rdma_aq_tx_rqcb0_process:

    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], bubble_to_next_stage
    nop
    
hdr_update:
    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, rsq_base
    nop
    
    tblwr     d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    tblwr     d.header_template_size, CAPRI_KEY_FIELD(IN_P, ah_len)
    
rsq_base:
    bbne        CAPRI_KEY_FIELD(IN_P, rsq_valid), 1, state
    nop
    
    tblwr       d.rsq_base_addr, CAPRI_KEY_RANGE(IN_P, rsq_base_addr_sbit0_ebit2, rsq_base_addr_sbit27_ebit31)
    tblwr       d.log_rsq_size, CAPRI_KEY_RANGE(IN_P, rsq_depth_log2_sbit0_ebit2, rsq_depth_log2_sbit3_ebit4)
    
state:
    bbne        CAPRI_KEY_FIELD(IN_P, state_valid), 1, pmtu
    nop
    
    tblwr       d.state, CAPRI_KEY_FIELD(IN_P, state)
    
pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P , pmtu_valid), 1, done
    nop
    
    tblwr       d.log_pmtu, CAPRI_KEY_FIELD(IN_P, pmtu_log2)
    
q_key:
    bbne        CAPRI_KEY_FIELD(IN_TO_S_P , q_key_valid), 1, done
    nop

    tblwr       d.q_key, K_Q_KEY

done:
    CAPRI_SET_TABLE_1_VALID(0)
    
bubble_to_next_stage:
exit: 
    nop.e
    nop
    
    
    
