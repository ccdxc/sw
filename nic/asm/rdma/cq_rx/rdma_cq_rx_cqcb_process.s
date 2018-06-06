#include "cq_rx.h"
#include "cqcb.h"

struct cq_rx_phv_t p;
struct cqcb_t d;
struct cq_rx_s5_t2_k k;

#define EQ_INFO_P t2_s2s_cqcb_to_eq_info

#define IN_P t2_s2s_cqcb0_to_cq_info
#define IN_TO_S_P to_s5_info

#define K_ARM_CINDEX CAPRI_KEY_FIELD(IN_P, cindex)
#define K_ARM        CAPRI_KEY_FIELD(IN_P, arm)
#define K_SARM       CAPRI_KEY_FIELD(IN_P, sarm)
#define K_CQ_ID       CAPRI_KEY_FIELD(IN_P, cq_id)

#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_LOG_NUM_CQ_ENTRIES CAPRI_KEY_FIELD(IN_TO_S_P, log_num_cq_entries)

%%

    .param   rdma_cq_rx_eqcb_process
.align
rdma_cq_rx_cqcb_process:

    // Pin cqcb process to stage 5
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], bubble_to_next_stage
    nop //BD Slot

cqcb_process:

check_arm:

    //if (arm):
    //    if (arm_cindex < proxy_pindex):
    //        #post EQ event
    //    else:
    //        #set ARM bit

    bbne          K_ARM, 1, check_sarm
    scwlt         c1, K_ARM_CINDEX, d.proxy_pindex //BD slot
    bcf           [c1], post_eq_event
    tblwr.!c1     d.arm, 1
    
check_sarm:

    //if (sarm):
    //    if (arm_cindex < proxy_s_pindex):
    //        #post EQ event
    //    else:
    //        set SARM bit

    bbne          K_SARM, 1, die_down
    scwlt         c1, K_ARM_CINDEX, d.proxy_s_pindex //BD slot
    bcf           [c1], post_eq_event
    tblwr.!c1     d.sarm, 1
  
die_down:

    phvwr.e       p.common.p4_intr_global_drop, 1   
    CAPRI_SET_TABLE_2_VALID(0) //Exit Slot
    

post_eq_event:
    CQ_RX_EQCB_ADDR_GET(r2, r3, d.eq_id, K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES)

    CAPRI_RESET_TABLE_2_ARG()

    phvwrpair CAPRI_PHV_FIELD(EQ_INFO_P, cq_id), K_CQ_ID, CAPRI_PHV_FIELD(EQ_INFO_P, eqcb_addr), r2
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, rdma_cq_rx_eqcb_process, r0)

    tblwr.e d.{arm...sarm}, 0
    nop //Exit Slot

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], exit
    CQCB0_ADDR_GET(r2) //BD Slot

    //invoke the same routine, but with valid cqcb addr
    CAPRI_GET_TABLE_2_K(cq_rx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r2)

exit:
    nop.e
    nop

