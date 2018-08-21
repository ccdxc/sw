#include "req_rx.h"
#include "cqcb.h"

struct req_rx_phv_t p;
struct req_rx_s6_t2_k k;
struct cqcb_t d;

#define NUM_LOG_WQE         r2
#define PAGE_INDEX          r3
#define PT_PINDEX           r1
#define PAGE_OFFSET         r1
#define PAGE_SEG_OFFSET     r4
#define CQCB_ADDR           r4
#define ARG_P               r5
#define NUM_LOG_PAGES       r6
    
#define IN_P t2_s2s_rrqwqe_to_cq_info
#define IN_TO_S_P to_s6_cq_info

#define CQ_PT_INFO_P    t2_s2s_cqcb_to_pt_info
#define CQ_EQ_INFO_P    t1_s2s_cqcb_to_eq_info

#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_LOG_NUM_CQ_ENTRIES CAPRI_KEY_FIELD(IN_TO_S_P, log_num_cq_entries)
#define K_BTH_SE CAPRI_KEY_FIELD(IN_TO_S_P, bth_se)

#define K_CQ_ID CAPRI_KEY_FIELD(IN_P, cq_id)
#define K_CQE_TYPE CAPRI_KEY_FIELD(IN_P, cqe_type)
    
%%
    .param  req_rx_cqpt_process
    .param  req_rx_eqcb_process
    .param  req_rx_recirc_mpu_only_process

.align
req_rx_cqcb_process:

    // Pin cqcb process to stage 6 as it runs in stage 6 in resp_rx path
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_6
    bcf              [!c1], bubble_to_next_stage
    seq              c5, CQ_PROXY_PINDEX, CQ_C_INDEX

    bbeq             d.cq_full, 1, error_disable_qp_using_recirc
    seq              c1, CQ_PROXY_PINDEX, 0 //BD Slot

    #check for CQ full
    #seq              c5, CQ_PROXY_PINDEX, CQ_C_INDEX
    bbeq.c5          d.cq_full_hint, 1, report_cqfull_error

    #Initialize c3(no_dma) to False
    setcf            c3, [!c0] //BD Slot

    tblwr            d.cq_full_hint, 0
    #seq             c1, CQ_PROXY_PINDEX, 0
    // flip the color if cq is wrap around
    tblmincri.c1    CQ_COLOR, 1, 1

    // set the color in cqe
    phvwrpair       p.cqe.type, K_CQE_TYPE, p.cqe.color, CQ_COLOR

    sub             NUM_LOG_WQE, d.log_cq_page_size, d.log_wqe_size
    srlv            r3, CQ_PROXY_PINDEX, NUM_LOG_WQE

    add             r1, d.pt_pg_index, 0
    beq             r1, r3, no_translate_dma
    add             r1, CQ_PROXY_PINDEX, 0  //BD slot

    //Compute the number of pages of CQ
    add             NUM_LOG_PAGES, d.log_num_wqes, d.log_wqe_size
    sub             NUM_LOG_PAGES, NUM_LOG_PAGES, d.log_cq_page_size
    
    add             r1, d.pt_next_pg_index, 0
    beq             r1, r3, translate_next 
    add             PT_PINDEX, r0, d.pt_next_pg_index //Branch delay slot    
    b               fire_cqpt
    add             PT_PINDEX, r0, CQ_PROXY_PINDEX //Branch delay slot    

translate_next:

    tblwr          d.pt_pa, d.pt_next_pa
    tblwr          d.pt_pg_index, d.pt_next_pg_index

    mincr          PT_PINDEX, NUM_LOG_PAGES, 1
    sll            PT_PINDEX, PT_PINDEX, NUM_LOG_WQE

    setcf          c3, [c0]
    
fire_cqpt:
    
    // page_index = p_index >> (log_rq_page_size - log_wqe_size)
    add             r1, r0, PT_PINDEX
    srlv            r3, r1, NUM_LOG_WQE

    CAPRI_RESET_TABLE_2_ARG()
    mfspr       CQCB_ADDR, spr_tbladdr
    phvwrpair CAPRI_PHV_FIELD(CQ_PT_INFO_P, cqcb_addr), CQCB_ADDR, \
              CAPRI_PHV_FIELD(CQ_PT_INFO_P, pt_next_pg_index), r3
    
    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr           r1, NUM_LOG_WQE, r0
    sll             r1, r1, d.log_wqe_size

    // r3 has page_index, r1 has page_offset by now

    // page_seg_offset = page_index & 0x7
    and     r4, r1, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    sub     r3, r3, r4
    // page_index = page_index * sizeof(u64)
    sll     r3, r3, CAPRI_LOG_SIZEOF_U64
    // page_index += cqcb_p->pt_base_addr
    add     r3, r3, d.pt_base_addr, PT_BASE_ADDR_SHIFT
    // now r3 has page_p to load
    
    phvwr     CAPRI_PHV_FIELD(CQ_PT_INFO_P, page_seg_offset), r4
    phvwrpair CAPRI_PHV_FIELD(CQ_PT_INFO_P, page_offset), r1, \
              CAPRI_PHV_FIELD(CQ_PT_INFO_P, no_translate), 0
    phvwr.c3  CAPRI_PHV_FIELD(CQ_PT_INFO_P, no_dma), 1

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_cqpt_process, r3)

    bcf     [!c3], incr_pindex
    nop
    b       do_dma
    add             r1, r0, CQ_PROXY_PINDEX
    
no_translate_dma:

    CAPRI_RESET_TABLE_2_ARG()
    phvwr     CAPRI_PHV_RANGE(CQ_PT_INFO_P, no_translate, no_dma), 0x3
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_cqpt_process, r0)
    
do_dma:

    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    //r1 has CQ_PROXY_PINDEX by the time we reach here
    mincr           r1, NUM_LOG_WQE, r0
    sll             r1, r1, d.log_wqe_size

    // cqe_p = (cqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             r1, d.pt_pa, r1

    DMA_CMD_STATIC_BASE_GET(r2, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_CQ)    
    DMA_PHV2MEM_SETUP(r2, c1, cqe, cqe, r1)    
    
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
    bbeq.c1         K_BTH_SE, 1, eqcb_setup
    setcf           c6, [c0] //BD Slot

    setcf           c6, [!c0]

eqcb_setup:
    bcf             [!c6], skip_eqcb
    REQ_RX_EQCB_ADDR_GET(r5, r2, d.eq_id, K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES) // BD Slot
    phvwr           CAPRI_PHV_FIELD(CQ_PT_INFO_P, fire_eqcb), 1
    tblwr           CQ_PROXY_S_PINDEX, CQ_PROXY_PINDEX

    phvwrpair   p.eqwqe.code, EQE_CODE_CQ_NOTIFY, p.eqwqe.type, EQE_TYPE_CQ
    phvwr       p.eqwqe.qid, d.cq_id

    CAPRI_RESET_TABLE_1_ARG()

    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_eqcb_process, r5) 

skip_eqcb:
   
    // increment p_index
    tblmincri       CQ_PROXY_PINDEX, d.log_num_wqes, 1
    crestore        [c1], CAPRI_KEY_FIELD(IN_TO_S_P, bth_se), 0x1
    tblwr.c1        CQ_PROXY_S_PINDEX, CQ_PROXY_PINDEX

    seq             c5, CQ_PROXY_PINDEX, CQ_C_INDEX
    tblwr.c5        d.cq_full_hint, 1

    bbne        d.wakeup_dpath, 1, skip_wakeup
    tblwr.c6    d.{arm...sarm}, 0 //Branch Delay Slot


    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_WAKEUP_DPATH)
    PREPARE_DOORBELL_INC_PINDEX(d.wakeup_lif, d.wakeup_qtype, d.wakeup_qid, d.wakeup_ring_id, r1, r2)
    phvwr          p.wakeup_dpath_data, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, wakeup_dpath_data, wakeup_dpath_data, r1)
    DMA_SET_END_OF_CMDS(struct capri_dma_cmd_phv2mem_t, r6)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)
    nop.e
    nop

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_5
    bcf         [!c1], exit

    //invoke the same routine, but with valid d[]
    CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
    REQ_RX_CQCB_ADDR_GET(r1, K_CQ_ID, K_CQCB_BASE_ADDR_HI)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1) //Exit Slot

report_cqfull_error:
 
    phvwrpair   p.eqwqe.code, EQE_CODE_CQ_ERR_FULL, p.eqwqe.type, EQE_TYPE_CQ
    phvwr       p.eqwqe.qid, d.cq_id

    tblwr       d.cq_full, 1

report_async:
    //PHV->eq_info is filled with appropriate error type and code by this time

    CAPRI_RESET_TABLE_1_ARG()

    REQ_RX_EQCB_ADDR_GET(r5, r2, RDMA_EQ_ID_ASYNC, K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES)
    CAPRI_SET_TABLE_2_VALID(0)
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_eqcb_process, r5) //Exit Slot

skip_wakeup:
exit:
    nop.e
    nop


error_disable_qp_using_recirc:

    //clear the completion flag in GLOBAL_FLAGS, so it won't invoke cqcb_process again on recirc
    phvwr       p.common.p4_intr_recirc, 1
    phvwr       p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_ERROR_DISABLE_QP

    //Disable other programs - as we are going to recirc
    CAPRI_SET_TABLE_1_VALID(0)
    CAPRI_SET_TABLE_2_VALID(0)
    CAPRI_SET_TABLE_3_VALID(0)

    // fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_recirc_mpu_only_process, r0)

    //fill the eqwqe
    phvwrpair   p.eqwqe.code, EQE_CODE_QP_ERR, p.eqwqe.type, EQE_TYPE_QP
    //post ASYCN EQ error on QP
    b           report_async
    phvwr       p.eqwqe.qid, K_GLOBAL_QID //BD Slot

