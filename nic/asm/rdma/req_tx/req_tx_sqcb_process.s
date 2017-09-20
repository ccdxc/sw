#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct rdma_stage0_table_k k;

#define INFO_OUT1_T struct req_tx_sqcb_to_pt_info_t
#define INFO_OUT2_T struct req_tx_sqcb_to_wqe_info_t
#define INFO_OUT3_T struct req_tx_wqe_to_sge_info_t

%%
    .param    req_tx_sqpt_process
    .param    req_tx_sqwqe_process
    .param    req_tx_sqsge_process

.align
req_tx_sqcb_process:
    // copy intrinsic to global
    add            r1, r0, offsetof(struct phv_, common_global_global_data)

    // enable below code after spr_tbladdr special purpose register is available in capsim
    #mfspr         r1, spr_tbladdr
    #srl           r1, r1, SQCB_ADDR_SHIFT
    #CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, cb_addr, r1)
    add            r2, r0, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    srl            r2, r2, SQCB_ADDR_SHIFT

    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, r2)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)

    //set dma_cmd_ptr in phv
    TXDMA_DMA_CMD_PTR_SET(REQ_TX_DMA_CMD_START_FLIT_ID) // Branch Delay Slot

    // if (sqcb0_p->fence) goto fence
    seq            c1, d.fence, 1
    bcf            [c1], fence
    nop

    // if (sqcb0_p->in_progress) goto in_progress
    seq            c1, 1, d.in_progress
    bcf            [c1], in_progress

    // log_num_wqe_per_page = (log_sq_page_size - log_wqe_size)
    // page_index = sq_c_index >> (log_num_wqe_per_page)
    add            r1, r0, SQ_C_INDEX
    sub            r2, d.log_sq_page_size, d.log_wqe_size
    srlv           r3, r1, r2

    // page_offset = ((sq_c_index & ((1 << log_num_wqe_per_page) -1)) << log_wqe_size)
    mincr          r1, r2, r0
    add            r2, r0, d.log_wqe_size
    sllv           r1, r1, r2

    // page_seg_offset = page_index & 0x7
    and            r2, r3, CAPRI_SEG_PAGE_MASK

    // page_index = page_index & ~0x7
    // page_p = sqcb_p->pt_base_addr + (page_index * sizoef(u64))
    sub            r3, r3, r2
    add            r3, d.pt_base_addr, r3, CAPRI_LOG_SIZEOF_U64

    // remaining_payload_bytes = (1 << sqcb0_p->log_pmtu), to start with
    add            r4, r0, d.log_pmtu
    b              in_progress_end
    sllv           r4, 1, r4 // Branch Delay Slot

in_progress:
    // load wqe using sqcb_p->wqe_addr

    // sge_offset = TXWQE_SGE_OFFSET + sqcb0_p->current_sge_id * sizeof(sge_t);
    add            r1, TXWQE_SGE_OFFSET, d.current_sge_id, LOG_SIZEOF_SGE_T
    // sge_p = sqcb0_p->curr_wqe_ptr + sge_offset
    add            r1, r1, d.curr_wqe_ptr

    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, in_progress, d.in_progress)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, current_sge_id, d.current_sge_id)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, current_sge_offset, d.current_sge_offset)
    // num_valid_sges = sqcb0_p->num_sges = sqcb0_p->current_sge_id
    sub            r2, d.num_sges, d.current_sge_id 
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, num_valid_sges, r2)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, remaining_payload_bytes, r4)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, dma_cmd_start_index, REQ_TX_DMA_CMD_START_FLIT_ID)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, wqe_addr, d.curr_wqe_ptr)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, first, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, op_type, d.curr_op_type)

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqsge_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)
    
    nop.e
    nop

in_progress_end:
    // populate t0 stage to stage data req_tx_sqcb_to_wqe_info_t for next stage
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, page_offset, r1)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, remaining_payload_bytes, r4)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, rrq_p_index, RRQ_P_INDEX)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, log_pmtu, d.log_pmtu)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, page_seg_offset, r2)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, pd, d.pd)

    // populate t0 PC and table address
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqpt_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r3)

    nop.e
    nop

fence:
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, in_progress, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, current_sge_id, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, current_sge_offset, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, num_valid_sges, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, remaining_payload_bytes, 0)
    //CAPRI_SET_FIELD(r7, INFO_OUT2_T, wqe_ptr, d.curr_wqe_ptr)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, rrq_p_index, RRQ_P_INDEX)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, log_pmtu, d.log_pmtu)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, li_fence_cleared, 0)

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqwqe_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, d.curr_wqe_ptr)

    nop.e
    nop
