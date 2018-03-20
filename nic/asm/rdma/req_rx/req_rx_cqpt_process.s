#include "req_rx.h"
#include "cqcb.h"

struct req_rx_phv_t p;
struct req_rx_cqpt_process_k_t k;

#define CQCB_PA_ADDR    r2
#define CQCB_PA_INDEX   r2
#define PAGE_ADDR_P     r1
    
//TODO: verify if it is in right order
//#define PHV_CQWQE_START cqwqe.id.wrid
//#define PHV_CQWQE_END   cqwqe.r_key

#define EQ_INFO_T struct req_rx_cqcb_to_eq_info_t
#define TMP r3

%%
    .param  req_rx_eqcb_process

.align
req_rx_cqpt_process:

    crestore [c3, c2], k.{args.no_translate...args.no_dma}, 0x3

    bcf             [c2 & c3], fire_eqcb
    DMA_CMD_STATIC_BASE_GET(r2, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_CQ)
    
    //page_addr_p = (u64 *) (d_p + sizeof(u64) * cqcb_to_pt_info_p->page_seg_offset);
    sub             PAGE_ADDR_P, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), k.args.page_seg_offset
    sll             PAGE_ADDR_P, PAGE_ADDR_P, CAPRI_LOG_SIZEOF_U64_BITS

    // Lets cache the translated page physical address
    // *cq_cb->pt_next_pa = page_addr_p
    add             CQCB_PA_ADDR, k.args.cqcb_addr, offsetof(struct cqcb_t, pt_next_pa) 
    memwr.d         CQCB_PA_ADDR, PAGE_ADDR_P
    add             CQCB_PA_INDEX, k.args.cqcb_addr, offsetof(struct cqcb_t, pt_next_pa_index)
    memwr.h         CQCB_PA_INDEX, k.args.pa_next_index

    bcf             [c2], fire_eqcb
    DMA_CMD_STATIC_BASE_GET(r2, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_CQ)
    
    tblrdp.dx       r1, r1, 0, CAPRI_SIZEOF_U64_BITS

    // cqwqe_p = (cqwqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             r1, r1, k.args.page_offset

    DMA_PHV2MEM_SETUP(r2, c1, cqwqe, cqwqe, r1)

fire_eqcb:    

    crestore        [c2, c1], k.{args.arm...args.wakeup_dpath}, 0x3
    bcf             [c1 | !c2], cqpt_exit
    
    REQ_RX_EQCB_ADDR_GET(r5, TMP, k.args.eq_id) // BD Slot

    CAPRI_GET_TABLE_2_ARG(req_rx_phv_t, r7)

    CAPRI_SET_FIELD(r7, EQ_INFO_T, cq_id, k.args.cq_id)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_eqcb_process, r5)
    nop.e
    nop
    
cqpt_exit:
    DMA_SET_END_OF_CMDS(struct capri_dma_cmd_phv2mem_t, r2)
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop

