# include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_sqsge_process_k_t k;

#define WQE_TO_SGE_T struct req_tx_wqe_to_sge_info_t

%%
    .param    req_tx_sqsge_process

.align
req_tx_sqsge_iterate_process:
    mfspr          r1, spr_mpuid
    add            r1, r1[6:2], r0
    
    beqi           r1, STAGE_7, trigger_stg0_sqsge_process
    nop            // Branch Delay Slot

    beqi           r1, STAGE_2, trigger_stg3_sqsge_process
    nop            // Branch Delay Slot

    //CAPRI_GET_TABLE_3_ARG_NO_RESET(req_tx_phv_t, r7)

    //add              r1, k.args.bubble_count, r0
    //sub              r1, r1, 1

    //beqi             r1, 0, trigger_sqsge_process
    //CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, bubble_count, r1) // Branch Delay Slot

    nop.e
    nop

trigger_stg3_sqsge_process:
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7) // Branch Delay Slot
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqsge_process)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6) // Branch Delay Slot
    CAPRI_SET_TABLE_3_VALID(0)

    nop.e
    nop

trigger_stg0_sqsge_process:
    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7) // Branch Delay Slot
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqsge_process)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6) // Branch Delay Slot
    phvwr          p.common.p4_intr_recirc, 1
    phvwr          p.common.rdma_recirc_recirc_reason, REQ_TX_RECIRC_REASON_SGE_WORK_PENDING

    nop.e
    nop
