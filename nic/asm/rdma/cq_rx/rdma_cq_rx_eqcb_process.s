#include "capri.h"
#include "cq_rx.h"
#include "eqcb.h"
#include "common_phv.h"
#include "capri-macros.h"

struct cq_rx_phv_t p;
struct cq_rx_s7_t1_k k;
struct eqcb_t d;

#define EQWQE_P r1
#define DMA_CMD_BASE r4

#define PHV_EQWQE_START eqwqe.qid
#define PHV_EQWQE_END   eqwqe.color

#define PHV_EQ_INT_ASSERT_DATA_BEGIN int_assert_data
#define PHV_EQ_INT_ASSERT_DATA_END int_assert_data

#define IN_P t1_s2s_cqcb_to_eq_info

#define K_CQ_ID CAPRI_KEY_FIELD(IN_P, cq_id)

%%

   .param RDMA_EQ_INTR_TABLE_BASE

.align
rdma_cq_rx_eqcb_process:

    seq             c1, EQ_P_INDEX, 0 //BD Slot
    // flip the color if cq is wrap around
    tblmincri.c1    EQ_COLOR, 1, 1

    phvwrpair       p.eqwqe.qid, K_CQ_ID, p.eqwqe.color, EQ_COLOR
    phvwrpair       p.eqwqe.code, EQE_CODE_CQ_NOTIFY, p.eqwqe.type, EQE_TYPE_CQ

    sllv            r1, EQ_P_INDEX, d.log_wqe_size
    add             EQWQE_P, d.eqe_base_addr, r1

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, CQ_RX_DMA_CMD_START_FLIT_ID, CQ_RX_DMA_CMD_EQ)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQWQE_START, PHV_EQWQE_END, EQWQE_P)

    //Writing Interrupt unconditionally... if needed, add a flag for this purpose
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, CQ_RX_DMA_CMD_START_FLIT_ID, CQ_RX_DMA_CMD_EQ_INT)
    phvwri          p.int_assert_data, CAPRI_INT_ASSERT_DATA
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQ_INT_ASSERT_DATA_BEGIN, PHV_EQ_INT_ASSERT_DATA_END, d.int_assert_addr)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    // increment p_index
    tblmincri.e     EQ_P_INDEX, d.log_num_wqes, 1
    CAPRI_SET_TABLE_1_VALID(0) //Exit Slot

