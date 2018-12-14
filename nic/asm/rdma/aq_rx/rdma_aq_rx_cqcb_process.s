#include "aq_rx.h"
#include "cqcb.h"
#include "aqcb.h"

struct aq_rx_phv_t p;
struct cqcb_t d;
struct aq_rx_s6_t2_k k;

#define IN_P t2_s2s_aqcb_to_cq_info
#define IN_TO_S_P to_s6_info

#define PAGE_INDEX          r3
#define CQE_P               r3
#define R_CQ_PROXY_PINDEX   r1
#define PAGE_SEG_OFFSET     r4
#define CQCB_ADDR           r6
#define DMA_CMD_BASE        r6
#define NUM_LOG_PAGES       r6
#define PT_PINDEX           r7
#define PAGE_OFFSET         r7
#define NUM_LOG_WQE         r2

#define CQ_EQ_INFO_P          t1_s2s_cqcb_to_eq_info
#define CQ_ASYNC_EQ_INFO_P    t0_s2s_cqcb_to_eq_info

#define IN_TO_S_P  to_s6_info
    
#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, cqcb_base_addr_hi)
#define K_LOG_NUM_CQ_ENTRIES CAPRI_KEY_FIELD(IN_TO_S_P, log_num_cq_entries)
#define K_AQCB_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, aqcb_addr_sbit0_ebit3, aqcb_addr_sbit4_ebit27)
#define K_CQ_ID CAPRI_KEY_FIELD(IN_TO_S_P, cq_id)
#define K_ERROR CAPRI_KEY_FIELD(IN_TO_S_P, error)
#define K_STATUS CAPRI_KEY_FIELD(IN_TO_S_P, status)
#define K_WQE_ID CAPRI_KEY_RANGE(IN_TO_S_P, wqe_id_sbit0_ebit7, wqe_id_sbit8_ebit15)
    
%%
    .param  rdma_aq_rx_eqcb_process

.align
rdma_aq_rx_cqcb_process:

    seq              c1, CQ_PROXY_PINDEX, 0 //BD Slot

    bbeq             d.cq_full, 1, error_disable_aq

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

    seq             c4, d.arm, 1

    CAPRI_RESET_TABLE_2_ARG()

    CAPRI_SET_TABLE_2_VALID(0);

    // page_offset = p_index << log_wqe_size
    add             r6, R_CQ_PROXY_PINDEX, r0
    sll             PAGE_OFFSET, r6, d.log_wqe_size

    // CQE_P = (cqe_t *)(*page_addr_p + cqcb_to_pt_info_p->page_offset);
    add             CQE_P, d.{pt_pa}.dx, PAGE_OFFSET

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CQ)

    seq         c1, d.host_addr, 1

    DMA_PHV2MEM_SETUP2(DMA_CMD_BASE, c1, cqe, cqe, CQE_P)

eqcb_eval:

    bbne            d.arm, 1, skip_eqcb
    // set the color in cqe
    phvwrpair       p.cqe.type, 0, p.cqe.color, CQ_COLOR  // BD slot

eqcb_setup:
    AQ_RX_EQCB_ADDR_GET(r5, r2, d.eq_id, K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES)
    tblwr.f     d.arm, 0

    phvwrpair   p.eqwqe.code, EQE_CODE_CQ_NOTIFY, p.eqwqe.type, EQE_TYPE_CQ
    phvwr       p.eqwqe.qid, d.cq_id

    CAPRI_RESET_TABLE_1_ARG()
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_rx_eqcb_process, r5)

skip_eqcb:
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CQ)

    DMA_SET_END_OF_CMDS_E(struct capri_dma_cmd_phv2mem_t, DMA_CMD_BASE)
    nop // Exit Slot

report_cqfull_error:
 
    phvwrpair   p.async_eqwqe.code, EQE_CODE_CQ_ERR_FULL, p.async_eqwqe.type, EQE_TYPE_CQ
    phvwr       p.async_eqwqe.qid, d.cq_id

    tblwr.f     d.cq_full, 1

report_async:
    CAPRI_SET_TABLE_2_VALID(0)
    //set aqcb.error = 1
    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_AQ_ERR_DIS)
    phvwr       p.error, 1
    sll         r2, K_AQCB_ADDR, AQCB_ADDR_SHIFT
    add         r2, r2, FIELD_OFFSET(aqcb0_t, error)
    DMA_HBM_PHV2MEM_SETUP(r6, error, error, r2)

    //PHV->eq_info is filled with appropriate error type and code by this time

    CAPRI_RESET_TABLE_0_ARG()
    phvwr          CAPRI_PHV_FIELD(CQ_ASYNC_EQ_INFO_P, async_eq), 1

    AQ_RX_EQCB_ADDR_GET(r5, r2, RDMA_EQ_ID_ASYNC, K_CQCB_BASE_ADDR_HI, K_LOG_NUM_CQ_ENTRIES)
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_rx_eqcb_process, r5) //Exit Slot

error_disable_aq:

    //fill the eqwqe
    phvwrpair   p.async_eqwqe.code, EQE_CODE_QP_ERR, p.async_eqwqe.type, EQE_TYPE_QP    //BD Slot

    //post ASYCN EQ error on QP
    b           report_async
    phvwr       p.async_eqwqe.qid, K_GLOBAL_QID //BD Slot

exit:
    nop.e
    nop
