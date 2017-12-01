#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_bktrack_sqpt_process_k_t k;

#define SQ_BKTRACK_T struct req_tx_sq_bktrack_info_t
#define TO_STAGE_T struct req_tx_to_stage_t

%%
    .param    req_tx_bktrack_sqwqe_process

.align
req_tx_bktrack_sqpt_process:

     // log_num_wqe_per_page = (log_sq_page_size - log_wqe_size)
    // page_index = (info_p->sq_c_index >> log_num_wqe_per_page)
    add            r1, r0, k.args.sq_c_index
    sub            r2, k.to_stage.bktrack.log_sq_page_size, k.to_stage.bktrack.log_wqe_size
    srlv           r3, r1, r2

    // page_offset = (info_p->sq_c_index & ((1 << log_num_wqe_per_page) - 1)) << info_p->log_wqe_size
    mincr          r1, r2, r0
    add            r2, r0, k.to_stage.bktrack.log_wqe_size
    sllv           r1, r1, r2
    
    // page_seg_offset = page_index & 0x7
    and            r2, r3, CAPRI_SEG_PAGE_MASK

    // big-endian
    sub            r2, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), r2

    // page_addr_p = (u64 *)(d_p + sizeof(u64) * sqcb_to_pt_info_p->page_seg_offset)
    sll            r2, r2, CAPRI_LOG_SIZEOF_U64_BITS
    // big-endian
    tblrdp.dx      r2, r2, 0, CAPRI_SIZEOF_U64_BITS

    //wqe_p = (void *)(*page_addr_p + sqcb_to_pt_info_p->page_offset)
    add            r1, r2, r1

    // populate t0 stage to stage data req_tx_sqpt_to_bktrack_wqe_info_t for next stage
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)

    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, tx_psn, k.args.tx_psn)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, ssn, k.args.ssn)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_c_index, k.args.sq_c_index)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_p_index, k.args.sq_p_index)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, in_progress, k.args.in_progress)
    //CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, bktrack_in_progress, k.args.bktrack_in_progress)
    CAPRI_SET_FIELD_RANGE(r7, SQ_BKTRACK_T, current_sge_offset, num_sges, k.{args.current_sge_offset...args.num_sges})

    //for now, use to_stage_args to pass the wqe_addr
    //until we organize better, copy to all stages
    CAPRI_GET_STAGE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)

    CAPRI_GET_STAGE_4_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)

    CAPRI_GET_STAGE_5_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)

    CAPRI_GET_STAGE_6_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)

    CAPRI_GET_STAGE_7_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)

    // populate t0 PC and table address
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_sqwqe_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

    nop.e
    nop
