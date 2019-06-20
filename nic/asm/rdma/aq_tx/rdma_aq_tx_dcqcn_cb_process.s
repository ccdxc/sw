#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct dcqcn_cb_t d;
struct aq_tx_s6_t3_k k;

#define IN_TO_S_P to_s6_info
#define IN_P    t3_s2s_dcqcn_config_to_cb_info

#define K_BYTE_CNTR_THR CAPRI_KEY_FIELD(IN_P, byte_cntr_thr)
#define K_ALPHA_VAL CAPRI_KEY_FIELD(IN_P, alpha_val)
#define K_LOG_SQ_SIZE CAPRI_KEY_FIELD(IN_P, log_sq_size)

%%

.align
rdma_aq_tx_dcqcn_cb_process:
    tblwr       d.byte_counter_thr, K_BYTE_CNTR_THR
    tblwr       d.alpha_value, K_ALPHA_VAL
    tblwr       d.log_sq_size, K_LOG_SQ_SIZE

    // TODO: Hardcoded enforced & target rate to 100G. Get this info from product skew
    tblwr       d.rate_enforced, 100000
    tblwr       d.target_rate, 100000
    // TODO: Hardcoded token bucket size to 150kb. Get info from dcqcn config
    tblwr       d.token_bucket_size, 150000
    tblwr       d.cur_avail_tokens, 150000

    CAPRI_SET_TABLE_3_VALID_CE(c0, 0)
    nop // Exit Slot
