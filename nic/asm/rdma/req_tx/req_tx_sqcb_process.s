#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct req_tx_s0_k_t k;

#define INFO_OUT1_T struct req_tx_sqcb_to_pt_info_t
#define INFO_OUT2_T struct req_tx_sqcb_to_wqe_info_t

%%
    .param    req_tx_sqpt_process
    .param    req_tx_sqwqe_process

req_tx_sqcb_process:
    // copy intrinsic to global
    add      r1, r0, offsetof(struct phv_, common_global_global_data)

    // enable below code after spr_tbladdr special purpose register is available in capsim
    #mfspr r1, spr_tbladdr
    #srl r1, r1, SQCB_ADDR_SHIFT
    #CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, cb_addr, r1)

    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, k.args.lif)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, k.args.qtype)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, k.args.qid)

    add      r1, r0, SQ_C_INDEX
    // log_num_wqe_per_page = (log_sq_page_size - log_wqe_size)
    sub      r2, d.log_sq_page_size, d.log_wqe_size
    // page_index = sq_c_index >> (log_num_wqe_per_page)
    srlv     r3, r1, r2
    // page_offset = ((sq_c_index & ((1 << log_num_wqe_per_page) -1)) << log_wqe_size)
    mincr    r1, r2, r0
    add      r2, r0, d.log_wqe_size
    sllv     r1, r1, r2
    // page_seg_offset = page_index & 0x7
    and      r2, r3, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    // page_p = sqcb_p->pt_base_addr + (page_index * sizoef(u64))
    sub      r3, r3, r2
    sll      r3, r3, CAPRI_LOG_SIZEOF_U64
    add      r3, r3, d.pt_base_addr
    // remaining_payload_bytes = (1 << sqcb0_p->log_pmtu), to start with
    add      r4, r0, d.log_pmtu
    sllv     r4, 1, r4

    // populate t0 stage to stage data req_tx_sqcb_to_wqe_info_t for next stage
    add      r1, r0, offsetof(struct req_tx_phv_t, common.common_t0_s2s_s2s_data)
    CAPRI_SET_FIELD(r1, INFO_OUT1_T, page_offset, r1)
    CAPRI_SET_FIELD(r1, INFO_OUT1_T, remaining_payload_bytes, r4)
    CAPRI_SET_FIELD(r1, INFO_OUT1_T, rrq_p_index, RRQ_P_INDEX)
    CAPRI_SET_FIELD(r1, INFO_OUT1_T, log_pmtu, d.log_pmtu)
    CAPRI_SET_FIELD(r1, INFO_OUT1_T, page_seg_offset, r2)
    CAPRI_SET_FIELD(r1, INFO_OUT1_T, pd, d.pd)

    // populate t0 PC and table address
    add r1, r0, offsetof(struct req_tx_phv_t, common.common_te0_phv_table_addr)
    CAPRI_NEXT_TABLE_I_READ(r1, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqpt_process, r3)

    seq         c1, d.fence, 1
    bcf         [c1], fence
    nop
    nop.e
    nop

fence:
    add r1, r0, offsetof(struct req_tx_phv_t, common.common_t0_s2s_s2s_data)

    CAPRI_SET_FIELD(r1, INFO_OUT2_T, in_progress, 0)
    CAPRI_SET_FIELD(r1, INFO_OUT2_T, current_sge_id, 0)
    CAPRI_SET_FIELD(r1, INFO_OUT2_T, current_sge_offset, 0)
    CAPRI_SET_FIELD(r1, INFO_OUT2_T, num_valid_sges, 0)
    CAPRI_SET_FIELD(r1, INFO_OUT2_T, remaining_payload_bytes, 0)
    //CAPRI_SET_FIELD(r1, INFO_OUT2_T, wqe_ptr, d.curr_wqe_ptr)
    CAPRI_SET_FIELD(r1, INFO_OUT2_T, rrq_p_index, RRQ_P_INDEX)
    CAPRI_SET_FIELD(r1, INFO_OUT2_T, log_pmtu, d.log_pmtu)
    CAPRI_SET_FIELD(r1, INFO_OUT2_T, li_fence_cleared, 0)

    add r1, r0, offsetof(struct req_tx_phv_t, common.common_te0_phv_table_addr)
    CAPRI_NEXT_TABLE_I_READ(r1, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, d.curr_wqe_ptr)

    nop.e
    nop

