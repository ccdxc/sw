#include "capri.h"
#include "resp_rx.h"
#include "cqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s4_t2_k k;
struct cqcb_t d;

#define ARG_P               r5
#define PAGE_INDEX          r3
#define CQWQE_P             r3
#define PT_PINDEX           r1
#define PAGE_OFFSET         r1
#define PAGE_SEG_OFFSET     r4
#define RQCB4_ADDR          r6
#define CQCB_ADDR           r6
#define PA_NEXT_INDEX       r6    

#define DMA_CMD_BASE        r6
#define DB_ADDR             r1
#define DB_DATA             r2
#define NUM_LOG_WQE         r2    
    
#define CQ_PT_INFO_P    t2_s2s_cqcb_to_pt_info

    #c1 : CQ_P_INDEX == 0
    #c2 : d.arm == 1
    #c3 : cqwqe_dma == True. Do cqwqe dma in cqcb stage.
    
%%
    .param  resp_rx_cqpt_process
.align
resp_rx_cqcb_process:

    // if completion is not necessary, die down
    bbeq    K_GLOBAL_FLAG(_completion), 0, exit

    seq             c1, CQ_P_INDEX, 0   //BD Slot
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
    srlv            PAGE_INDEX, r1, NUM_LOG_WQE

    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr           r1, NUM_LOG_WQE, r0
    sll             PAGE_OFFSET, r1, d.log_wqe_size

    //next_p_index = (PAGE_INDEX+1) << (log_cq_page_size - log_wqe_size)
    add             PA_NEXT_INDEX, PAGE_INDEX, 1
    sll             PA_NEXT_INDEX, PA_NEXT_INDEX, NUM_LOG_WQE
        
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

    
    CAPRI_RESET_TABLE_2_ARG()
    #copy fields cq_id, eq_id, and arm
    phvwrpair   CAPRI_PHV_FIELD(CQ_PT_INFO_P, page_offset), PAGE_OFFSET, \
                CAPRI_PHV_FIELD(CQ_PT_INFO_P, page_seg_offset), PAGE_SEG_OFFSET
    phvwrpair   CAPRI_PHV_FIELD(CQ_PT_INFO_P, no_translate), 0, \
                CAPRI_PHV_FIELD(CQ_PT_INFO_P, pa_next_index), PA_NEXT_INDEX
    CAPRI_SET_FIELD2_C(CQ_PT_INFO_P, no_dma, 1, c3)    
    
    mfspr       CQCB_ADDR, spr_tbladdr
    phvwrpair   CAPRI_PHV_RANGE(CQ_PT_INFO_P, cq_id, wakeup_dpath), \
                d.{cq_id...wakeup_dpath}, \
                CAPRI_PHV_FIELD(CQ_PT_INFO_P, cqcb_addr), \
                CQCB_ADDR
    
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_cqpt_process, PAGE_INDEX)

    bcf     [!c3], incr_pindex
    nop
    b       do_dma
    add             r1, r0, CQ_P_INDEX
    
no_translate_dma:
    
    CAPRI_RESET_TABLE_2_ARG()
    #copy fields cq_id, eq_id, and arm
    phvwrpair   CAPRI_PHV_RANGE(CQ_PT_INFO_P, cq_id, wakeup_dpath), \
                d.{cq_id...wakeup_dpath}, \
                CAPRI_PHV_RANGE(CQ_PT_INFO_P, no_translate, no_dma), \
                3
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_cqpt_process, r0)
    
do_dma:

    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    //r1 has CQ_P_INDEX by the time we reach here
    mincr           r1, NUM_LOG_WQE, r0
    sll             PAGE_OFFSET, r1, d.log_wqe_size

    // cqwqe_p = (cqwqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             CQWQE_P, d.pt_pa, PAGE_OFFSET

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_CQ)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, cqwqe, cqwqe, CQWQE_P)
    
incr_pindex: 
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
