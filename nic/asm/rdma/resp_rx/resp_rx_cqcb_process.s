#include "capri.h"
#include "resp_rx.h"
#include "cqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_cqcb_process_k_t k;
struct cqcb_t d;

#define ARG_P               r5
#define PAGE_INDEX          r3
#define PAGE_OFFSET         r1
#define PAGE_SEG_OFFSET     r4
#define RQCB4_ADDR          r6

#define CQ_PT_INFO_T    struct resp_rx_cqcb_to_pt_info_t

%%
    .param  resp_rx_cqpt_process
    .param  resp_rx_stats_process

.align
resp_rx_cqcb_process:

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
    srlv            PAGE_INDEX, r1, r2

    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr           r1, r2, r0
    sll             PAGE_OFFSET, r1, d.log_wqe_size

    // r3 has page_index, r1 has page_offset by now

    // page_seg_offset = page_index & 0x7
    and     PAGE_SEG_OFFSET, PAGE_OFFSET, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    sub     PAGE_INDEX, PAGE_INDEX, PAGE_SEG_OFFSET
    // page_index = page_index * sizeof(u64)
    sll     PAGE_INDEX, PAGE_INDEX, CAPRI_LOG_SIZEOF_U64
    // page_index += cqcb_p->pt_base_addr
    add     PAGE_INDEX, PAGE_INDEX, d.pt_base_addr, PT_BASE_ADDR_SHIFT
    // now r3 has page_p to load
    
    CAPRI_GET_TABLE_2_ARG(resp_rx_phv_t, ARG_P)
    #copy fields cq_id, eq_id, and arm
    CAPRI_SET_FIELD_RANGE(ARG_P, CQ_PT_INFO_T, cq_id, arm, d.{cq_id...arm})
    CAPRI_SET_FIELD(ARG_P, CQ_PT_INFO_T, page_seg_offset, PAGE_SEG_OFFSET)
    CAPRI_SET_FIELD(ARG_P, CQ_PT_INFO_T, page_offset, PAGE_OFFSET)

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_cqpt_process, PAGE_INDEX)

    // always call stats process at stage6/table1
    CAPRI_GET_TABLE_1_ARG(resp_rx_phv_t, ARG_P)   // sets STATS_INFO_T->bubble_up to 0
    RQCB4_ADDR_GET(RQCB4_ADDR)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_stats_process, RQCB4_ADDR)

    // increment p_index
    tblmincri       CQ_P_INDEX, d.log_num_wqes, 1
    // if arm, disarm.
    seq             c2, d.arm, 1
    tblwr.c2        d.arm, 0

    //for send with imm_as_dbell, set the pindex 
    //optimizing conditional checks
    RESP_RX_UPDATE_IMM_AS_DB_DATA_WITH_PINDEX(CQ_P_INDEX_HX)

    nop.e
    nop
