#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb2_t d;
struct resp_tx_s1_t2_k k;

#define IN_P t2_s2s_prefetch_info
#define OUT_P t2_s2s_prefetch_info

#define PREFETCH_CB_ADDR CAPRI_KEY_FIELD(IN_P, prefetch_cb_or_base_addr)
#define K_RQ_PINDEX CAPRI_KEY_FIELD(IN_P, rq_pindex)

#define RQCB2_ADDR r3

%%
    .param       resp_tx_pre_checkout_process
    .param       resp_tx_rqprefetch_mpu_only_process

resp_tx_setup_checkout_process:

    //seq         c1, K_RQ_PINDEX, RQ_PROXY_C_INDEX
    //bcf         [c1], process_checkin
    seq         c2, d.checkout_done, 1 // BD Slot
    // c2: checkout_done

    bcf         [c2], exit
    nop // BD Slot

process_checkout:
    tblwr       d.checkout_done, 1

load_pre_checkout:
    add         r1, r0, PREFETCH_CB_ADDR, PT_BASE_ADDR_SHIFT
    sub         r1, r1, 2, HBM_PAGE_SIZE_SHIFT
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_pre_checkout_process, r1) // Exit Slot

process_checkin:
    bcf         [!c2], exit
    tblwr.c2    d.checkout_done, 0 // BD Slot

    b           load_pre_checkout
    phvwrpair   CAPRI_PHV_FIELD(OUT_P, prefetch_buf_index), d.prefetch_buf_index, \
                CAPRI_PHV_FIELD(OUT_P, check_in), 1 // BD Slot

exit:
    // Bubbles down to stage 4
    RQCB2_ADDR_GET(RQCB2_ADDR)
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_rqprefetch_mpu_only_process, RQCB2_ADDR) // Exit Slot
