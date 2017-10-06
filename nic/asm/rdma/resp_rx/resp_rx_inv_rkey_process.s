#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_inv_rkey_process_k_t k;

#define KEY_P   r7
#define TBL_ID  r6

%%

.align
resp_rx_inv_rkey_process:

    // rkey_p = rkey_p + rkey_info_p->key_id;
    //big-endian
    sub         KEY_P, (HBM_NUM_KEY_ENTRIES_PER_CACHE_LINE - 1), k.args.key_id
    add         KEY_P, r0, KEY_P, LOG_SIZEOF_KEY_ENTRY_T_BITS

    // it is an error to invalidate an MR not eligible for invalidation
    // (Disabled for now till MR objects in DOL can have this
    //  configuration)
    // it is an error to invalidate an MR in INVALID state
    //CAPRI_TABLE_GET_FIELD(r1, KEY_P, KEY_ENTRY_T, flags)
    //ARE_ALL_FLAGS_SET_B(c1, r1, MR_FLAG_INV_EN)
    //bcf         [!c1], error_completion

    CAPRI_TABLE_GET_FIELD(r1, KEY_P, KEY_ENTRY_T, state) //BD Slot
    seq         c1, r1, KEY_STATE_INVALID
    bcf         [c1], error_completion
    nop
    
    // update the state to FREE
    add r2, r0, KEY_STATE_FREE
    CAPRI_TABLE_SET_FIELD(r2, KEY_P, KEY_ENTRY_T, state)

    add         TBL_ID, r0, k.args.tbl_id
    CAPRI_SET_TABLE_I_VALID(TBL_ID, 0)

    nop.e
    nop

error_completion:
    //TODO

    nop.e
    nop
