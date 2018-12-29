#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"

struct resp_rx_phv_t p;
struct resp_rx_s5_t2_k k;
struct rqcb1_t d;


#define RQCB1_ADDR   r3

%%

    .param  resp_rx_stats_process

.align
resp_rx_rqcb1_write_back_err_process:

    // Pin it to stage 5
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_5
    bcf              [!c1], bubble_to_next_stage

    nop              // BD Slot

    //without checking any token_id, move the QP to error disable state
    tblwr.f         d.state, QP_STATE_ERR

    //invoke stats
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_stats_process, r0)

    //drop the phv as we do not want to execute any DMA commands.
    //we don't want to return any NAK or post any error completion, as we cannot 
    //relate the error to any specific psn or wrid.
    //in future, possibly generate an async event.
    phvwr.e         p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)  //Exit slot

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], exit

    //invoke the same routine, but with valid d[]
    CAPRI_GET_TABLE_2_K(resp_rx_phv_t, r7) //BD Slot
    RQCB1_ADDR_GET(RQCB1_ADDR)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RQCB1_ADDR)

exit:
    nop.e
    nop


