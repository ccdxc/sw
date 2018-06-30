#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s1_t0_k k;

#define RQCB0_ADDR  r7
#define WB_INFO_P   t1_s2s_rqcb0_bt_write_back_info

%%

    .param  resp_tx_rqcb0_bt_write_back_process

.align
resp_tx_bt_mpu_only_process:

    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_2
    bcf              [!c1], bubble_to_next_stage
    RQCB0_ADDR_GET(RQCB0_ADDR)      //BD Slot

    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_SET_FIELD2(WB_INFO_P, update_drain_done, 1)
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb0_bt_write_back_process, RQCB0_ADDR)

bubble_to_next_stage:
    nop.e
    nop
