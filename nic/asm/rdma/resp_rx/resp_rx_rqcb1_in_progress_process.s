#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_rx_phv_t p;
struct rqcb1_t d;
struct resp_rx_rqcb1_in_progress_process_k_t k;

#define WQE_OFFSET      r1
#define NUM_VALID_SGES  r2
#define ADDR_TO_LOAD    r3

#define RAW_TABLE_PC    r7

#define RQCB_TO_WQE_T   struct resp_rx_rqcb_to_wqe_info_t


%%
    .param    resp_rx_rqwqe_process

.align
resp_rx_rqcb1_in_progress_process:
    
    //  wqe_offset = RX_SGE_OFFSET +
    //  rqcb1_p->current_sge_id * sizeof(sge_t);

    add     WQE_OFFSET, RQWQE_SGE_OFFSET, d.current_sge_id, LOG_SIZEOF_SGE_T 
    add     ADDR_TO_LOAD, d.curr_wqe_ptr, WQE_OFFSET
    sub     NUM_VALID_SGES, d.num_sges, d.current_sge_id
    
    // resp_rx_rqcb_to_wqe_info_t *rqcb_to_wqe_info_p =
    //  (resp_rx_rqcb_to_wqe_info_t *) PHV_TABLE_I_ARG_PTR_GET(phv_p, tbl_id);
    // rqcb_to_wqe_info_p->in_progress = info_p->in_progress;
    // rqcb_to_wqe_info_p->remaining_payload_bytes = 
    //         info_p->remaining_payload_bytes;
    // rqcb_to_wqe_info_p->current_sge_id = rqcb1_p->current_sge_id;
    // rqcb_to_wqe_info_p->current_sge_offset = rqcb1_p->current_sge_offset;
    // rqcb_to_wqe_info_p->num_valid_sges =
    //         rqcb1_p->num_sges - rqcb1_p->current_sge_id;
    // rqcb_to_wqe_info_p->curr_wqe_ptr = rqcb1_p->curr_wqe_ptr;
    
    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, current_sge_id, d.current_sge_id)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, current_sge_offset, d.current_sge_offset)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, num_valid_sges, NUM_VALID_SGES)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, curr_wqe_ptr, d.curr_wqe_ptr)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, dma_cmd_index, RESP_RX_DMA_CMD_PYLD_BASE)
    //CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, tbl_id, 0)

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqwqe_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, ADDR_TO_LOAD)

    nop.e
    nop
