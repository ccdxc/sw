#include "req_rx.h"
#include "cqcb.h"

struct req_rx_phv_t p;
struct req_rx_s4_t2_k k;
struct cqcb_t d;

#define NUM_LOG_WQE         r2
#define PAGE_INDEX          r3
#define PT_PINDEX           r1
#define PAGE_OFFSET         r1
#define PAGE_SEG_OFFSET     r4
#define CQCB_ADDR           r4
#define ARG_P               r5
    
#define IN_P t2_s2s_rrqwqe_to_cq_info
#define IN_TO_S_P to_s4_to_stage

#define CQ_PT_INFO_P    t2_s2s_cqcb_to_pt_info

#define K_CQCB_BASE_ADDR_PAGE_ID CAPRI_KEY_RANGE(IN_TO_S_P, cqcb_base_addr_page_id_sbit0_ebit15, cqcb_base_addr_page_id_sbit16_ebit21)
#define K_CQ_ID CAPRI_KEY_RANGE(IN_P, cq_id_sbit0_ebit15, cq_id_sbit16_ebit23)
    
%%
    .param  req_rx_cqpt_process

.align
req_rx_cqcb_process:

    // Pin cqcb process to stage 4 as it runs in stage 4 in resp_rx path
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_4
    bcf              [!c1], bubble_to_next_stage

    seq             c1, CQ_P_INDEX, 0
    // flip the color if cq is wrap around
    tblmincri.c1    CQ_COLOR, 1, 1

    // set the color in cqwqe
    phvwr           p.cqwqe.color, CQ_COLOR

    add             r1, CQ_P_INDEX, 0
    add             r2, d.pt_pa_index, 0
    blt             r1, r2, no_translate_dma
    sub             NUM_LOG_WQE, d.log_cq_page_size, d.log_wqe_size //BD slot
    add             r2, d.pt_next_pa_index, 0   
    blt             r1, r2, translate_next 
    crestore        [c3], 0x0, 0x4            //Branch delay slot    
    b               fire_cqpt
    add             PT_PINDEX, r0, CQ_P_INDEX //Branch delay slot    

translate_next:

    tblwr          d.pt_pa, d.pt_next_pa
    tblwr          d.pt_pa_index, d.pt_next_pa_index

    add             PT_PINDEX, r0, d.pt_next_pa_index 

    crestore        [c3], 0x4, 0x4
    
fire_cqpt:
    
    // page_index = p_index >> (log_rq_page_size - log_wqe_size)
    add             r1, r0, PT_PINDEX
    sub             NUM_LOG_WQE, d.log_cq_page_size, d.log_wqe_size
    srlv            r3, r1, NUM_LOG_WQE

    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr           r1, NUM_LOG_WQE, r0
    sll             r1, r1, d.log_wqe_size

    // r3 has page_index, r1 has page_offset by now

    //next_p_index = (PAGE_INDEX+1) << (log_cq_page_size - log_wqe_size)
    add             r4, r3, 1
    sll             r2, r4, NUM_LOG_WQE
    
    // page_seg_offset = page_index & 0x7
    and     r4, r1, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    sub     r3, r3, r4
    // page_index = page_index * sizeof(u64)
    sll     r3, r3, CAPRI_LOG_SIZEOF_U64
    // page_index += cqcb_p->pt_base_addr
    add     r3, r3, d.pt_base_addr, PT_BASE_ADDR_SHIFT
    // now r3 has page_p to load
    
    CAPRI_RESET_TABLE_2_ARG()
    
    CAPRI_SET_FIELD_RANGE2(CQ_PT_INFO_P, cq_id, wakeup_dpath, d.{cq_id...wakeup_dpath})
    CAPRI_SET_FIELD2(CQ_PT_INFO_P, page_seg_offset, r4)
    CAPRI_SET_FIELD2(CQ_PT_INFO_P, page_offset, r1)
    CAPRI_SET_FIELD2(CQ_PT_INFO_P, no_translate, 0)
    CAPRI_SET_FIELD2_C(CQ_PT_INFO_P, no_dma, 1, c3)    
    CAPRI_SET_FIELD2(CQ_PT_INFO_P, pa_next_index, r2)  
    
    mfspr          CQCB_ADDR, spr_tbladdr
    CAPRI_SET_FIELD2(CQ_PT_INFO_P, cqcb_addr, CQCB_ADDR)
    
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_cqpt_process, r3)

    bcf     [!c3], incr_pindex
    nop
    b       do_dma
    add             r1, r0, CQ_P_INDEX
    
no_translate_dma:

    CAPRI_RESET_TABLE_2_ARG()
    //cq_id, eq_id, arm, wakeup_dpath
    CAPRI_SET_FIELD_RANGE2(CQ_PT_INFO_P, cq_id, wakeup_dpath, d.{cq_id...wakeup_dpath})
    CAPRI_SET_FIELD_RANGE2(CQ_PT_INFO_P, no_translate, no_dma, 0x3)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_cqpt_process, r0)
    
do_dma:

    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    //r1 has CQ_P_INDEX by the time we reach here
    mincr           r1, NUM_LOG_WQE, r0
    sll             r1, r1, d.log_wqe_size

    // cqwqe_p = (cqwqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             r1, d.pt_pa, r1

    DMA_CMD_STATIC_BASE_GET(r2, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_CQ)    
    DMA_PHV2MEM_SETUP(r2, c1, cqwqe, cqwqe, r1)    
    
incr_pindex: 
    
    // increment p_index
    tblmincri       CQ_P_INDEX, d.log_num_wqes, 1
    // if arm, disarm.
    seq             c2, d.arm, 1

    bbne        d.wakeup_dpath, 1, skip_wakeup
    tblwr.c2    d.arm, 0 //Branch Delay Slot

    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_WAKEUP_DPATH)
    PREPARE_DOORBELL_INC_PINDEX(d.wakeup_lif, d.wakeup_qtype, d.wakeup_qid, d.wakeup_ring_id, r1, r2)
    phvwr          p.wakeup_dpath_data, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, wakeup_dpath_data, wakeup_dpath_data, r1)
    DMA_SET_END_OF_CMDS(struct capri_dma_cmd_phv2mem_t, r6)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_3    
    bcf         [!c1], exit

    //invoke the same routine, but with valid d[]
    CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
    REQ_RX_CQCB_ADDR_GET(r1, K_CQ_ID, K_CQCB_BASE_ADDR_PAGE_ID)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

skip_wakeup:
    nop.e
    nop

exit:
    nop.e
    nop
