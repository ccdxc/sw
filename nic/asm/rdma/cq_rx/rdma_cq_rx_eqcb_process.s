#include "capri.h"
#include "cq_rx.h"
#include "eqcb.h"
#include "common_phv.h"
#include "capri-macros.h"

struct cq_rx_phv_t p;
struct cq_rx_s6_t2_k k;
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

#define K_CQ_ID CAPRI_KEY_FIELD(IN_P, cq_id)
#define K_EQCB_ADDR CAPRI_KEY_RANGE(IN_P, eqcb_addr_sbit0_ebit15, eqcb_addr_sbit24_ebit63)

%%

   .param RDMA_EQ_INTR_TABLE_BASE

.align
rdma_cq_rx_eqcb_process:

    // Pin eqcb process to stage 7
    mfspr         r1, spr_mpuid
    seq           c2, r1[4:2], STAGE_7
    bcf           [!c2], bubble_to_next_stage

    seq             c1, EQ_P_INDEX, 0 //BD Slot
    // flip the color if cq is wrap around
    tblmincri.c1    EQ_COLOR, 1, 1     

    phvwrpair       p.eqwqe.cq_id, K_CQ_ID, p.eqwqe.color, EQ_COLOR

    sllv            r1, EQ_P_INDEX, d.log_wqe_size
    add             EQWQE_P, d.eqe_base_addr, r1

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, CQ_RX_DMA_CMD_START_FLIT_ID, CQ_RX_DMA_CMD_EQ)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQWQE_START, PHV_EQWQE_END, EQWQE_P)
    #need to move this end-of-command one down when interrupt dma command is enabled
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    // RDMA_EQ_INTR_TABLE_BASE is 34 bytes, so load it into register using 2 instructions
    addui       EQ_INT_ADDR, r0, hiword(RDMA_EQ_INTR_TABLE_BASE)
    addi        EQ_INT_ADDR, EQ_INT_ADDR, loword(RDMA_EQ_INTR_TABLE_BASE)

    add             EQ_INT_ADDR, EQ_INT_ADDR, d.int_num, RDMA_EQ_INTR_TABLE_ENTRY_SIZE_SHFT

    add             DMA_CMD_BASE, DMA_CMD_BASE, DMA_CMD_SIZE_BITS
    //DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQ_INT_NUM_START, PHV_EQ_INT_NUM_END, EQ_INT_ADDR)
    
    CAPRI_SET_TABLE_2_VALID(0)

    // increment p_index
    tblmincri.e     EQ_P_INDEX, d.log_num_wqes, 1
    nop //Exit Slot

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], exit
    //invoke the same routine, with valid eqcb addr
    CAPRI_GET_TABLE_2_K(cq_rx_phv_t, r7) //BD Slot
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, K_EQCB_ADDR)

exit:
    nop.e
    nop

