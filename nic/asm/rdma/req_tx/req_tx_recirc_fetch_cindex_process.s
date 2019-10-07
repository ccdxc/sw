#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s6_t2_k k;
struct sqcb0_t d;

#define IN_TO_S_P   to_s6_sqcb_wb_add_hdr_info

#define K_SPEC_MSG_PSN CAPRI_KEY_RANGE(IN_TO_S_P, read_req_adjust_sbit0_ebit7, read_req_adjust_sbit8_ebit23)

%%

.align
req_tx_recirc_fetch_cindex_process:

    // Pin fetch_cindex to stage 6
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], bubble_to_next_stage

    seq           c2, K_GLOBAL_SPEC_CINDEX, SQ_C_INDEX // BD-slot
    bcf           [!c2], drop_recirc_spec_miss
    CAPRI_SET_TABLE_2_VALID(0) //BD-slot
    bbeq           d.spec_enable, 0, skip_msg_psn_check
    seq            c1, K_SPEC_MSG_PSN, d.msg_psn // BD-slot
    bcf            [!c1], drop_recirc_spec_miss
    nop //BD-slot

skip_msg_psn_check:   
    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], exit
    SQCB0_ADDR_GET(r1) // BD-slot

    //invoke the same routine, but with valid sqcb0 as d[] vector
    CAPRI_GET_TABLE_2_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

drop_recirc_spec_miss:
    phvwr         p.common.p4_intr_global_drop, 1  
   
exit:
    nop.e
    nop
