#include "capri.h"
#include "resp_rx.h"
#include "eqcb.h"
#include "common_phv.h"
#include "capri-macros.h"

struct resp_rx_phv_t p;
struct resp_rx_s7_t1_k k;
struct eqcb_t d;

#define EQWQE_P r1
#define DMA_CMD_BASE r4
#define TMP r5
#define GLOBAL_FLAGS r7

#define PHV_EQWQE_START       s1.eqwqe.qid
#define PHV_EQWQE_END         s1.eqwqe.color
#define PHV_ASYNC_EQWQE_START s1.eqwqe.qid
#define PHV_ASYNC_EQWQE_END   s1.eqwqe.color

#define PHV_EQ_INT_ASSERT_DATA_BEGIN       s1.int_assert_data
#define PHV_EQ_INT_ASSERT_DATA_END         s1.int_assert_data
#define PHV_ASYNC_EQ_INT_ASSERT_DATA_BEGIN s1.int_assert_data
#define PHV_ASYNC_EQ_INT_ASSERT_DATA_END   s1.int_assert_data

#define IN_P t1_s2s_cqcb_to_eq_info

#define K_ASYNC_EQ CAPRI_KEY_FIELD(IN_P, async_eq)
#define K_CMD_EOP CAPRI_KEY_FIELD(IN_P, cmd_eop)

%%

   .param RDMA_EQ_INTR_TABLE_BASE

.align
resp_rx_eqcb_process:

    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    IS_ANY_FLAG_SET(c2, GLOBAL_FLAGS, RESP_RX_FLAG_READ_REQ|RESP_RX_FLAG_ATOMIC_FNA|RESP_RX_FLAG_ATOMIC_CSWAP)
    seq             c1, EQ_P_INDEX, 0
    // flip the color if cq is wrap around
    tblmincri.c1    EQ_COLOR, 1, 1     


    sllv            r1, EQ_P_INDEX, d.log_wqe_size
    add             EQWQE_P, d.eqe_base_addr, r1

    bbeq            K_ASYNC_EQ, 1, async_eq
    // increment p_index
    tblmincri      EQ_P_INDEX, d.log_num_wqes, 1


completion_eq:
    phvwr           p.s1.eqwqe.color, EQ_COLOR
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_EQ, !c2)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RD_ATOMIC_EQ, c2)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQWQE_START, PHV_EQWQE_END, EQWQE_P)

    //Writing Interrupt unconditionally... if needed, add a flag for this purpose
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_EQ_INT, !c2)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RD_ATOMIC_EQ_INT, c2)
    phvwri          p.s1.int_assert_data, CAPRI_INT_ASSERT_DATA
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQ_INT_ASSERT_DATA_BEGIN, PHV_EQ_INT_ASSERT_DATA_END, d.int_assert_addr)

    CAPRI_SET_TABLE_1_VALID(0)
    seq.e          c1, K_CMD_EOP, 1
    DMA_SET_END_OF_CMDS_C(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE, c1)

async_eq:
    phvwr           p.s1.eqwqe.color, EQ_COLOR
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ASYNC_EQ, !c2)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RD_ATOMIC_ASYNC_EQ, c2)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_ASYNC_EQWQE_START, PHV_ASYNC_EQWQE_END, EQWQE_P)

    //Writing Interrupt unconditionally... if needed, add a flag for this purpose
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ASYNC_EQ_INT, !c2)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RD_ATOMIC_ASYNC_EQ_INT, c2)
    phvwri          p.s1.int_assert_data, CAPRI_INT_ASSERT_DATA
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_ASYNC_EQ_INT_ASSERT_DATA_BEGIN, PHV_ASYNC_EQ_INT_ASSERT_DATA_END, d.int_assert_addr)

    DMA_SET_END_OF_CMDS_E(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
    CAPRI_SET_TABLE_0_VALID(0)
