#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct rdma_stage0_table_k k;

#define SQCB_TO_PT_T struct req_tx_sqcb_to_pt_info_t
#define SQCB_TO_WQE_T struct req_tx_sqcb_to_wqe_info_t
#define WQE_TO_SGE_T struct req_tx_wqe_to_sge_info_t
#define TO_STAGE_T struct req_tx_to_stage_t
#define SQCB0_TO_SQCB1_T struct req_tx_sqcb0_to_sqcb1_info_t

%%
    .param    req_tx_sqpt_process
    .param    req_tx_sqwqe_process
    .param    req_tx_sqsge_iterate_process
    .param    req_tx_sqcb1_process
    .param    req_tx_bktrack_sqcb1_process
    //.param    req_tx_add_headers_process
    .param    req_tx_write_back_process
    .param    req_tx_sqcb1_cnp_process

.align
req_tx_sqcb_process:

    // are all rings empty ?
    seq            c1, r7[MAX_SQ_HOST_RINGS-1:0], r0
    bcf            [c1], all_rings_empty

    seq            c1, CAPRI_TXDMA_INTRINSIC_RECIRC_COUNT, 0 // Branch Delay Slot
    bcf            [!c1], process_recirc
    
    // copy intrinsic to global
    add            r1, r0, offsetof(struct phv_, common_global_global_data) //Branch Delay Slot

    // enable below code after spr_tbladdr special purpose register is available in capsim
    #mfspr         r1, spr_tbladdr
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(SQCB_ADDR_SHIFT))
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)
    // Is it UD service?
    seq            c1, d.service, RDMA_SERV_TYPE_UD
    CAPRI_SET_FIELD_C(r1, PHV_GLOBAL_COMMON_T, flags, REQ_TX_FLAG_UD_SERVICE, c1)

    //set dma_cmd_ptr in phv
    TXDMA_DMA_CMD_PTR_SET(REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_START_FLIT_CMD_ID) // Branch Delay Slot

    seq            c1, r7[MAX_SQ_HOST_RINGS-1:1], r0
    bcf            [c1], process_sq

    // if either of the busy flags are set, give up the scheduler opportunity
    crestore [c2,c1], d.{busy...cb1_busy}, 0x3 // Branch Delay Slot
    bcf            [c1 | c2], exit

    // Process CNP packet ring first as its the highest priority.
    seq            c1, CNP_C_INDEX, CNP_P_INDEX
    bcf            [c1], process_credits
    // sqcb1
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r4) 
    tblwr          CNP_C_INDEX, CNP_P_INDEX
    add            r1, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqcb1_cnp_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

    nop.e
    nop

 
process_credits: 
    ARE_ALL_RINGS_EMPTY(c1, r7[MAX_SQ_HOST_RINGS-1:0], FC_RING_ID_BITMAP)
    bcf          [c1], process_sq_or_sq_bktrack
    // take both the busy flags
    tblwr          d.{busy...cb1_busy}, 0x3 //Branch Delay Slot

    // set cindex same as pindex without ringing doorbell, as stage0
    // can get scheduled again while doorbell memwr is still in the queue
    // to be processed. This will cause credits to get updated again. Instead,
    // set cindex to pindex using tblwr. When the queue gets scheduled again, 
    // check for ring empty case and perform doorbell to eval scheduler at that
    // time
    tblwr          FC_C_INDEX, FC_P_INDEX

    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, update_credits, 1)
    //CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, bktrack, 0)

    // sqcb1_p
    add            r1, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqcb1_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)
    
    nop.e
    nop

process_sq_or_sq_bktrack:
    ARE_ALL_RINGS_EMPTY(c1, r7[MAX_SQ_HOST_RINGS-1:0], SQ_BKTRACK_RING_ID_BITMAP|TIMER_RING_ID_BITMAP)
    bcf          [c1], process_sq 
    nop          // Branch Delay Slot 

process_sq_bktrack:
    bbeq           d.in_progress, 1, sq_bktrack
    add            r1, r0, SQ_C_INDEX // Branch Delay Slot

    mincr          r1, d.log_num_wqes, -1
    seq            c1, r1, SQ_P_INDEX
    bcf            [c1], invalid_bktrack

sq_bktrack:

    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)

    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, sq_c_index, r1)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, sq_p_index, SQ_P_INDEX)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, in_progress, d.in_progress)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, bktrack_in_progress, d.bktrack_in_progress)
    CAPRI_SET_FIELD_RANGE(r7, SQCB0_TO_SQCB1_T, current_sge_offset, num_sges, d.{current_sge_offset...num_sges})
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, update_credits, 0)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, bktrack, 1)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, pt_base_addr, d.pt_base_addr)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, op_type, d.curr_op_type)

    add            r1, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_sqcb1_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

    // if (sqcb0_p->in_progress || sqcb0_p->bktracking)
    //     sqcb0_to_sqcb1_info_p->cur_wqe_addr = sqcb0_p->curr_wqe_ptr
    ssle           c1, r0, d.in_progress, d.bktrack_in_progress
    cmov           r1, c1, r0, d.curr_wqe_ptr

    CAPRI_GET_STAGE_1_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)
    CAPRI_SET_FIELD_RANGE(r7, TO_STAGE_T, bktrack.log_pmtu, bktrack.log_num_wqes, d.{log_pmtu...log_num_wqes})

    CAPRI_GET_STAGE_2_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)
    CAPRI_SET_FIELD_RANGE(r7, TO_STAGE_T, bktrack.log_pmtu, bktrack.log_num_wqes, d.{log_pmtu...log_num_wqes})

    CAPRI_GET_STAGE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)
    CAPRI_SET_FIELD_RANGE(r7, TO_STAGE_T, bktrack.log_pmtu, bktrack.log_num_wqes, d.{log_pmtu...log_num_wqes})
 
    CAPRI_GET_STAGE_4_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)
    CAPRI_SET_FIELD_RANGE(r7, TO_STAGE_T, bktrack.log_pmtu, bktrack.log_num_wqes, d.{log_pmtu...log_num_wqes})

    CAPRI_GET_STAGE_5_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)
    CAPRI_SET_FIELD_RANGE(r7, TO_STAGE_T, bktrack.log_pmtu, bktrack.log_num_wqes, d.{log_pmtu...log_num_wqes})

    CAPRI_GET_STAGE_6_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r1)
    CAPRI_SET_FIELD_RANGE(r7, TO_STAGE_T, bktrack.log_pmtu, bktrack.log_num_wqes, d.{log_pmtu...log_num_wqes})

    nop.e
    nop

process_sq:
    seq            c3, d.need_credits, 1
    bcf            [c3], exit
                          
    // if (sqcb0_p->fence) goto fence
    seq            c3, d.fence, 1 // Branch Delay Slot
    bcf            [c3], fence

    // if (sqcb0_p->in_progress) goto in_progress
    seq            c3, 1, d.in_progress // Branch Delay Slot
    bcf            [c3], in_progress

    // Check if spec cindex is pointing to yet to be filled wqe
    seq            c3, SPEC_SQ_C_INDEX, SQ_P_INDEX
    bcf            [c3], exit

    // Use speculative cindex to checkout wqe and start processing if SQCB
    // is not in in_progress state. Before state update at the end of pipeline,
    // check if speculative cindex is matching current cindex. If so,
    // update state, otherwise discard and redo in the next scheduler slot
    add            r1, r0, SPEC_SQ_C_INDEX // Branch Delay Slot

    // log_num_wqe_per_page = (log_sq_page_size - log_wqe_size)
    // page_index = sq_c_index >> (log_num_wqe_per_page)
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
    sllv           r4, 1, r4

    // populate t0 stage to stage data req_tx_sqcb_to_wqe_info_t for next stage
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB_TO_PT_T, page_offset, r1)
    CAPRI_SET_FIELD(r7, SQCB_TO_PT_T, remaining_payload_bytes, r4)
    // TODO Need to check for room in RRQ ring for Read/Atomic before
    // proceeding further. Otherwise recirc until there is room
    CAPRI_SET_FIELD(r7, SQCB_TO_PT_T, rrq_p_index, RRQ_P_INDEX)
    CAPRI_SET_FIELD(r7, SQCB_TO_PT_T, log_pmtu, d.log_pmtu)
    CAPRI_SET_FIELD(r7, SQCB_TO_PT_T, page_seg_offset, r2)
    CAPRI_SET_FIELD(r7, SQCB_TO_PT_T, pd, d.pd)
    //CAPRI_SET_FIELD(r7, SQCB_TO_PT_T, ud, r5)

    // populate t0 PC and table address
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqpt_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r3)

    CAPRI_GET_STAGE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.spec_cindex, SPEC_SQ_C_INDEX)

    CAPRI_GET_STAGE_4_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.spec_cindex, SPEC_SQ_C_INDEX)

    tblmincri      SPEC_SQ_C_INDEX, d.log_num_wqes, 1 

    nop.e
    nop

in_progress:
    // do not speculate for in_progress processing
    bcf            [c1 | c2], exit
    add            r1, r0, SQ_C_INDEX // Branch Delay Slot
    // Assert busy for multi-packet message as each packet has to continue
    // from where the previous packet has left off 
    tblwr          d.{busy...cb1_busy}, 0x3 //Branch Delay Slot

    // load wqe using sqcb_p->wqe_addr

    // sge_offset = TXWQE_SGE_OFFSET + sqcb0_p->current_sge_id * sizeof(sge_t);
    add            r2, TXWQE_SGE_OFFSET, d.current_sge_id, LOG_SIZEOF_SGE_T
    // sge_p = sqcb0_p->curr_wqe_ptr + sge_offset
    add            r2, r2, d.curr_wqe_ptr

    // Use table 3 for sqsge_iterate_process in a consistent way, which will
    // then invoke sqsge_process in stage 3 using table 0
    CAPRI_GET_TABLE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, in_progress, d.in_progress)
    CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, current_sge_id, d.current_sge_id)
    CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, current_sge_offset, d.current_sge_offset)
    // num_valid_sges = sqcb0_p->num_sges = sqcb0_p->current_sge_id
    sub            r3, d.num_sges, d.current_sge_id 
    CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, num_valid_sges, r3)
    // remaining_payload_bytes = (1 >> sqcb0_p->log_pmtu)
    add            r4, r0, d.log_pmtu
    sllv           r4, 1, r4
    CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, remaining_payload_bytes, r4)
    CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, dma_cmd_start_index, REQ_TX_DMA_CMD_PYLD_BASE)
    //CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, wqe_addr, d.curr_wqe_ptr)
    //CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, first, 0)
    CAPRI_SET_FIELD(r7, WQE_TO_SGE_T, op_type, d.curr_op_type)

    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqsge_iterate_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, r6, r2)
    
    //for now, use to_stage_args to pass the wqe_addr
    //until we organize better, copy to all stages
    //CAPRI_GET_STAGE_0_ARG(req_tx_phv_t, r7)
    //CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.wqe_addr, d.curr_wqe_ptr)

    //CAPRI_GET_STAGE_1_ARG(req_tx_phv_t, r7)
    //CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.wqe_addr, d.curr_wqe_ptr)
    //CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.spec_cindex, r1)

    CAPRI_GET_STAGE_2_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.wqe_addr, d.curr_wqe_ptr)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.spec_cindex, r1)

    CAPRI_GET_STAGE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.wqe_addr, d.curr_wqe_ptr)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.spec_cindex, r1)

    CAPRI_GET_STAGE_4_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.wqe_addr, d.curr_wqe_ptr)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.spec_cindex, r1)

    //CAPRI_GET_STAGE_5_ARG(req_tx_phv_t, r7)
    //CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.wqe_addr, d.curr_wqe_ptr)

    //CAPRI_GET_STAGE_6_ARG(req_tx_phv_t, r7)
    //CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.wqe_addr, d.curr_wqe_ptr)

    //CAPRI_GET_STAGE_7_ARG(req_tx_phv_t, r7)
    //CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.wqe_addr, d.curr_wqe_ptr)

    nop.e
    nop

process_recirc:
    seq            c1, CAPRI_APP_DATA_RECIRC_REASON, REQ_TX_RECIRC_REASON_SGE_WORK_PENDING
    bcf            [c1], process_sge_recirc
    phvwr          p.common.p4_intr_recirc, 0 // Branch Delay Slot
    
    nop.e
    nop

process_sge_recirc:
    // nothing to be done here, table 3 is programmed to execute req_tx_sqsge_process
    nop.e
    nop

fence:
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    //CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, in_progress, 0)
    //CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, current_sge_id, 0)
    //CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, current_sge_offset, 0)
    //CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, num_valid_sges, 0)
    //CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, remaining_payload_bytes, 0)
    //CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, wqe_ptr, d.curr_wqe_ptr)
    CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, rrq_p_index, RRQ_P_INDEX)
    CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, log_pmtu, d.log_pmtu)
    //CAPRI_SET_FIELD(r7, SQCB_TO_WQE_T, li_fence_cleared, 0)

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqwqe_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, d.curr_wqe_ptr)

end:
    nop.e
    nop

all_rings_empty:
    // ring doorbell to re-evaluate scheduler
    DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r2, r3)

exit:
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop

invalid_bktrack:
    phvwr  p.common.p4_intr_global_drop, 1
    // TODO Error Disable the QP
    // fall through to unlock and exit

unlock_and_exit:
    tblwr          d.busy, 0
    tblwr          d.cb1_busy, 0
    nop.e
    nop
