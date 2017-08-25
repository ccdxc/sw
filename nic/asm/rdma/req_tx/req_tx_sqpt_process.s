#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_sqpt_process_k_t k;

#define INFO_OUT_T struct req_tx_sqcb_to_wqe_info_t

%%
    .param    req_tx_sqwqe_process

req_tx_sqpt_process:

    // page_addr_p = (u64 *)(d_p + sizeof(u64) * sqcb_to_pt_info_p->page_seg_offset)
    sll     r1, k.args.page_seg_offset, CAPRI_LOG_SIZEOF_U64_BITS
    tblrdp  r1, r1, 0, CAPRI_SIZEOF_U64_BITS

    //wqe_p = (void *)(*page_addr_p + sqcb_to_pt_info_p->page_offset)
    add    r1, r1, k.args.page_offset

    // populate t0 stage to stage data req_tx_sqwqe_info_t for next stage
    add     r2, r0, offsetof(struct req_tx_phv_t, common.common_t0_s2s_s2s_data)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, in_progress, 0)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, log_pmtu, k.args.log_pmtu)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, li_fence_cleared, 0)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, current_sge_id, 0)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, num_valid_sges, 0)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, current_sge_offset, 0)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, wqe_addr, r1)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, rrq_p_index, k.args.rrq_p_index)
    CAPRI_SET_FIELD(r2, INFO_OUT_T, pd, k.args.pd)

    // populate t0 PC and table address
    add     r2, r0, offsetof(struct req_tx_phv_t, common.common_te0_phv_table_addr)
    CAPRI_NEXT_TABLE_I_READ(r1, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, r1)

    nop.e
    nop
