#include "req_rx.h"
#include "sqcb.h"


struct req_rx_phv_t p;
struct req_rx_s4_t2_k k;
struct sqcb1_t d;

%%
    .param req_rx_stats_process

.align
req_rx_sqcb1_write_back_err_process:

    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_4
    bcf              [!c1], bubble_to_next_stage

    nop              // BD Slot

    //without checking any token_id, move the QP to error disable state
    tblwr.f         d.state, QP_STATE_ERR 


    //invoke stats
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_stats_process, r0)

    // memwr sqcb0 state to QP_STATE_ERR
    sll            r3, d.service, SQCB0_SERVICE_BIT_OFFSET
    or             r3, r3, QP_STATE_ERR
    SQCB0_ADDR_GET(r1)
    add            r1, FIELD_OFFSET(sqcb0_t, service), r1
    memwr.b        r1, r3

    //drop the phv as we do not want to execute any DMA commands.
    //we don't want to post any error completion, as we cannot 
    //relate the error to any specific psn or wrid.
    //in future, possibly generate an async event.
    phvwr.e         p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)  //Exit slot

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1], exit
    SQCB1_ADDR_GET(r1)
    CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

exit:
    nop.e
    nop
