#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_inv_rkey_process_k_t k;
struct key_entry_aligned_t d;

#define KEY_P   r7
#define TBL_ID  r6

%%

.align
resp_rx_inv_rkey_process:

    // it is an error to invalidate an MR not eligible for invalidation
    // (Disabled for now till MR objects in DOL can have this
    //  configuration)
    // it is an error to invalidate an MR in INVALID state
    //CAPRI_TABLE_GET_FIELD(r1, KEY_P, KEY_ENTRY_T, flags)
    //ARE_ALL_FLAGS_SET_B(c1, r1, MR_FLAG_INV_EN)
    //bcf         [!c1], error_completion

    seq         c1, d.state, KEY_STATE_INVALID //BD slot (after uncomment of above code)
    bcf         [c1], error_completion
    nop    //BD slot
    
    // update the state to FREE
    tblwr       d.state, KEY_STATE_FREE

    add         TBL_ID, r0, k.args.tbl_id
    CAPRI_SET_TABLE_I_VALID(TBL_ID, 0)

    nop.e
    nop

error_completion:
    //TODO

    nop.e
    nop
