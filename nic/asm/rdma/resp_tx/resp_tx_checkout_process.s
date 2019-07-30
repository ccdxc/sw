#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct pref_ring_t d;
struct resp_tx_s3_t2_k k;

#define IN_P t2_s2s_prefetch_info
#define OUT_P t2_s2s_prefetch_info

#define PREF_CB_ADDR CAPRI_KEY_FIELD(IN_P, pref_cb_or_base_addr)
#define K_CHECK_IN   CAPRI_KEY_FIELD(IN_P, check_in)
#define K_PREF_BUFF_INDEX CAPRI_KEY_FIELD(IN_P, pref_buff_index)

#define RQCB2_ADDR r3

%%

    .param  resp_tx_rqprefetch_process

.align
resp_tx_checkout_process:

    bbeq   K_CHECK_IN, 1, process_checkin
    // prefetch base is 2 pages away from cb base

    sll    r1, PREF_CB_ADDR, PT_BASE_ADDR_SHIFT // BD Slot

    add    r1, r1, d.{val}.hx, 11 // 11 is log (size of prefetch buffer per QP)

    srl    r1, r1, PT_BASE_ADDR_SHIFT
    CAPRI_SET_FIELD2(OUT_P, pref_cb_or_base_addr, r1)

load_rqprefetch:
    RQCB2_ADDR_GET(RQCB2_ADDR)
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqprefetch_process, RQCB2_ADDR) // Exit Slot

process_checkin:
    b      load_rqprefetch
    tblwr  d.{val}.hx, K_PREF_BUFF_INDEX // BD Slot
