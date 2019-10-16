#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct prefetch_cb_t d;
struct resp_tx_s2_t2_k k;

#define IN_P t2_s2s_prefetch_info
#define OUT_P t2_s2s_prefetch_info

#define PREFETCH_CB_ADDR CAPRI_KEY_FIELD(IN_P, prefetch_cb_or_base_addr)
#define K_CHECK_IN   CAPRI_KEY_FIELD(IN_P, check_in)
#define K_PREFETCH_BUF_INDEX CAPRI_KEY_FIELD(IN_P, prefetch_buf_index)

%%

    .param  resp_tx_checkout_process

.align
resp_tx_pre_checkout_process:

    sll         r1, PREFETCH_CB_ADDR, PT_BASE_ADDR_SHIFT
    sub         r1, r1, 1, HBM_PAGE_SIZE_SHIFT
    // r1: base addr of prefetch ring

    bbeq        K_CHECK_IN, 1, process_checkin
    seq         c1, d.{p_index}.hx, d.{c_index}.hx // BD Slot

    bcf         [c1], checkout_fail
    add         r1, r1, d.{c_index}.hx, 1 // BD Slot

    add         r2, r0, d.{c_index}.hx
    CAPRI_SET_FIELD2(OUT_P, prefetch_buf_index, r2)

    tblmincri   d.{c_index}.hx, 10, 1
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_checkout_process, r1) // Exit Slot

process_checkin:
    // TODO check if there is room to check_in
    add         r1, r1, d.{p_index}.hx, 1

    tblmincri   d.{p_index}.hx, 10, 1
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_checkout_process, r1) // Exit Slot

checkout_fail:
    // TODO handle checkout fail
    CAPRI_SET_TABLE_2_VALID_CE(c0, 0)
    phvwr       p.common.p4_intr_global_drop, 1 // Exit Slot
