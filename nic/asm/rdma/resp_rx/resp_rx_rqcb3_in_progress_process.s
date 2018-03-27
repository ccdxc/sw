#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct rqcb3_t d;
struct resp_rx_s1_t0_k k;

#define WQE_OFFSET      r1
#define NUM_VALID_SGES  r2
#define ADDR_TO_LOAD    r3

#define RQCB_TO_WQE_P   t0_s2s_rqcb_to_wqe_info
#define IN_P            t0_s2s_rqcb_to_rqcb1_info


%%
    .param    resp_rx_rqwqe_process

.align
resp_rx_rqcb1_in_progress_process:
    
    //  wqe_offset = RX_SGE_OFFSET +
    //  rqcb1_p->current_sge_id * sizeof(sge_t);

    add     WQE_OFFSET, RQWQE_SGE_OFFSET, CAPRI_KEY_FIELD(IN_P, current_sge_id), LOG_SIZEOF_SGE_T 
    add     ADDR_TO_LOAD, CAPRI_KEY_RANGE(IN_P, curr_wqe_ptr_sbit0_ebit7, curr_wqe_ptr_sbit56_ebit63), WQE_OFFSET
    sub     NUM_VALID_SGES, CAPRI_KEY_FIELD(IN_P, num_sges), CAPRI_KEY_FIELD(IN_P, current_sge_id)

    // we come here only in case of SEND MID/LAST packets. sometimes for MID packets also completion may be 
    // required (in case of lkey access permission failures). Hence copying wrid field always into phv's cqwqe
    // structure. It may or may not be used depending on whether completion is happening or not.
    phvwr   p.cqwqe.id.wrid, d.wrid
    
    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD_RANGE2(RQCB_TO_WQE_P, in_progress, current_sge_id, CAPRI_KEY_RANGE(IN_P, in_progress, current_sge_id))
    phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_WQE_P, num_valid_sges), \
                NUM_VALID_SGES, \
                CAPRI_PHV_FIELD(RQCB_TO_WQE_P, dma_cmd_index), \
                RESP_RX_DMA_CMD_PYLD_BASE

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_process, ADDR_TO_LOAD)

    nop.e
    nop
