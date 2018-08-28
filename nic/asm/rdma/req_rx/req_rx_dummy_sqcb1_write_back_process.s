#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_s1_t0_k k;

#define SQCB1_WRITE_BACK_P t3_s2s_sqcb1_write_back_info

%%
    .param req_rx_sqcb1_write_back_process

.align
req_rx_dummy_sqcb1_write_back_process:

    // Today this program is only invoked in error-path to load sqcb1 write-back in (stage3,table3) to increment nxt-to-go-token-id.
    // Load MPU only program in stage2. write-back has bubble logic to load sqcb1 in stage3.
    CAPRI_RESET_TABLE_3_ARG()
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

    CAPRI_SET_TABLE_0_VALID(0)
    nop.e
    nop

