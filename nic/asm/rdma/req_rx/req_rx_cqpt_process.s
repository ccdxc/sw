#include "req_rx.h"
#include "cqcb.h"

struct req_rx_phv_t p;
struct req_rx_s6_t2_k k;

#define CQCB_PA_ADDR    r2
#define CQCB_PA_INDEX   r2
#define PAGE_ADDR_P     r1
    
//TODO: verify if it is in right order
//#define PHV_CQWQE_START cqwqe.id.wrid
//#define PHV_CQWQE_END   cqwqe.r_key

#define IN_P t2_s2s_cqcb_to_pt_info

#define EQ_INFO_P t2_s2s_cqcb_to_eq_info

#define K_PAGE_SEG_OFFSET CAPRI_KEY_FIELD(IN_P, page_seg_offset)
#define K_PA_NEXT_INDEX   CAPRI_KEY_RANGE(IN_P, pt_next_pg_index_sbit0_ebit2, pt_next_pg_index_sbit11_ebit15)
#define K_CQCB_ADDR       CAPRI_KEY_RANGE(IN_P, cqcb_addr_sbit0_ebit4, cqcb_addr_sbit29_ebit33)
#define K_PAGE_OFFSET     CAPRI_KEY_FIELD(IN_P, page_offset)
#define K_CQ_ID           CAPRI_KEY_FIELD(IN_P, cq_id)
#define K_EQCB_ADDR       CAPRI_KEY_RANGE(IN_P, eqcb_addr_sbit0_ebit2, eqcb_addr_sbit27_ebit33)

%%
    .param  req_rx_eqcb_process

.align
req_rx_cqpt_process:

    //no_translate, no_dma
    crestore [c3, c2], CAPRI_KEY_RANGE(IN_P, no_translate, no_dma), 0x3

    bcf             [c2 & c3], fire_eqcb
    DMA_CMD_STATIC_BASE_GET(r2, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_CQ)
    
    //page_addr_p = (u64 *) (d_p + sizeof(u64) * cqcb_to_pt_info_p->page_seg_offset);
    sub             PAGE_ADDR_P, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), K_PAGE_SEG_OFFSET
    sll             PAGE_ADDR_P, PAGE_ADDR_P, CAPRI_LOG_SIZEOF_U64_BITS

    tblrdp.dx       PAGE_ADDR_P, PAGE_ADDR_P, 0, CAPRI_SIZEOF_U64_BITS
    
    // Lets cache the translated page physical address
    // *cq_cb->pt_next_pa = page_addr_p
    add             CQCB_PA_ADDR, K_CQCB_ADDR, offsetof(struct cqcb_t, pt_next_pa) 
    memwr.d         CQCB_PA_ADDR, PAGE_ADDR_P
    add             CQCB_PA_INDEX, K_CQCB_ADDR, offsetof(struct cqcb_t, pt_next_pg_index)
    memwr.h         CQCB_PA_INDEX, K_PA_NEXT_INDEX

    bcf             [c2], fire_eqcb
    DMA_CMD_STATIC_BASE_GET(r2, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_CQ) //BD Slot
    
    // cqwqe_p = (cqwqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             r1, r1, K_PAGE_OFFSET

    DMA_PHV2MEM_SETUP(r2, c1, cqwqe, cqwqe, r1)

fire_eqcb:    

    bbne CAPRI_KEY_FIELD(IN_P, fire_eqcb), 1, cqpt_exit
    add   r5, r0, K_EQCB_ADDR //BD Slot

    CAPRI_RESET_TABLE_2_ARG()

    phvwr CAPRI_PHV_FIELD(EQ_INFO_P, cq_id), K_CQ_ID
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_eqcb_process, r5)
    nop.e
    nop
    
cqpt_exit:
    DMA_SET_END_OF_CMDS(struct capri_dma_cmd_phv2mem_t, r2)
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop

