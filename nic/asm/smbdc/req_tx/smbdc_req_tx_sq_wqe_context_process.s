#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sq_wqe_context_aligned_t d;
struct smbdc_req_tx_s2_t0_k k;

#define IN_P t0_s2s_rdma_cqe_to_wqe_context_info
#define IN_TO_S2_P to_s2_to_stage

#define TO_CQCB_INFO_P to_s2s_rdma_cqcb_info

#define K_MSN CAPRI_KEY_RANGE(IN_P, msn_sbit0_ebit7, msn_sbit16_ebit23)
#define K_CQCB_ADDR CAPRI_KEY_RANGE(IN_TO_S2_P, cqcb_addr_sbit0_ebit31, cqcb_addr_sbit32_ebit33)


%%
    .param    smbdc_req_tx_cqcb_process

.align
smbdc_req_tx_sq_wqe_context_process:

    slt     c1, K_MSN, d.ctx.end_msn
    bcf     [c1], skip_cqe_posting

    seq     c1, d.ctx.type, SQ_WQE_CONTEXT_TYPE_MR //BD Slot
    bcf     [!c1], process_send
    nop //BD Slot
process_mr:
    phvwr      p.smbdc_cqe.wrid, d.ctx.wrid
    phvwr      p.{smbdc_cqe.mr_id1...smbdc_cqe.mr_id3}, d.{ctx.mr.mr_id1...ctx.mr.mr_id3}
    //color to be filled in later in cqcb_process

    CAPRI_RESET_TABLE_0_ARG()

    add        r2, r0, K_CQCB_ADDR

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_cqcb_process, r2)
        
    nop.e
    nop

process_send:
    seq     c1, d.ctx.type, SQ_WQE_CONTEXT_TYPE_SEND
    bcf     [!c1], process_others

    nop.e
    nop
    
process_others:


//partial smbdc completion
skip_cqe_posting:

    nop.e
    nop
