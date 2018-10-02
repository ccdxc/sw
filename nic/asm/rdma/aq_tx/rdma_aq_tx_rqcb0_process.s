#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "rqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct rqcb0_t d;
struct aq_tx_s4_t1_k k;

#define IN_P t1_s2s_sqcb0_to_rqcb0_info
    
%%

.align
rdma_aq_tx_rqcb0_process:

//TODO: Remove  redundant labels later
    
hdr_update:
    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, rsq_base
    nop
    
    tblwr     d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    tblwr     d.header_template_size, CAPRI_KEY_FIELD(IN_P, ah_len)
    
rsq_base:
    bbne        CAPRI_KEY_FIELD(IN_P, rsq_valid), 1, state
    nop
    
    tblwr       d.rsq_base_addr, CAPRI_KEY_RANGE(IN_P, rsq_base_addr_sbit0_ebit2, rsq_base_addr_sbit3_ebit26)
    tblwr       d.log_rsq_size, CAPRI_KEY_RANGE(IN_P, rsq_depth_log2_sbit0_ebit2, rsq_depth_log2_sbit3_ebit4)
    
state:
    bbne        CAPRI_KEY_FIELD(IN_P, state_valid), 1, pmtu
    nop
    
    tblwr       d.state, CAPRI_KEY_FIELD(IN_P, state)
    
pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P , pmtu_valid), 1, done
    nop
    
    tblwr       d.log_pmtu, CAPRI_KEY_FIELD(IN_P, pmtu_log2)
    
done:
    CAPRI_SET_TABLE_1_VALID(0)              
    nop.e
    nop
    
