#include "capri.h"
#include "resp_rx.h"
#include "cqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s5_t2_k k;
struct cqcb_t d;

#define ARG_P               r5
#define PAGE_INDEX          r3
#define CQWQE_P             r3
#define PT_PINDEX           r1
#define PAGE_OFFSET         r1
#define PAGE_SEG_OFFSET     r4
#define RQCB4_ADDR          r6
#define CQCB_ADDR           r6
#define PG_NEXT_INDEX       r6    

#define DMA_CMD_BASE        r6
#define NUM_LOG_PAGES       r6    
#define DB_ADDR             r1
#define DB_DATA             r2
#define NUM_LOG_WQE         r2    
    
#define IN_TO_S_P to_s5_cqcb_info

#define CQ_PT_INFO_P    t2_s2s_cqcb_to_pt_info

#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_LOG_NUM_CQ_ENTRIES CAPRI_KEY_FIELD(IN_TO_S_P, log_num_cq_entries)
#define K_BTH_SE CAPRI_KEY_FIELD(IN_TO_S_P, bth_se)

    #c1 : d.proxy_pindex == 0
    #c2 : d.arm == 1
    #c3 : cqwqe_dma == True. Do cqwqe dma in cqcb stage.
    
%%
    .param  resp_rx_cqpt_process
.align
resp_rx_cqcb_process:

    // Pin cqcb process to stage 5 as it runs in stage 5 in resp_rx path
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_5
    bcf              [!c1], bubble_to_next_stage

    // if completion is not necessary, die down
    bbeq    K_GLOBAL_FLAG(_completion), 0, exit

    #Initialize c3(no_dma) to False
    setcf            c3, [!c0] //BD Slot

    seq             c1, d.proxy_pindex, 0
    // flip the color if cq is wrap around
    tblmincri.c1    CQ_COLOR, 1, 1

    // set the color in cqwqe
    phvwr           p.cqwqe.color, CQ_COLOR

    /* get the page index corresponding to p_index */
    sub             NUM_LOG_WQE, d.log_cq_page_size, d.log_wqe_size
    srlv            PAGE_INDEX, d.proxy_pindex, NUM_LOG_WQE
    
    add             r1, d.pt_pg_index, 0
    beq             r1, PAGE_INDEX, no_translate_dma
    add             r1, d.proxy_pindex, 0  //BD slot

    //Compute the number of pages of CQ
    add             NUM_LOG_PAGES, d.log_num_wqes, d.log_wqe_size
    sub             NUM_LOG_PAGES, NUM_LOG_PAGES, d.log_cq_page_size
    
    add             r1, d.pt_next_pg_index, 0   
    beq             r1, PAGE_INDEX, translate_next
    add             PT_PINDEX, r0, d.pt_next_pg_index //Branch delay slot
    b               fire_cqpt
    add             PT_PINDEX, r0, d.proxy_pindex //Branch delay slot    

translate_next:

    tblwr          d.pt_pa, d.pt_next_pa
    tblwr          d.pt_pg_index, d.pt_next_pg_index

    mincr          PT_PINDEX, NUM_LOG_PAGES, 1
    sll            PT_PINDEX, PT_PINDEX, NUM_LOG_WQE
    setcf          c3, [c0]
    
fire_cqpt:
    // page_index = p_index >> (log_rq_page_size - log_wqe_size)
    add             r1, r0, PT_PINDEX
    srlv            PAGE_INDEX, r1, NUM_LOG_WQE

    CAPRI_RESET_TABLE_2_ARG()
    phvwrpair   CAPRI_PHV_FIELD(CQ_PT_INFO_P, no_translate), 0, \
                CAPRI_PHV_FIELD(CQ_PT_INFO_P, pt_next_pg_index), PAGE_INDEX
    
    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr           r1, NUM_LOG_WQE, r0
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

    
    phvwrpair   CAPRI_PHV_FIELD(CQ_PT_INFO_P, page_offset), PAGE_OFFSET, \
                CAPRI_PHV_FIELD(CQ_PT_INFO_P, page_seg_offset), PAGE_SEG_OFFSET
    CAPRI_SET_FIELD2_C(CQ_PT_INFO_P, no_dma, 1, c3)    
    
    mfspr       CQCB_ADDR, spr_tbladdr
    phvwrpair   CAPRI_PHV_FIELD(CQ_PT_INFO_P, cq_id), \
                d.cq_id, \
                CAPRI_PHV_FIELD(CQ_PT_INFO_P, cqcb_addr), \
                CQCB_ADDR
    
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_cqpt_process, PAGE_INDEX)

    bcf     [!c3], incr_pindex
    nop
    b       do_dma
    add             r1, r0, d.proxy_pindex
    
no_translate_dma:
    
    CAPRI_RESET_TABLE_2_ARG()
    phvwrpair   CAPRI_PHV_FIELD(CQ_PT_INFO_P, cq_id), \
                d.cq_id, \
                CAPRI_PHV_RANGE(CQ_PT_INFO_P, no_translate, no_dma), \
                3
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_cqpt_process, r0)
    
do_dma:

    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    //r1 has d.proxy_pindex by the time we reach here
    mincr           r1, NUM_LOG_WQE, r0
    sll             PAGE_OFFSET, r1, d.log_wqe_size

    // cqwqe_p = (cqwqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             CQWQE_P, d.pt_pa, PAGE_OFFSET

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_CQ)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, cqwqe, cqwqe, CQWQE_P)
    
incr_pindex: 

eqcb_eval:

    //if (wakeup_dpath == 1), do not fire_eqcb
    bbeq            d.wakeup_dpath, 1, skip_eqcb

    #c6 is used to specify whether to fire eqcb or not(1: fire, 0: no)
    setcf           c6, [!c0] //BD Slot

    #c2 - arm   
    #c1 - sarm  
    crestore        [c2, c1], d.{arm...sarm}, 0x3

    //if (arm = 1), fire_eqcb
    bcf             [c2], eqcb_setup
    setcf.c2        c6, [c0] //BD Slot

    //if (sarm == 1) && (arm = 0) && (bth_se == 1), fire_eqcb
    bbeq.c1         CAPRI_KEY_FIELD(IN_TO_S_P, bth_se), 1, eqcb_setup
    setcf           c6, [c0] //BD Slot

    setcf           c6, [!c0]

eqcb_setup:

    RESP_RX_EQCB_ADDR_GET(r5, r2, d.eq_id) // BD Slot
    phvwr.c6       CAPRI_PHV_FIELD(CQ_PT_INFO_P, fire_eqcb), 1
    phvwr.c6       CAPRI_PHV_FIELD(CQ_PT_INFO_P, eqcb_addr), r5

skip_eqcb:

    // increment p_index
    tblmincri       d.proxy_pindex, d.log_num_wqes, 1
    crestore        [c1], CAPRI_KEY_FIELD(IN_TO_S_P, bth_se), 0x1
    tblwr.c1        d.proxy_s_pindex, d.proxy_pindex

    tblwr.c6    d.{arm...sarm}, 0

    bbne        d.wakeup_dpath, 1, skip_wakeup

    //for send with imm_as_dbell, set the pindex 
    //optimizing conditional checks
    RESP_RX_UPDATE_IMM_AS_DB_DATA_WITH_PINDEX(d.{proxy_pindex}.hx) //Branch Delay Slot

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_WAKEUP_DPATH)
    RESP_RX_POST_WAKEUP_DPATH_INCR_PINDEX(DMA_CMD_BASE, 
                                          d.wakeup_lif, d.wakeup_qtype, d.wakeup_qid, d.wakeup_ring_id, 
                                          DB_ADDR, DB_DATA);
    DMA_SET_END_OF_CMDS(struct capri_dma_cmd_pkt2mem_t, DMA_CMD_BASE)

bubble_to_next_stage:

skip_wakeup:
    nop.e
    nop

exit:
    nop.e
    CAPRI_SET_TABLE_2_VALID(0) //Exit slot
