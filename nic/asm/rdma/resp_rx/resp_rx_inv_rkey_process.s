#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s6_t0_k k;
struct key_entry_aligned_t d;

%%

.align
resp_rx_inv_rkey_process:
    // update the state to FREE
    CAPRI_SET_TABLE_0_VALID_CE(c0, 0)
    tblwr       d.state, KEY_STATE_FREE // Exit Slot

