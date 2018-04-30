#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct smbdc_sqwqe_t d;
struct smbdc_req_tx_s1_t0_k k;

#define WQE_TO_SELECT_MR_P t0_s2s_wqe_to_mr_select_info


%%
    .param    smbdc_req_tx_mr_select_process

.align
smbdc_req_tx_wqe_process:

    seq     c1, d.type, SMBDC_SQWQE_OP_TYPE_SEND
    bcf     [c1], process_send

process_mr:
    CAPRI_RESET_TABLE_0_ARG() //BD Slot

    add     r1, d.mr.num_sg_lists, r0
    
    phvwr CAPRI_PHV_FIELD(WQE_TO_SELECT_MR_P, num_mrs), d.mr.num_sg_lists

    //sqcb1 has the MR bitmap associated with this connection
    SQCB1_ADDR_GET(r2)    

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_mr_select_process, r2)
    
    add     r2, r0, r0
    setcf   c1, [c0] //first sg_list
    add     r1, r0, offsetof(struct req_tx_phv_t, rdma_wqe3)
    add     r3, r0, offsetof(struct smbdc_sqwqe_mr_t, sg3.num_pages)

#go thru each of the SG list and prepare one RDMA FRMR wqe on PHV
# - this stage will setup wrid, frmr_type, dma_src_address, num_pages
# - next stage(mr_select) will setup mr_id associated with each wqe
# - one after that(post_rdma) will setup DMA instructions to post WQEs to RDMA queue
for_each_sg_list:

    phvwrp  r1, offsetof(struct sqwqe_t, base.wrid), sizeof(p.rdma_wqe0.base.wrid), d.mr.wrid
    phvwrp  r1, offsetof(struct sqwqe_t, base.op_type), sizeof(p.rdma_wqe0.base.op_type), OP_TYPE_FRMR
    phvwrp  r1, offsetof(struct sqwqe_t, frmr.type), sizeof(p.rdma_wqe0.frmr.type), FRMR_TYPE_REGULAR
    tblrdp  r4, r3, 0, sizeof(d.mr.sg0.num_pages)
    phvwrp  r1, offsetof(struct sqwqe_t, frmr.num_pages), sizeof(p.rdma_wqe0.frmr.num_pages), r4
    tblrdp  r4, r3, sizeof(d.mr.sg0.num_pages), sizeof(d.mr.sg0.base_addr)
    phvwrp  r1, offsetof(struct sqwqe_t, frmr.dma_src_address), sizeof(p.rdma_wqe0.frmr.dma_src_address), r4
    
    add     r1, r1, sizeof(p.rdma_wqe0)
    add     r2, r2, 1
    seq     c1, r2, d.mr.num_sg_lists
    bcf     [!c1], for_each_sg_list
    add     r3, r3, sizeof(d.mr.sg0) //jump to next sg element - BD Slot
    
    //fill context
    phvwr   p.smbdc_wqe_context.wrid, d.mr.wrid
    
    nop.e
    nop

process_send:
    nop.e
    nop

exit:
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop
