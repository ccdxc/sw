#include "req_rx.h"
#include "eqcb.h"

struct req_rx_phv_t p;
struct req_rx_eqcb_process_k_t k;
struct eqcb_t d;

#define PHV_EQWQE_START eqwqe.cq_id
#define PHV_EQWQE_END   eqwqe.rsvd

#define PHV_EQ_INT_NUM_START eq_int_num
#define PHV_EQ_INT_NUM_END eq_int_num

%%

.align
req_rx_eqcb_process:

    add             r1, r0, k.args.dma_cmd_index
    add             r3, 0, k.args.tbl_id

    seq             c1, EQ_P_INDEX, 0
    // flip the color if cq is wrap around
    tblmincri.c1    EQ_COLOR, 1, 1     

    phvwr           p.eqwqe.cq_id, k.args.cq_id
    phvwr           p.eqwqe.color, EQ_COLOR 

    add             r1, r0, d.log_wqe_size
    sllv            r1, EQ_P_INDEX, r1
    add             r1, d.base_addr, r1

    DMA_CMD_I_BASE_GET(r4, r5, REQ_RX_DMA_CMD_START_FLIT_ID, r1)
    DMA_PHV2MEM_SETUP(r4, c1, PHV_EQWQE_START, PHV_EQWQE_END, r1)
    add             r1, r1, 1
    add             r4, r4, DMA_CMD_SIZE_BITS

    //TODO: use proper interrupt base
    add             r6, CAPRI_INTR_BASE, d.int_num
    DMA_PHV2MEM_SETUP(r4, c1, PHV_EQ_INT_NUM_START, PHV_EQ_INT_NUM_END, r6)
    
    CAPRI_SET_TABLE_I_VALID(r3, 0)

    // increment p_index
    tblmincri       EQ_P_INDEX, d.log_num_wqes, 1
    nop.e
    nop

