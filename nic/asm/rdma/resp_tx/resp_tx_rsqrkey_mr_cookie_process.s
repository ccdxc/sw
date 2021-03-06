#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s4_t3_k k;
struct key_entry_aligned_t d;

#define GLOBAL_FLAGS r6

#define IN_P         t3_s2s_rsqrkey_to_rkey_cookie_info
#define TO_S5_P      to_s5_rqcb1_wb_info
#define TO_S7_P      to_s7_stats_info

#define K_MW_COOKIE CAPRI_KEY_FIELD(IN_P, mw_cookie)

%%

.align
resp_tx_rsqrkey_mr_cookie_process:

    // access is allowed only in valid state
    seq         c1, d.state, KEY_STATE_VALID
    seq         c2, d.mr_cookie, K_MW_COOKIE 

    bcf         [!c1 | !c2], error_completion
    nop // BD Slot

    CAPRI_SET_TABLE_2_VALID_CE(c0, 0)
    nop // Exit Slot

error_completion:
    // When rkey validation for a read response fails, we generate a NAK
    // and ensure further read responses are not generated by moving the QP to
    // error disable state

    CAPRI_SET_FIELD2(phv_global_common, _error_disable_qp, 1)

    CAPRI_SET_FIELD2(TO_S5_P, ack_nak_process, 1)
    phvwr       CAPRI_PHV_FIELD(TO_S7_P, last_syndrome), AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR)
    phvwr.e     p.aeth.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR)
    CAPRI_SET_TABLE_2_VALID_CE(c0, 0) // Exit Slot
