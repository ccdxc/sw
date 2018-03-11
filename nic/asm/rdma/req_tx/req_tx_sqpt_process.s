#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_sqpt_process_k_t k;

#define SQCB_TO_WQE_T struct req_tx_sqcb_to_wqe_info_t
#define TO_STAGE_T struct req_tx_to_stage_t

%%
    .param    req_tx_sqwqe_process

.align
req_tx_sqpt_process:

    // big-endian
    sub           r1, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), k.args.page_seg_offset
 
    // page_addr_p = (u64 *)(d_p + sizeof(u64) * sqcb_to_pt_info_p->page_seg_offset)
    sll            r1, r1, CAPRI_LOG_SIZEOF_U64_BITS
    // big-endian
    tblrdp.dx      r1, r1, 0, CAPRI_SIZEOF_U64_BITS

    //wqe_p = (void *)(*page_addr_p + sqcb_to_pt_info_p->page_offset)
    add            r1, r1, k.args.page_offset

    // populate t0 stage to stage data req_tx_sqcb_to_wqe_info_t for next stage
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, log_pmtu, k.args.log_pmtu)
    CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD_RANGE(r7, SQCB_TO_WQE_T, poll_in_progress, color, k.{args.poll_in_progress...args.color})
    //CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, pd, k.args.pd)
    
    CAPRI_GET_STAGE_5_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.wqe_addr, r1)

    // populate t0 PC and table address
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, r1)

    nop.e
    nop
