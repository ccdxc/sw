#include "capri.h"
#include "resp_rx.h"
#include "cqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s6_t2_k k;
struct cqcb_t d;

#define PAGE_INDEX          r3
#define CQE_P               r3
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
    
#define IN_TO_S_P to_s6_cqcb_info

#define CQ_PT_INFO_P          t2_s2s_cqcb_to_pt_info
#define CQ_EQ_INFO_P          t1_s2s_cqcb_to_eq_info
#define CQ_ASYNC_EQ_INFO_P    t0_s2s_cqcb_to_eq_info

#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_LOG_NUM_CQ_ENTRIES CAPRI_KEY_FIELD(IN_TO_S_P, log_num_cq_entries)
#define K_BTH_SE CAPRI_KEY_FIELD(IN_TO_S_P, bth_se)
#define K_ASYNC_ERROR_EVENT CAPRI_KEY_FIELD(IN_TO_S_P, async_error_event)
#define K_ASYNC_EVENT CAPRI_KEY_FIELD(IN_TO_S_P, async_event)
#define K_QP_STATE CAPRI_KEY_RANGE(IN_TO_S_P, qp_state_sbit0_ebit0, qp_state_sbit1_ebit2)
#define K_FEEDBACK CAPRI_KEY_FIELD(IN_TO_S_P, feedback)

    #c1 : CQ_PROXY_PINDEX == 0
    #c2 : d.arm == 1
    #c3 : cqe_dma == True. Do cqe dma in cqcb stage.
    
%%
    .param  resp_rx_cqpt_process
    .param  resp_rx_eqcb_process
    .param  resp_rx_recirc_mpu_only_process
.align
resp_rx_cqcb_process:

    bbeq             d.cq_full, 1, error_disable_qp_using_recirc
    seq              c1, CQ_PROXY_PINDEX, 0 //BD Slot

    seq              c6, K_ASYNC_EVENT, 1
    bbeq             K_ASYNC_ERROR_EVENT, 1, report_async

    #check for CQ full
    seq              c5, CQ_PROXY_PINDEX, CQ_C_INDEX //BD Slot
    bbeq.c5          d.cq_full_hint, 1, report_cqfull_error

    add             R_CQ_PROXY_PINDEX, CQ_PROXY_PINDEX, 0 //BD Slot

    // flip the color if cq is wrap around
    tblmincri.c1    CQ_COLOR, 1, 1

    // increment p_index
    tblmincri       CQ_PROXY_PINDEX, d.log_num_wqes, 1

    crestore        [c7], K_BTH_SE, 0x1

    // check incremented p_index for cq_full_hint
    seq             c5, CQ_PROXY_PINDEX, CQ_C_INDEX
    tblwr.c5        d.cq_full_hint, 1
    tblwr.!c5       d.cq_full_hint, 0

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

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_cqpt_process, PAGE_INDEX)

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

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_CQ)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, cqe, cqe, CQE_P)
    
cq_done:
eqcb_eval:

    // set the color in cqe
    phvwr           p.cqe.color, CQ_COLOR

    #c2 - arm
    #c1 - sarm
    #c7 - bth_se
    crestore        [c2, c1], d.{arm...sarm}, 0x3
    bbeq            d.wakeup_dpath, 1, skip_eqcb
    setcf           c1, [c1 & c7]
    bcf             ![c2 | c1], skip_eqcb

eqcb_setup:
    RESP_RX_EQCB_ADDR_GET(r5, r2, d.eq_id) // BD Slot
    phvwr       CAPRI_PHV_FIELD(CQ_PT_INFO_P, fire_eqcb), 1
    tblwr       CQ_PROXY_S_PINDEX, R_CQ_PROXY_PINDEX
    tblwr.f     d.{arm...sarm}, 0

    phvwrpair   p.s1.eqwqe.code, EQE_CODE_CQ_NOTIFY, p.s1.eqwqe.type, EQE_TYPE_CQ
    phvwr       p.s1.eqwqe.qid, d.cq_id

    CAPRI_RESET_TABLE_1_ARG()
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_eqcb_process, r5) //Exit Slot
    phvwr.!c6.e CAPRI_PHV_FIELD(CQ_EQ_INFO_P, cmd_eop), 1
    bcf            [c6], report_async
    nop            // Branch Delay Slot

skip_eqcb:
eval_wakeup:

    bbne        d.wakeup_dpath, 1, skip_wakeup

    //for send with imm_as_dbell, set the pindex 
    //optimizing conditional checks
    RESP_RX_UPDATE_IMM_AS_DB_DATA_WITH_PINDEX(d.proxy_pindex) //Branch Delay Slot

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_WAKEUP_DPATH)
    RESP_RX_POST_WAKEUP_DPATH_INCR_PINDEX(DMA_CMD_BASE, 
                                          d.wakeup_lif, d.wakeup_qtype, d.wakeup_qid, d.wakeup_ring_id, 
                                          DB_ADDR, DB_DATA);

skip_wakeup:
    bcf            [c6], report_async
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_CQ) // Branch Delay Slot

    DMA_SET_END_OF_CMDS_E(struct capri_dma_cmd_phv2mem_t, DMA_CMD_BASE)
    nop //Exit Slot

report_cqfull_error:
 
    phvwrpair   p.s1.eqwqe.code, EQE_CODE_CQ_ERR_FULL, p.s1.eqwqe.type, EQE_TYPE_CQ
    phvwr       p.s1.eqwqe.qid, d.cq_id

    tblwr.f     d.cq_full, 1

report_async:
    //PHV->eq_info is filled with appropriate error type and code by this time

    CAPRI_RESET_TABLE_0_ARG()
    phvwr          CAPRI_PHV_FIELD(CQ_ASYNC_EQ_INFO_P, async_eq), 1
    
    RESP_RX_EQCB_ADDR_GET(r5, r2, RDMA_EQ_ID_ASYNC)
    CAPRI_SET_TABLE_2_VALID_C(!c6, 0)
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_eqcb_process, r5)  //Exit Slot

exit:
    nop.e
    CAPRI_SET_TABLE_2_VALID(0) //Exit slot

error_disable_qp_using_recirc:

    //If QP->state is already in ERR, ignore recircing to Error Disable
    seq         c1, K_QP_STATE, QP_STATE_ERR
    bcf         [c1], skip_recirc_error_disable

    //clear the completion flag in GLOBAL_FLAGS, so it won't invoke cqcb_process again on recirc
    phvwr.!c1   CAPRI_PHV_FIELD(phv_global_common, _completion), 0 //BD Slot
    phvwr       p.common.p4_intr_recirc, 1
    phvwr       p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_ERROR_DISABLE_QP

    //Disable stats process - as we are going to recirc
    CAPRI_SET_TABLE_3_VALID(0)

    // fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_recirc_mpu_only_process, r0)

skip_recirc_error_disable:

    // If an ACK was getting generated for this packet, make it a NAK as CQ is full and QP is 
    // going to get error disabled
    seq         c1, K_FEEDBACK, 1

    // do not overwrite nak code if it is a feedback phv
    phvwr.!c1   p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_OP_ERR)
    //fill the eqwqe
    phvwrpair   p.s1.eqwqe.code, EQE_CODE_QP_ERR, p.s1.eqwqe.type, EQE_TYPE_QP
    //post ASYCN EQ error on QP
    b           report_async
    phvwr       p.s1.eqwqe.qid, K_GLOBAL_QID
