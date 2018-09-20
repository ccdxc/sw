#include "capri.h"
#include "resp_rx.h"
#include "cqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s7_t2_k k;

#define PAGE_ADDR_P     r1
#define CQE_P           r1
#define DMA_CMD_BASE    r2
#define TMP             r3
#define EQCB_ADDR       r5
#define CQCB_PA_ADDR    r4
#define CQCB_PA_INDEX   r4

#define ARG_P           r7

    #c2: no_dma
    #c3: no_translate
    
#define IN_P t2_s2s_cqcb_to_pt_info
#define K_PA_NEXT_INDEX   CAPRI_KEY_RANGE(IN_P, pt_next_pg_index_sbit0_ebit2, pt_next_pg_index_sbit11_ebit15)
#define K_CQCB_ADDR CAPRI_KEY_RANGE(IN_P, cqcb_addr_sbit0_ebit4, cqcb_addr_sbit29_ebit33)
#define K_EQCB_ADDR CAPRI_KEY_RANGE(IN_P, eqcb_addr_sbit0_ebit2, eqcb_addr_sbit27_ebit33)

%%
    .param  resp_rx_eqcb_process

.align
resp_rx_cqpt_process:

    bbeq            CAPRI_KEY_FIELD(IN_P, report_error), 1, fire_eqcb
    crestore [c3, c2], CAPRI_KEY_RANGE(IN_P, no_translate, no_dma), 0x3 //BD Slot

    bcf             [c2 & c3], fire_eqcb
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_CQ) //BD slot
    
    //page_addr_p = (u64 *) (d_p + sizeof(u64) * cqcb_to_pt_info_p->page_seg_offset);
    sub             PAGE_ADDR_P, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), CAPRI_KEY_FIELD(IN_P, page_seg_offset)  
    sll             PAGE_ADDR_P, PAGE_ADDR_P, CAPRI_LOG_SIZEOF_U64_BITS

    tblrdp.dx       PAGE_ADDR_P, PAGE_ADDR_P, 0, CAPRI_SIZEOF_U64_BITS
    or              PAGE_ADDR_P, PAGE_ADDR_P, 1, 63
    or              PAGE_ADDR_P, PAGE_ADDR_P, K_GLOBAL_LIF, 52
    
    // Lets cache the translated page physical address
    // *cq_cb->pt_next_pa = page_addr_p
    add             CQCB_PA_ADDR, K_CQCB_ADDR, FIELD_OFFSET(cqcb_t, pt_next_pa) 
    memwr.d         CQCB_PA_ADDR, PAGE_ADDR_P
    add             CQCB_PA_INDEX, K_CQCB_ADDR, FIELD_OFFSET(cqcb_t, pt_next_pg_index)
    //memwr.h         CQCB_PA_INDEX, CAPRI_KEY_FIELD(IN_P, pa_next_index)
    memwr.h         CQCB_PA_INDEX, K_PA_NEXT_INDEX

    bcf             [c2], fire_eqcb

        // CQE_P = (cqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             CQE_P, PAGE_ADDR_P, CAPRI_KEY_FIELD(IN_P, page_offset) //BD Slot

    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, cqe, cqe, CQE_P)

fire_eqcb:
    bbeq CAPRI_KEY_FIELD(IN_P, fire_eqcb), 1, cqpt_exit
    nop // BD Slot

    DMA_SET_END_OF_CMDS(struct capri_dma_cmd_phv2mem_t, DMA_CMD_BASE)

cqpt_exit:
    CAPRI_SET_TABLE_2_VALID_CE(c0, 0)
    nop

