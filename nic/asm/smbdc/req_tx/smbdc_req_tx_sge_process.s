#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct smbdc_sqsge_t d;
struct smbdc_req_tx_s2_t0_k k;

#define TO_RDMA_REQ_INFO_P t0_s2s_mr_select_to_rdma_info
#define TO_S4_P       to_s4_to_stage_sq

#define IN_P t0_s2s_wqe_to_sge_info
#define IN_TO_S2_P to_s2_to_stage

#define K_NUM_VALID_SGES CAPRI_KEY_FIELD(IN_P, num_valid_sges)
#define K_CURRENT_SGE_ID CAPRI_KEY_FIELD(IN_P, current_sge_id)
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_FIELD(IN_P, current_sge_offset)
#define K_WRID CAPRI_KEY_RANGE(IN_P, wrid_sbit0_ebit7, wrid_sbit24_ebit31)
#define K_MAX_SEND_SIZE CAPRI_KEY_RANGE(IN_P, max_send_size_sbit0_ebit5, max_send_size_sbit30_ebit31)
#define K_CURRENT_TOTAL_OFFSET CAPRI_KEY_RANGE(IN_P, current_total_offset_sbit0_ebit7, current_total_offset_sbit24_ebit31)
#define K_TOTAL_DATA_LENGTH CAPRI_KEY_FIELD(IN_TO_S2_P, total_data_length)

//we have only a total of 8 DMA command space.
//Each send requires 2 DMA commands (one for WQE and other for Context) + 2 more (Doorbell and SMBD context)
//Hence limitting to 3
#define MAX_RDMA_REQ_PER_PHV 3

#define RDMA_WQE_PTR r5
#define CUR_RDMA_WQE_ID r2
#define CUR_SGE_OFFSET r4
#define CUR_SGE_ID r1
#define SMBD_SGE_PTR r6
#define CUR_TOTAL_OFFSET r7

%%
    .param    smbdc_req_tx_post_rdma_req_process

.align
smbdc_req_tx_sge_process:


    add     CUR_SGE_ID, r0, r0 #count upto K_NUM_VALID_SGES
    add     CUR_RDMA_WQE_ID, r0, r0 #count upto MAX_RDMA_REQ_PER_PHV
    add     CUR_SGE_OFFSET, r0, K_CURRENT_SGE_OFFSET
    add     RDMA_WQE_PTR, r0, offsetof(struct req_tx_phv_t, rdma_wqe0)
    add     SMBD_SGE_PTR, r0, offsetof(struct smbdc_sqsge_t, sge0)
    add     CUR_TOTAL_OFFSET, r0, K_CURRENT_TOTAL_OFFSET

for_each_rdma_wqe:

    //set rdmawqe->wrid and op_type
    phvwrp  RDMA_WQE_PTR, offsetof(struct sqwqe_t, base.wrid), sizeof(p.rdma_wqe0.base.wrid), K_WRID
    phvwrp  RDMA_WQE_PTR, offsetof(struct sqwqe_t, base.op_type), sizeof(p.rdma_wqe0.base.op_type), OP_TYPE_SEND

    //read smbdc_sge->va
    tblrdp  r3, SMBD_SGE_PTR, 0, sizeof(d.sge0.va)
    //add current_sge_offset
    add     r3, r3, CUR_SGE_OFFSET //add current_sge_offset
    //set rdmawqe->sge1->va with this value
    phvwrp  RDMA_WQE_PTR, offsetof(struct sqwqe_t, sge1.va), sizeof(p.rdma_wqe0.sge1.va), r3

    //read smbdc_sge->len
    tblrdp  r3, SMBD_SGE_PTR, 0, sizeof(d.sge0.len)
    //check if it is less than RDMA_PMTU
    slt     c1, r3, K_MAX_SEND_SIZE
    //pick rdma_wqe->sge1->len as max(MAX_SEND_SIZE, smbdc_sge->len)
    cmov    r3, c1, r3, K_MAX_SEND_SIZE
    phvwrp  RDMA_WQE_PTR, offsetof(struct sqwqe_t, sge1.len), sizeof(p.rdma_wqe0.sge1.len), r3

    #add to current_total_offset
    add     CUR_TOTAL_OFFSET, CUR_TOTAL_OFFSET, r3

    //get rdma_wqe_context ptr associated with the current wqe
    //no-available registers - so, do the math every time
    add     r3, r0, offsetof(struct req_tx_phv_t, rdma_wqe_ctx0.smbdc_hdr)
    add     r3, r3, (1 << RDMA_SQ_WQE_CONTEXT_LOG_SIZE)
    phvwrp  r3, offsetof(struct rdma_smbdc_header_t, data_length), sizeof(p.rdma_wqe_ctx0.smbdc_hdr.data_length), r3 
    sub     r3, K_TOTAL_DATA_LENGTH, CUR_TOTAL_OFFSET
    phvwrp  r3, offsetof(struct rdma_smbdc_header_t, remaining_data_length), sizeof(p.rdma_wqe_ctx0.smbdc_hdr.remaining_data_length), r3 
    //add more fields here..

    //adjust the current_sge_offset
    add     CUR_SGE_OFFSET, CUR_SGE_OFFSET, r3
    //if more data in smbdc_sge, shift to the next beginning. Otherwise, set 0
    cmov    CUR_SGE_OFFSET, c1, r0, CUR_SGE_OFFSET
    //move to next smbd_wqe
    add.c1  SMBD_SGE_PTR, SMBD_SGE_PTR, sizeof(d.sge0)

    //if this smbdc_wqe has no more data, increment the cur_sge_id
    add.c1  CUR_SGE_ID, CUR_SGE_ID, 1
    slt     c3, CUR_SGE_ID, K_NUM_VALID_SGES
    
    //wind up this rdma_wqe, and move to next one
    phvwrp  RDMA_WQE_PTR, offsetof(struct sqwqe_t, base.num_sges), sizeof(p.rdma_wqe0.base.num_sges), 2

    sub     RDMA_WQE_PTR, RDMA_WQE_PTR, sizeof(p.rdma_wqe0)

    slt     c2, CUR_RDMA_WQE_ID, MAX_RDMA_REQ_PER_PHV
    bcf     [c2 & !c3], for_each_rdma_wqe
    add.c2  CUR_RDMA_WQE_ID, CUR_RDMA_WQE_ID, 1 //BD Slot

    CAPRI_RESET_TABLE_0_ARG()

    phvwr.c3  CAPRI_PHV_FIELD(TO_S4_P, in_progress), 1
    phvwrpair CAPRI_PHV_FIELD(TO_S4_P, current_sge_id), CUR_SGE_ID, \
              CAPRI_PHV_FIELD(TO_S4_P, current_sge_offset), CUR_SGE_OFFSET
    phvwr     CAPRI_PHV_FIELD(TO_S4_P, current_total_offset), CUR_TOTAL_OFFSET

    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(TO_RDMA_REQ_INFO_P, num_wqes), CUR_RDMA_WQE_ID, \
              CAPRI_PHV_FIELD(TO_RDMA_REQ_INFO_P, dma_cmd_start_index), REQ_TX_DMA_CMD_RDMA_REQ_BASE

    SQCB2_ADDR_GET(r2)    
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_post_rdma_req_process, r2)

    nop.e
    nop
