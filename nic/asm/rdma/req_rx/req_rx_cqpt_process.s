#include "req_rx.h"
#include "cqcb.h"

struct req_rx_phv_t p;
struct req_rx_s5_t2_k k;

#define CQCB_PA_ADDR    r2
#define CQCB_PA_INDEX   r2
#define PAGE_ADDR_P     r1
    
//TODO: verify if it is in right order
//#define PHV_CQWQE_START cqwqe.id.wrid
//#define PHV_CQWQE_END   cqwqe.r_key

#define IN_P t2_s2s_cqcb_to_pt_info
#define IN_TO_S_P to_s5_to_stage

#define EQ_INFO_P t2_s2s_cqcb_to_eq_info
#define TMP r3

#define K_PAGE_SEG_OFFSET CAPRI_KEY_FIELD(IN_P, page_seg_offset)
#define K_PA_NEXT_INDEX   CAPRI_KEY_RANGE(IN_P, pa_next_index_sbit0_ebit0, pa_next_index_sbit9_ebit15)
#define K_CQCB_ADDR       CAPRI_KEY_RANGE(IN_P, cqcb_addr_sbit0_ebit2, cqcb_addr_sbit27_ebit33)
#define K_PAGE_OFFSET     CAPRI_KEY_FIELD(IN_P, page_offset)
#define K_EQ_ID           CAPRI_KEY_FIELD(IN_P, eq_id)
#define K_CQ_ID           CAPRI_KEY_FIELD(IN_P, cq_id)

#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_LOG_NUM_CQ_ENTRIES CAPRI_KEY_FIELD(IN_TO_S_P, log_num_cq_entries)

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

    // Lets cache the translated page physical address
    // *cq_cb->pt_next_pa = page_addr_p
    add             CQCB_PA_ADDR, K_CQCB_ADDR, offsetof(struct cqcb_t, pt_next_pa) 
    memwr.d         CQCB_PA_ADDR, PAGE_ADDR_P
    add             CQCB_PA_INDEX, K_CQCB_ADDR, offsetof(struct cqcb_t, pt_next_pa_index)
    memwr.h         CQCB_PA_INDEX, K_PA_NEXT_INDEX

    bcf             [c2], fire_eqcb
    DMA_CMD_STATIC_BASE_GET(r2, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_CQ)
    
    tblrdp.dx       r1, r1, 0, CAPRI_SIZEOF_U64_BITS

    // cqwqe_p = (cqwqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             r1, r1, K_PAGE_OFFSET

    DMA_PHV2MEM_SETUP(r2, c1, cqwqe, cqwqe, r1)

fire_eqcb:    

    //arm, wakeup_dpath
    crestore        [c2, c1], CAPRI_KEY_RANGE(IN_P, arm, wakeup_dpath), 0x3
    bcf             [c1 | !c2], cqpt_exit
    
    REQ_RX_EQCB_ADDR_GET(r5, TMP, K_EQ_ID, K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES) // BD Slot

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

