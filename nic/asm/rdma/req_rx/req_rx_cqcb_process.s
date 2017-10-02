#include "req_rx.h"
#include "cqcb.h"

struct req_rx_phv_t p;
struct req_rx_cqcb_process_k_t k;
struct cqcb_t d;

#define CQ_PT_INFO_T    struct req_rx_cqcb_to_pt_info_t

%%
    .param  req_rx_cqpt_process

.align
req_rx_cqcb_process:

    add             r7, r0, k.args.tbl_id

    seq             c1, CQ_P_INDEX, 0
    // flip the color if cq is wrap around
    add             r1, r0, CQ_COLOR
    mincr.c1        r1, 1, 1     
    tblwr.c1        CQ_COLOR, r1

    // set the color in cqwqe
    phvwr           p.cqwqe.color, r1

    // page_index = p_index >> (log_rq_page_size - log_wqe_size)
    add             r1, r0, CQ_P_INDEX
    sub             r2, d.log_cq_page_size, d.log_wqe_size
    srlv            r3, r1, r2

    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr           r1, r2, r0
    add             r2, r0, d.log_wqe_size
    sllv            r1, r1, r2

    // r3 has page_index, r1 has page_offset by now

    // page_seg_offset = page_index & 0x7
    and     r4, r1, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    sub     r3, r3, r4
    // page_index = page_index * sizeof(u64)
    sll     r3, r3, CAPRI_LOG_SIZEOF_U64
    // page_index += cqcb_p->pt_base_addr
    add     r3, r3, d.pt_base_addr
    // now r3 has page_p to load
    
    CAPRI_GET_TABLE_I_K_AND_ARG(req_rx_phv_t, r7, r6, r5)

    CAPRI_SET_FIELD(r5, CQ_PT_INFO_T, tbl_id, r7)
    CAPRI_SET_FIELD(r5, CQ_PT_INFO_T, arm, d.arm)
    CAPRI_SET_FIELD(r5, CQ_PT_INFO_T, cq_id, d.cq_id)
    CAPRI_SET_FIELD(r5, CQ_PT_INFO_T, eq_id, d.eq_id)
    CAPRI_SET_FIELD(r5, CQ_PT_INFO_T, dma_cmd_index, k.args.dma_cmd_index)
    CAPRI_SET_FIELD(r5, CQ_PT_INFO_T, page_seg_offset, r4)
    CAPRI_SET_FIELD(r5, CQ_PT_INFO_T, page_offset, r1)

    CAPRI_SET_RAW_TABLE_PC(r2, req_rx_cqpt_process)
    CAPRI_NEXT_TABLE_I_READ(r6, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r2, r3)

    // increment p_index
    tblmincri       CQ_P_INDEX, d.log_num_wqes, 1
    // if arm, disarm.
    seq             c2, d.arm, 1
    tblwr.c2        d.arm, 0

    nop.e
    nop
