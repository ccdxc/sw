# include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s1_t2_k k;

#define IN_P t2_s2s_wqe_to_sge_info
#define IN_TO_S_P to_s1_sq_to_stage

#define WQE_TO_SGE_P t0_s3s_wqe_to_sge_info

#define K_CURRENT_SGE_ID CAPRI_KEY_RANGE(IN_P, current_sge_id_sbit0_ebit5, current_sge_id_sbit6_ebit7)
#define K_WQE_ADDR       CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr)

%%
    .param    req_tx_sqsge_process

.align
req_tx_sqsge_iterate_process:
    mfspr          r1, spr_mpuid
    add            r1, r1[4:2], r0
    
    beqi           r1, STAGE_7, trigger_stg0_sqsge_process
    nop            // Branch Delay Slot

    beqi           r1, STAGE_2, trigger_stg3_sqsge_process
    nop            // Branch Delay Slot

    //CAPRI_GET_TABLE_3_ARG_NO_RESET(req_tx_phv_t, r7)

    //add              r1, k.args.bubble_count, r0
    //sub              r1, r1, 1

    //beqi             r1, 0, trigger_sqsge_process
    //CAPRI_SET_FIELD(r7, WQE_TO_SGE_P, bubble_count, r1) // Branch Delay Slot

    nop.e
    nop

trigger_stg3_sqsge_process:
    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD_RANGE2(WQE_TO_SGE_P, in_progress, inv_key_or_ah_handle, CAPRI_KEY_RANGE(IN_P, in_progress, inv_key_or_ah_handle_sbit8_ebit31))

    //mfspr          r1, spr_tbladdr

    // sge_offset = TXWQE_SGE_OFFSET + sqcb0_p->current_sge_id * sizeof(sge_t);
    add            r1, TXWQE_SGE_OFFSET, K_CURRENT_SGE_ID, LOG_SIZEOF_SGE_T
    // sge_p = sqcb0_p->curr_wqe_ptr + sge_offset
    add            r1, r1, K_WQE_ADDR

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqsge_process, r1)
    CAPRI_SET_TABLE_2_VALID(0)

    nop.e
    nop

trigger_stg0_sqsge_process:
    CAPRI_GET_TABLE_2_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqsge_process)
    phvwr          p.common.p4_intr_recirc, 1
    phvwr          p.common.rdma_recirc_recirc_reason, REQ_TX_RECIRC_REASON_SGE_WORK_PENDING

    nop.e
    nop
