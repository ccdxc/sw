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
#define WQE_PTR         r5

#define RQCB_TO_WQE_T   struct resp_rx_rqcb_to_wqe_info_t


%%
    .param    resp_rx_rqwqe_process

.align
resp_rx_rqcb1_recirc_sge_process:
    
    //  wqe_offset = RX_SGE_OFFSET +
    //  rqcb1_p->current_sge_id * sizeof(sge_t);

    add         r7, r0, k.global.flags

    add     WQE_OFFSET, RQWQE_SGE_OFFSET, k.to_stage.s1.recirc.current_sge_id, LOG_SIZEOF_SGE_T 

    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_FIRST|RESP_RX_FLAG_ONLY)

    // if it is the first or only packet getting recirc'd,
    // rqcb1 is not yet populated with wqe_ptr and num_sges field.
    // In these cases, take the wqe_ptr and num_sges from recirc info

    cmov    WQE_PTR, c1, k.to_stage.s1.recirc.curr_wqe_ptr, d.curr_wqe_ptr
    add     ADDR_TO_LOAD, WQE_PTR, WQE_OFFSET

    cmov    NUM_VALID_SGES, c1, k.to_stage.s1.recirc.num_sges, d.num_sges

    sub     NUM_VALID_SGES, NUM_VALID_SGES, k.to_stage.s1.recirc.current_sge_id
    
    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, in_progress, 1)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, recirc_path, 1)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, remaining_payload_bytes, k.to_stage.s1.recirc.remaining_payload_bytes)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, current_sge_id, k.to_stage.s1.recirc.current_sge_id)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, current_sge_offset, k.to_stage.s1.recirc.current_sge_offset)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, num_valid_sges, NUM_VALID_SGES)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, curr_wqe_ptr, WQE_PTR)
    CAPRI_SET_FIELD(r4, RQCB_TO_WQE_T, dma_cmd_index, k.to_stage.s1.recirc.dma_cmd_index)

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
    CAPRI_NEXT_TABLE_I_READ_PC(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_process, ADDR_TO_LOAD)

    nop.e
    nop
