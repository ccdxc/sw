#include "capri.h"
#include "resp_rx.h"
#include "eqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_eqcb_process_k_t k;
struct eqcb_t d;

#define EQWQE_P r1
#define DMA_CMD_INDEX r2
#define TBL_ID r3
#define DMA_CMD_BASE r4
#define TMP r5
#define EQ_INT_ADDR r6

#define PHV_EQWQE_START eqwqe.cq_id
#define PHV_EQWQE_END   eqwqe.rsvd

#define PHV_EQ_INT_NUM_START eq_int_num
#define PHV_EQ_INT_NUM_END eq_int_num

%%

.align
resp_rx_eqcb_process:

    add             DMA_CMD_INDEX, r0, k.args.dma_cmd_index
    add             TBL_ID, 0, k.args.tbl_id

    seq             c1, EQ_P_INDEX, 0
    // flip the color if cq is wrap around
    tblmincri.c1    EQ_COLOR, 1, 1     

    phvwr           p.eqwqe.cq_id, k.args.cq_id
    phvwr           p.eqwqe.color, EQ_COLOR 

    add             r1, r0, d.log_wqe_size
    sllv            r1, EQ_P_INDEX, r1
    add             EQWQE_P, d.base_addr, r1

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, TMP, RESP_RX_DMA_CMD_START_FLIT_ID, DMA_CMD_INDEX)
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQWQE_START, PHV_EQWQE_END, EQWQE_P)
    add             DMA_CMD_INDEX, DMA_CMD_INDEX, 1
    add             DMA_CMD_BASE, DMA_CMD_BASE, DMA_CMD_SIZE_BITS

    //TODO: use proper interrupt base
    add             EQ_INT_ADDR, CAPRI_INTR_BASE, d.int_num
    DMA_PHV2MEM_SETUP(DMA_CMD_BASE, c1, PHV_EQ_INT_NUM_START, PHV_EQ_INT_NUM_END, EQ_INT_ADDR)
    
    CAPRI_SET_TABLE_I_VALID(TBL_ID, 0)

    // increment p_index
    tblmincri       EQ_P_INDEX, d.log_num_wqes, 1
    nop.e
    nop

