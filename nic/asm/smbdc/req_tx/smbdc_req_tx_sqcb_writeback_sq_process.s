#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct smbdc_req_tx_s4_t0_k k;

#define IN_P t0_s2s_writeback_info
#define IN_TO_S4_P to_s4_to_stage_sq

#define K_CURRENT_SGE_ID     CAPRI_KEY_RANGE(IN_TO_S4_P, current_sge_id_sbit0_ebit5, current_sge_id_sbit6_ebit7)
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_RANGE(IN_TO_S4_P, current_sge_offset_sbit0_ebit5, current_sge_offset_sbit30_ebit31)
#define K_CLEAR_BUSY_AND_EXIT CAPRI_KEY_FIELD(IN_TO_S4_P, clear_busy_and_exit)

%%

.align

smbdc_req_tx_sqcb_writeback_sq_process:

    // Pin to stage 4
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], bubble_to_next_stage

    CAPRI_SET_TABLE_0_VALID(0)

    bbeq          K_CLEAR_BUSY_AND_EXIT, 1, exit
    tblwr         d.busy, 0 //BD Slot
    #restore flags
    crestore      [c5, c4], CAPRI_KEY_RANGE(IN_TO_S4_P, incr_sq_cindex, decr_send_credits), 0x3
    #c5 - incr_sq_cindex
    #c4 - decr_send_credits

    tblwr         d.send_in_progress, CAPRI_KEY_FIELD(IN_TO_S4_P, in_progress)
    tblwr         d.current_sge_id, K_CURRENT_SGE_ID
    tblwr         d.current_sge_offset, K_CURRENT_SGE_OFFSET
    tblmincri.c5  SQ_C_INDEX, d.log_num_wqes, 1
    tblsub.c4     d.send_credits, 1

    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1 | c2], exit
    nop           // Branch Delay Slot

    SQCB0_ADDR_GET(r2)
    //invoke the same routine, but with valid d[] vector
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r2)
    nop.e
    nop

skip_cq_cindex_update:

exit:
    nop.e
    nop
