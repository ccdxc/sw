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
#define K_LOG_PMTU      CAPRI_KEY_FIELD(IN_P, log_pmtu)
#define K_REM_PYLD_BYTES CAPRI_KEY_FIELD(IN_P, remaining_payload_bytes)


%%
    .param    resp_rx_rqwqe_process

.align
resp_rx_rqcb3_in_progress_process:

    // increment num_pkts_in_curr_msg by 1 and store in r5
    // if last packet, make r6 zero, else make it same as r5
    // use r6 to update d.num_pkts_in_curr_msg
    // note that r5 will still have num_pkts_in_curr_msg + 1.

    add     r5, d.num_pkts_in_curr_msg, 1
    seq     c1, K_GLOBAL_FLAG(_last), 1
    cmov    r6, c1, r0, r5
    bcf     [!c1], skip_cqe_pyld_len
    tblwr.f d.num_pkts_in_curr_msg, r6   //BD Slot

    // only for last packet, compute the payload length to be populated in cqe.
    sll     r5, r5, K_LOG_PMTU
    add     r5, r5, K_REM_PYLD_BYTES
    
    // note that length and status fields are unionized in cqe.
    // if last packet encounters any further error down the line (for ex: lkey error),
    // status field will overwrite the length that is populated below. But cqe.error bit will be set
    // so that driver can interpret the field accordingly.
    phvwr   p.cqe.length, r5

skip_cqe_pyld_len:
    
    //  wqe_offset = RX_SGE_OFFSET +
    //  rqcb1_p->current_sge_id * sizeof(sge_t);

    add     WQE_OFFSET, RQWQE_SGE_OFFSET, CAPRI_KEY_FIELD(IN_P, current_sge_id), LOG_SIZEOF_SGE_T 
    add     ADDR_TO_LOAD, CAPRI_KEY_RANGE(IN_P, curr_wqe_ptr_sbit0_ebit7, curr_wqe_ptr_sbit56_ebit63), WQE_OFFSET
    sub     NUM_VALID_SGES, CAPRI_KEY_FIELD(IN_P, num_sges), CAPRI_KEY_FIELD(IN_P, current_sge_id)

    seq     c2, NUM_VALID_SGES, 1
    // move addr_to_load back by sizeof 2 SGE's
    sub.!c2 ADDR_TO_LOAD, ADDR_TO_LOAD, 2, LOG_SIZEOF_SGE_T
    // move addr_to_load back by sizeof 3 SGE's
    sub.c2  ADDR_TO_LOAD, ADDR_TO_LOAD, 3, LOG_SIZEOF_SGE_T

    // we come here only in case of SEND MID/LAST packets. sometimes for MID packets also completion may be 
    // required (in case of lkey access permission failures). Hence copying wrid field always into phv's cqwqe
    // structure. It may or may not be used depending on whether completion is happening or not.
    phvwr   p.cqe.recv.wrid, d.wrid
    
    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD_RANGE2(RQCB_TO_WQE_P, in_progress, current_sge_id, CAPRI_KEY_RANGE(IN_P, in_progress, current_sge_id))
    phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_WQE_P, num_valid_sges), \
                NUM_VALID_SGES, \
                CAPRI_PHV_FIELD(RQCB_TO_WQE_P, dma_cmd_index), \
                RESP_RX_DMA_CMD_PYLD_BASE

    // invoke rqwqe
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_process, ADDR_TO_LOAD)

