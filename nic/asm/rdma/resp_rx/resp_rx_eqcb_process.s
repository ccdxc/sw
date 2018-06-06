#include "capri.h"
#include "resp_rx.h"
#include "eqcb.h"
#include "common_phv.h"
#include "capri-macros.h"

struct resp_rx_phv_t p;
struct resp_rx_s6_t2_k k;
struct eqcb_t d;

#define EQWQE_P r1
#define DMA_CMD_BASE r4
#define TMP r5
#define EQ_INT_ADDR r6

#define PHV_EQWQE_START eqwqe.cq_id
#define PHV_EQWQE_END   eqwqe.rsvd

#define PHV_EQ_INT_NUM_START eq_int_num
#define PHV_EQ_INT_NUM_END eq_int_num

#define IN_P t2_s2s_cqcb_to_eq_info

%%

   .param RDMA_EQ_INTR_TABLE_BASE

.align
resp_rx_eqcb_process:

    seq             c1, EQ_P_INDEX, 0
    // flip the color if cq is wrap around
    tblmincri.c1    EQ_COLOR, 1, 1     

    phvwrpair       p.eqwqe.cq_id, CAPRI_KEY_RANGE(IN_P, cq_id_sbit0_ebit7, cq_id_sbit16_ebit23), p.eqwqe.color, EQ_COLOR

    sllv            r1, EQ_P_INDEX, d.log_wqe_size
    add             EQWQE_P, d.eqe_base_addr, r1

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_EQ)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQWQE_START, PHV_EQWQE_END, EQWQE_P)
    #need to move this end-of-command one down when interrupt dma command is enabled
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    // RDMA_EQ_INTR_TABLE_BASE is 34 bytes, so load it into register using 2 instructions
    addui       EQ_INT_ADDR, r0, hiword(RDMA_EQ_INTR_TABLE_BASE)
    addi        EQ_INT_ADDR, EQ_INT_ADDR, loword(RDMA_EQ_INTR_TABLE_BASE)

    add             EQ_INT_ADDR, EQ_INT_ADDR, d.int_num, RDMA_EQ_INTR_TABLE_ENTRY_SIZE_SHFT

    add             DMA_CMD_BASE, DMA_CMD_BASE, DMA_CMD_SIZE_BITS
    //DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQ_INT_NUM_START, PHV_EQ_INT_NUM_END, EQ_INT_ADDR)
    

    // increment p_index
    tblmincri.e     EQ_P_INDEX, d.log_num_wqes, 1
    CAPRI_SET_TABLE_2_VALID(0)

