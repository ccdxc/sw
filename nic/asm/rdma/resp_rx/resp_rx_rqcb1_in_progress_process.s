#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_rx_phv_t p;
// this is an mpu only program, hence there is no d-vector
struct resp_rx_rqcb1_in_progress_process_k_t k;

#define WQE_OFFSET      r1
#define NUM_VALID_SGES  r2
#define ADDR_TO_LOAD    r3

#define RQCB_TO_WQE_T   struct resp_rx_rqcb_to_wqe_info_t


%%
    .param    resp_rx_rqwqe_process

.align
resp_rx_rqcb1_in_progress_process:
    
    //  wqe_offset = RX_SGE_OFFSET +
    //  rqcb1_p->current_sge_id * sizeof(sge_t);

    add     WQE_OFFSET, RQWQE_SGE_OFFSET, k.args.current_sge_id, LOG_SIZEOF_SGE_T 
    add     ADDR_TO_LOAD, k.args.curr_wqe_ptr, WQE_OFFSET
    sub     NUM_VALID_SGES, k.args.num_sges, k.args.current_sge_id
    
    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD_RANGE(r4, RQCB_TO_WQE_T, in_progress, current_sge_id, k.{args.in_progress...args.current_sge_id})
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, num_valid_sges, NUM_VALID_SGES)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, dma_cmd_index, RESP_RX_DMA_CMD_PYLD_BASE)

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_process, ADDR_TO_LOAD)

    nop.e
    nop
