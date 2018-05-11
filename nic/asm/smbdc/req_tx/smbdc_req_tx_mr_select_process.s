#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb1_t d;
struct smbdc_req_tx_s2_t0_k k;

#define IN_P t0_s2s_wqe_to_mr_select_info

#define SELECT_MR_TO_RDMA_P t0_s2s_mr_select_to_rdma_info

#define K_NUM_MRS CAPRI_KEY_FIELD(IN_P, num_mrs)

%%
    .param    smbdc_req_tx_post_rdma_req_process

.align
smbdc_req_tx_mr_select_process:

    add     r1, K_NUM_MRS, d.total_use_count
    sle     c1, r1, MAX_MRS_PER_SMBDC_CONNECTION
    bcf     [!c1], resource_exhausted
    tblwr.c1 d.total_use_count, r1 //BD Slot

    add     r1, r0, r0

    add     r3, r0, r0

    add     r6, r0, offsetof(struct req_tx_phv_t, rdma_wqe3)
    add     r5, r0, offsetof(struct req_tx_phv_t, smbdc_wqe_context)
    phvwrp  r5, offsetof(struct sq_wqe_context_t, type), sizeof(p.smbdc_wqe_context.type), SQ_WQE_CONTEXT_TYPE_MR

allocate_next_mr:

mr_try_next_row:
    #seq     c1, r1, 512
    #bcf     [c1], resource_exhausted
    tblrdp  r4, r1, 0, 64 //row6
    ffcv    r2, r4, r0 //BD Slot
    seq     c1, r2, -1
    bcf     [c1], mr_try_next_row
    add.c1  r1, r1, MRS_PER_ROW //BD Slot
    fsetv   r4, r4, r2, r2
    tblwrp  r1, 0, 64, r4

    add     r2, d.mr_base, r2
    phvwrp  r6, offsetof(struct sqwqe_t, frmr.mr_id), sizeof(p.rdma_wqe0.frmr.mr_id), r2
    phvwrp  r5, offsetof(struct sq_wqe_context_t, mr.mr_id1), sizeof(p.smbdc_wqe_context.mr.mr_id1), r2
    sub     r5, r5, sizeof(p.smbdc_wqe_context.mr.mr_id1)

    add     r3, r3, 1
    seq     c1, r3, K_NUM_MRS
    bcf     [!c1], allocate_next_mr
    add.!c1 r6, r6, sizeof(p.rdma_wqe0) //BD slot

    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(SELECT_MR_TO_RDMA_P, num_wqes), K_NUM_MRS, \
              CAPRI_PHV_FIELD(SELECT_MR_TO_RDMA_P, dma_cmd_start_index), REQ_TX_DMA_CMD_RDMA_REQ_BASE
    SQCB2_ADDR_GET(r6)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_post_rdma_req_process, r6)

    nop.e
    nop
    
resource_exhausted:
//ignoring handling of error for now

exit:
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop
