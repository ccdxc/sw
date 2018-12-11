#include "aq_rx.h"
#include "cqcb.h"
#include "aqcb.h"    

struct aq_rx_phv_t p;
struct aq_rx_s6_t0_k k;

%%
    .param      rdma_aq_rx_cqcb_process

.align
rdma_aq_rx_cqcb_mpu_only_process:    
    // Pin cqcb process to stage 6
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], bubble_to_next_stage

    CAPRI_SET_TABLE_0_VALID_C(c1, 0)  //BD slot
    mfspr       r2, spr_tbladdr
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_rx_cqcb_process, r2)    

bubble_to_next_stage:
exit:
    nop.e
    nop
    
