#include "cq_rx.h"
#include "cqcb.h"

struct cq_rx_phv_t p;
struct cqcb_t d;
struct cq_rx_s6_t2_k k;

#define EQ_INFO_P t1_s2s_cqcb_to_eq_info

#define IN_P t2_s2s_cqcb0_to_cq_info
#define IN_TO_S_P to_s6_info

#define K_ARM_CINDEX CAPRI_KEY_FIELD(IN_P, cindex)
#define K_ARM_COLOR  CAPRI_KEY_FIELD(IN_P, color)
#define K_ARM        CAPRI_KEY_FIELD(IN_P, arm)
#define K_SARM       CAPRI_KEY_FIELD(IN_P, sarm)
#define K_CQ_ID      CAPRI_KEY_FIELD(IN_P, cq_id)
#define K_CQCB_ADDR  CAPRI_KEY_RANGE(IN_P, cqcb_addr_sbit0_ebit4, cqcb_addr_sbit29_ebit33)

#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_LOG_NUM_CQ_ENTRIES CAPRI_KEY_FIELD(IN_TO_S_P, log_num_cq_entries)

%%

    .param   rdma_cq_rx_eqcb_process
.align
rdma_cq_rx_cqcb_process:

    // Pin cqcb process to stage 6
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], bubble_to_next_stage
    nop //BD Slot

cqcb_process:

check_arm:

    //if (arm):
    //    if (arm_color != color) || (arm_cindex != proxy_pindex):
    //        #post EQ event
    //    else:
    //        #set ARM bit

    bbne          K_ARM, 1, check_sarm
    sne           c1, K_ARM_COLOR, d.color //BD Slot
    sne.!c1       c1, K_ARM_CINDEX, CQ_PROXY_PINDEX
    bcf           [c1], post_eq_event
    tblwr.!c1     d.arm, 1 //BD Slot
    
check_sarm:

    //if (sarm):
    //    if (arm_color != color) || (arm_cindex != proxy_s_pindex):
    //        #post EQ event
    //    else:
    //        set SARM bit

    bbne          K_SARM, 1, die_down
    sne           c1, K_ARM_COLOR, d.color //BD Slot
    sne.!c1       c1, K_ARM_CINDEX, CQ_PROXY_S_PINDEX
    bcf           [c1], post_eq_event
    tblwr.!c1     d.sarm, 1 //BD Slot
  
die_down:

    phvwr.e       p.common.p4_intr_global_drop, 1   
    CAPRI_SET_TABLE_2_VALID(0) //Exit Slot
    

post_eq_event:
    CQ_RX_EQCB_ADDR_GET(r2, r3, d.eq_id, K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES)

    CAPRI_RESET_TABLE_1_ARG()

    phvwr   CAPRI_PHV_FIELD(EQ_INFO_P, cq_id), K_CQ_ID

    CAPRI_SET_TABLE_2_VALID(0) 
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_cq_rx_eqcb_process, r2)

    tblwr.e d.{arm...sarm}, 0
    tblwr   CQ_PROXY_S_PINDEX, K_ARM_CINDEX //Exit Slot

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], exit

    //invoke the same routine, but with valid cqcb addr
    CAPRI_GET_TABLE_2_K_NO_VALID(cq_rx_phv_t, r7) //nop
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, K_CQCB_ADDR)

exit:
    nop.e
    nop

