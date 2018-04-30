#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct cqwqe_t d;
struct smbdc_req_tx_s1_t0_k k;

#define CQE_TO_WQE_CTXT_P t0_s2s_rdma_cqe_to_wqe_context_info

#define IN_P t0_s2s_rdma_cqe_info

#define K_WQE_CONTEXT_ADDR CAPRI_KEY_RANGE(IN_P, wqe_context_addr_sbit0_ebit7, wqe_context_addr_sbit32_ebit33)

%%
    .param    smbdc_req_tx_sq_wqe_context_process

.align
smbdc_req_tx_rdma_cqe_process:

    seq     c1, d.status, CQ_STATUS_SUCCESS
    cmov    r2, c1, SMBDC_CQ_STATUS_SUCCESS, SMBDC_CQ_STATUS_FAILURE
    phvwr   p.smbdc_cqe.status, r2
    
    CAPRI_RESET_TABLE_0_ARG()
    
    add     r2, r0, K_WQE_CONTEXT_ADDR

    CAPRI_SET_FIELD2(CQE_TO_WQE_CTXT_P, msn, d.id.msn)

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_sq_wqe_context_process, r2)

    nop.e
