#include "req_rx.h"
#include "cqcb.h"

struct req_rx_phv_t p;
struct req_rx_s6_t2_k k;
struct cqcb_t d;

#define IN_P t2_s2s_rrqwqe_to_cq_info
#define IN_TO_S_P to_s6_cq_info

#define PAGE_INDEX          r3
#define CQE_P               r3
#define SQCB5_ADDR          r3
#define R_CQ_PROXY_PINDEX   r1
#define PAGE_SEG_OFFSET     r4
#define CQCB_ADDR           r6

#define DMA_CMD_BASE        r6
#define NUM_LOG_PAGES       r6    
#define PT_PINDEX           r7
#define DB_ADDR             r7
#define PAGE_OFFSET         r7
#define DB_DATA             r2
#define NUM_LOG_WQE         r2    
 
#define CQ_PT_INFO_P          t2_s2s_cqcb_to_pt_info
#define CQ_EQ_INFO_P          t1_s2s_cqcb_to_eq_info
#define CQ_ASYNC_EQ_INFO_P    t0_s2s_cqcb_to_eq_info

#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_LOG_NUM_CQ_ENTRIES CAPRI_KEY_FIELD(IN_TO_S_P, log_num_cq_entries)
#define K_BTH_SE CAPRI_KEY_FIELD(IN_TO_S_P, bth_se)
#define K_ASYNC_ERROR_EVENT CAPRI_KEY_FIELD(IN_TO_S_P, async_error_event)
#define K_ASYNC_EVENT CAPRI_KEY_FIELD(IN_TO_S_P, async_event)
#define K_QP_STATE CAPRI_KEY_RANGE(IN_TO_S_P, state_sbit0_ebit0, state_sbit1_ebit2)

#define K_CQ_ID CAPRI_KEY_RANGE(IN_P, cq_id_sbit0_ebit7, cq_id_sbit8_ebit23)
#define K_CQE_TYPE CAPRI_KEY_FIELD(IN_P, cqe_type)
    
%%
    .param  req_rx_cqpt_process
    .param  req_rx_eqcb_process
    .param  req_rx_recirc_mpu_only_process
    .param  req_rx_stats_process

.align
req_rx_cqcb_process:

    // Pin cqcb process to stage 6 as it runs in stage 6 in resp_rx path
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_6
    bcf              [!c1], bubble_to_next_stage
    seq              c1, CQ_PROXY_PINDEX, 0 //BD Slot

    bbeq             d.cq_full, 1, error_disable_qp_using_recirc

    seq              c6, K_ASYNC_EVENT, 1 // BD Slot
    bbeq             K_ASYNC_ERROR_EVENT, 1, report_async

    #check for CQ full
    seq              c5, CQ_PROXY_PINDEX, CQ_C_INDEX //BD Slot
    bbeq.c5          d.cq_full_hint, 1, report_cqfull_error

    add              R_CQ_PROXY_PINDEX, CQ_PROXY_PINDEX, 0 //BD Slot

    // flip the color if cq is wrap around
    tblmincri.c1    CQ_COLOR, 1, 1

    // increment p_index
    tblmincri       CQ_PROXY_PINDEX, d.log_num_wqes, 1

    // check incremented p_index for cq_full_hint
    seq             c5, CQ_PROXY_PINDEX, CQ_C_INDEX
    tblwr.c5        d.cq_full_hint, 1
    tblwr.!c5       d.cq_full_hint, 0

    crestore        [c7], K_BTH_SE, 0x1

    tblwr.c7        CQ_PROXY_S_PINDEX, R_CQ_PROXY_PINDEX

    //if (wakeup_dpath == 1) or ((arm == 0) && (sarm == 0)), do not fire_eqcb
    seq             c4, d.wakeup_dpath, 1
    seq.!c4         c4, d.{arm...sarm}, 0x0

    /* get the page index corresponding to p_index */
    sub             NUM_LOG_WQE, d.log_cq_page_size, d.log_wqe_size
    srlv            PAGE_INDEX, R_CQ_PROXY_PINDEX, NUM_LOG_WQE
    
    add             PT_PINDEX, d.pt_pg_index, 0
    beq             PT_PINDEX, PAGE_INDEX, no_translate_dma
    #Initialize c3(no_dma) to False
    setcf            c3, [!c0] //BD Slot


    add             PT_PINDEX, d.pt_next_pg_index, 0   
    bne             PT_PINDEX, PAGE_INDEX, fire_cqpt
    CAPRI_RESET_TABLE_2_ARG() //BD SLot

translate_next:

    tblwr          d.pt_pa, d.pt_next_pa
    tblwr.c4.f     d.pt_pg_index, d.pt_next_pg_index
    tblwr.!c4      d.pt_pg_index, d.pt_next_pg_index

    //Compute the number of pages of CQ
    add            NUM_LOG_PAGES, d.log_num_wqes, d.log_wqe_size
    sub            NUM_LOG_PAGES, NUM_LOG_PAGES, d.log_cq_page_size
    
    mincr          PAGE_INDEX, NUM_LOG_PAGES, 1
    setcf          c3, [c0]
    phvwr          CAPRI_PHV_FIELD(CQ_PT_INFO_P, no_dma), 1
    
fire_cqpt:

    phvwrpair   CAPRI_PHV_FIELD(CQ_PT_INFO_P, no_translate), 0, \
                CAPRI_PHV_FIELD(CQ_PT_INFO_P, pt_next_pg_index), PAGE_INDEX
    
    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    add             r7, R_CQ_PROXY_PINDEX, r0
    mincr           r7, NUM_LOG_WQE, r0
    sll             PAGE_OFFSET, r7, d.log_wqe_size

    // r3 has page_index, r7 has page_offset by now

    // page_seg_offset = page_index & 0x7
    and     PAGE_SEG_OFFSET, PAGE_INDEX, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    sub     PAGE_INDEX, PAGE_INDEX, PAGE_SEG_OFFSET
    // page_index = page_index * sizeof(u64)
    sll     PAGE_INDEX, PAGE_INDEX, CAPRI_LOG_SIZEOF_U64
    // page_index += cqcb_p->pt_base_addr
    add     PAGE_INDEX, PAGE_INDEX, d.pt_base_addr, PT_BASE_ADDR_SHIFT
    // now r3 has page_p to load

    mfspr       CQCB_ADDR, spr_tbladdr
    phvwrpair   CAPRI_PHV_FIELD(CQ_PT_INFO_P, host_addr), d.host_addr, \
                CAPRI_PHV_FIELD(CQ_PT_INFO_P, cqcb_addr), CQCB_ADDR

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_cqpt_process, PAGE_INDEX)

    bcf     [!c3], cq_done 
    phvwrpair   CAPRI_PHV_FIELD(CQ_PT_INFO_P, page_offset), PAGE_OFFSET, \
                CAPRI_PHV_FIELD(CQ_PT_INFO_P, page_seg_offset), PAGE_SEG_OFFSET //BD Slot
    b       do_dma
    nop
    
no_translate_dma:
    
    //dummy table write to flush
    tblwr.c4.f        d.pt_pa, d.pt_pa
    CAPRI_SET_TABLE_2_VALID(0);
    
    // page_offset = p_index & ((1 << (log_cq_page_size - log_wqe_size))-1) << log_wqe_size
    add             r6, R_CQ_PROXY_PINDEX, r0
    mincr           r6, NUM_LOG_WQE, r0
    sll             PAGE_OFFSET, r6, d.log_wqe_size

do_dma:

    // CQE_P = (cqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             CQE_P, d.{pt_pa}.dx, PAGE_OFFSET

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_CQ)

    seq         c1, d.host_addr, 1
    DMA_PHV2MEM_SETUP2(DMA_CMD_BASE, c1, cqe, cqe, CQE_P)
    
cq_done:
eqcb_eval:

    SQCB5_ADDR_GET(SQCB5_ADDR)
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_stats_process, SQCB5_ADDR)

    // set the color in cqe
    phvwrpair       p.cqe.type, K_CQE_TYPE, p.cqe.color, CQ_COLOR

    #c2 - arm
    #c1 - sarm
    #c7 - bth_se
    crestore        [c2, c1], d.{arm...sarm}, 0x3
    bbeq            d.wakeup_dpath, 1, skip_eqcb
    setcf           c1, [c1 & c7]
    bcf             ![c2 | c1], skip_eqcb

eqcb_setup:
    REQ_RX_EQCB_ADDR_GET(r5, r2, d.eq_id, K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES) // BD Slot
    phvwr       CAPRI_PHV_FIELD(CQ_PT_INFO_P, fire_eqcb), 1
    tblwr       CQ_PROXY_S_PINDEX, R_CQ_PROXY_PINDEX
    tblwr.f     d.{arm...sarm}, 0

    phvwrpair   p.eqwqe.code, EQE_CODE_CQ_NOTIFY, p.eqwqe.type, EQE_TYPE_CQ
    phvwr       p.eqwqe.qid, d.cq_id

    CAPRI_RESET_TABLE_1_ARG()
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_eqcb_process, r5)
    phvwr.!c6.e CAPRI_PHV_FIELD(CQ_EQ_INFO_P, cmd_eop), 1
    bcf         [c6], report_async
    nop         // Branch Delay Slot

skip_eqcb:
eval_wakeup:

    bbne        d.wakeup_dpath, 1, skip_wakeup
    nop

    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_WAKEUP_DPATH) //BD Slot
    PREPARE_DOORBELL_INC_PINDEX(d.wakeup_lif, d.wakeup_qtype, d.wakeup_qid, d.wakeup_ring_id, r1, r2)
    phvwr          p.wakeup_dpath_data, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, wakeup_dpath_data, wakeup_dpath_data, r1)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)

skip_wakeup:
    bcf            [c6], report_async
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_CQ) // Branch Delay Slot

    DMA_SET_END_OF_CMDS_E(struct capri_dma_cmd_phv2mem_t, DMA_CMD_BASE)
    nop // Exit Slot

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_5
    bcf         [!c1], exit

    //invoke the same routine, but with valid d[]
    CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
    REQ_RX_CQCB_ADDR_GET(r1, K_CQ_ID, K_CQCB_BASE_ADDR_HI)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1) //Exit Slot

report_cqfull_error:
 
    phvwrpair   p.async_eqwqe.code, EQE_CODE_CQ_ERR_FULL, p.async_eqwqe.type, EQE_TYPE_CQ
    phvwr       p.async_eqwqe.qid, d.cq_id

    tblwr.f     d.cq_full, 1

report_async:
    //PHV->eq_info is filled with appropriate error type and code by this time

    CAPRI_RESET_TABLE_0_ARG()
    phvwr          CAPRI_PHV_FIELD(CQ_ASYNC_EQ_INFO_P, async_eq), 1

    REQ_RX_EQCB_ADDR_GET(r5, r2, RDMA_EQ_ID_ASYNC, K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES)
    CAPRI_SET_TABLE_2_VALID_C(!c6, 0)
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_eqcb_process, r5) //Exit Slot

error_disable_qp_using_recirc:

    //If QP->state is already in ERR, ignore recircing to Error Disable
    seq         c1, K_QP_STATE, QP_STATE_ERR
    bcf         [c1], skip_recirc_error_disable
    //fill the eqwqe
    phvwrpair   p.async_eqwqe.code, EQE_CODE_QP_ERR, p.async_eqwqe.type, EQE_TYPE_QP    //BD Slot

    //clear the completion flag in GLOBAL_FLAGS, so it won't invoke cqcb_process again on recirc
    phvwr       p.common.p4_intr_recirc, 1
    phvwr       p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_ERROR_DISABLE_QP

    //Disable other programs - as we are going to recirc
    CAPRI_SET_TABLE_1_VALID(0)
    CAPRI_SET_TABLE_2_VALID(0)
    CAPRI_SET_TABLE_3_VALID(0)

    // fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_recirc_mpu_only_process, r0)

skip_recirc_error_disable:
    //post ASYCN EQ error on QP
    b           report_async
    phvwr       p.async_eqwqe.qid, K_GLOBAL_QID //BD Slot

exit:
    nop.e
    nop
