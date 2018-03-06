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

#define DMA_CMD_BASE        r6
#define DB_ADDR             r1
#define DB_DATA             r2

#define CQ_PT_INFO_T    struct resp_rx_cqcb_to_pt_info_t

%%
    .param  resp_rx_cqpt_process

.align
resp_rx_cqcb_process:

    // if completion is not necessary, die down
    bbeq    k.global.flags.resp_rx._completion, 0, exit

    seq             c1, CQ_P_INDEX, 0   //BD Slot
    // flip the color if cq is wrap around
    tblmincri.c1    CQ_COLOR, 1, 1

    // set the color in cqwqe
    phvwr           p.cqwqe.color, CQ_COLOR

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
    CAPRI_SET_FIELD(ARG_P, CQ_PT_INFO_T, wakeup_dpath, d.wakeup_dpath)
    CAPRI_SET_FIELD(ARG_P, CQ_PT_INFO_T, page_seg_offset, PAGE_SEG_OFFSET)
    CAPRI_SET_FIELD(ARG_P, CQ_PT_INFO_T, page_offset, PAGE_OFFSET)

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_cqpt_process, PAGE_INDEX)

    // increment p_index
    tblmincri       CQ_P_INDEX, d.log_num_wqes, 1
    // if arm, disarm.
    seq             c2, d.arm, 1
    tblwr.c2        d.arm, 0

    bbne            d.wakeup_dpath, 1, skip_wakeup

    //for send with imm_as_dbell, set the pindex 
    //optimizing conditional checks
    RESP_RX_UPDATE_IMM_AS_DB_DATA_WITH_PINDEX(CQ_P_INDEX_HX) //Branch Delay Slot

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_WAKEUP_DPATH)
    RESP_RX_POST_WAKEUP_DPATH_INCR_PINDEX(DMA_CMD_BASE, 
                                          d.wakeup_lif, d.wakeup_qtype, d.wakeup_qid, d.wakeup_ring_id, 
                                          DB_ADDR, DB_DATA);
    DMA_SET_END_OF_CMDS(struct capri_dma_cmd_pkt2mem_t, DMA_CMD_BASE)

skip_wakeup:
    nop.e
    nop

exit:
    nop.e
    CAPRI_SET_TABLE_2_VALID(0) //Exit slot
