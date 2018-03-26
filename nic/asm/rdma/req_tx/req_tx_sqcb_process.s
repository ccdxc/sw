#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct req_tx_s0_t0_k k;

#define SQCB_TO_PT_T t0_s2s_sqcb_to_pt_info
#define SQCB_TO_WQE_T t0_s2s_sqcb_to_wqe_info
#define WQE_TO_SGE_T t2_s2s_wqe_to_sge_info
#define SQCB0_TO_SQCB2_T t0_s2s_sqcb0_to_sqcb2_info

#define SPEC_LEN 32
#define LOG_SPEC_LEN 5

#define TO_S1_P to_s1_sq_to_stage
#define TO_S2_P to_s2_sq_to_stage
#define TO_S3_P to_s3_sq_to_stage
#define TO_S4_P to_s4_sq_to_stage
#define TO_S5_P to_s5_sq_to_stage

#define TO_S1_BKTRACK_P to_s1_bktrack_to_stage
#define TO_S2_BKTRACK_P to_s2_bktrack_to_stage
#define TO_S3_BKTRACK_P to_s3_bktrack_to_stage
#define TO_S4_BKTRACK_P to_s4_bktrack_to_stage
#define TO_S5_BKTRACK_P to_s5_bktrack_to_stage
#define TO_S6_BKTRACK_P to_s6_bktrack_to_stage


%%
    .param    req_tx_sqpt_process
    .param    req_tx_dummy_sqpt_process
    .param    req_tx_sqsge_iterate_process
    .param    req_tx_credits_process
    .param    req_tx_bktrack_sqcb2_process
    .param    req_tx_sqcb2_cnp_process

.align
req_tx_sqcb_process:

    seq            c1, CAPRI_TXDMA_INTRINSIC_RECIRC_COUNT, 0
    bcf            [!c1], process_recirc
    nop

#   // moving to _ext program
#   // copy intrinsic to global
#   add            r1, r0, offsetof(struct phv_, common_global_global_data) 

#   // enable below code after spr_tbladdr special purpose register is available in capsim
#   #mfspr         r1, spr_tbladdr
#   CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(SQCB_ADDR_SHIFT))
#   CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
#   CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
#   CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)
#   // Is it UD service?
#   seq            c1, d.service, RDMA_SERV_TYPE_UD
#   CAPRI_SET_FIELD_C(r1, PHV_GLOBAL_COMMON_T, flags, REQ_TX_FLAG_UD_SERVICE, c1)

#   //set dma_cmd_ptr in phv
#   TXDMA_DMA_CMD_PTR_SET(REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_START_FLIT_CMD_ID)

    .brbegin
    brpri          r7[MAX_SQ_HOST_RINGS-1: 0], [CNP_PRI, TIMER_PRI, SQ_BKTRACK_PRI, FC_PRI, SQ_PRI]
    nop

    .brcase        SQ_RING_ID

        bbeq           d.poll_in_progress, 1, exit
        crestore [c2,c1], d.{busy...cb1_busy}, 0x3 //BD Slot
        
        // Load sqcb1 to fetch dcqcn_cb addr if congestion_mgmt is enabled.
        bbeq           d.congestion_mgmt_enable, 0, process_send

        CAPRI_SET_FIELD2(TO_S2_P, header_template_addr, d.header_template_addr) //BD Slot
        CAPRI_SET_FIELD2(TO_S2_P, congestion_mgmt_enable, 1)

        CAPRI_SET_FIELD2(TO_S3_P, header_template_addr, d.header_template_addr)
        CAPRI_SET_FIELD2(TO_S3_P, congestion_mgmt_enable, 1)

        bbeq           d.dcqcn_rl_failure, 0, process_send
        CAPRI_SET_FIELD2(TO_S4_P, congestion_mgmt_enable, 1) // Branch Delay Slot
        // Reset spec-cindex to cindex and resend packet on dcqcn-rl-failure.
        tblwr          SPEC_SQ_C_INDEX, SQ_C_INDEX
        tblwr          d.dcqcn_rl_failure, 0

process_send:
        bbeq           d.need_credits, 1, exit
                              
        // if (sqcb0_p->fence) goto fence
        seq            c3, d.fence, 1 // Branch Delay Slot
        bcf            [c3], fence
        
        // if (sqcb0_p->in_progress) goto in_progress
        seq            c3, 1, d.in_progress // Branch Delay Slot
        bcf            [c3], in_progress
        
        // Check if spec cindex is pointing to yet to be filled wqe
        seq            c3, SPEC_SQ_C_INDEX, SQ_P_INDEX
        bcf            [c3], exit
        
poll_for_work:

        //Remove this piece with a simple copy from sqcb0 to to_stage when there is room
        //in sqcb0. Also remove the called function
        //Load sqcb1 to fetch hdr_template_addr 
        // To hide read latency for header template addr, pass it to stage 4
        // so that it can be read in parallel to other HBM access in that stage.
        // TODO this should go in parallel to wqe/sge stage
        CAPRI_SET_FIELD2(TO_S4_P, header_template_addr, d.header_template_addr)

        // reset sched_eval_done 
        tblwr          d.ring_empty_sched_eval_done, 0

        // Use speculative cindex to checkout wqe and start processing if SQCB
        // is not in in_progress state. Before state update at the end of pipeline,
        // check if speculative cindex is matching current cindex. If so,
        // update state, otherwise discard and redo in the next scheduler slot
        add            r1, r0, SPEC_SQ_C_INDEX
        
        bbne           d.sq_in_hbm, 1, pt_process
        sll            r2, r1, d.log_wqe_size // Branch Delay Slot
        add            r2, r2, d.hbm_sq_base_addr, HBM_SQ_BASE_ADDR_SHIFT
        // remaining_payload_bytes = (1 << sqcb0_p->log_pmtu), to start with
        sll            r4, 1, d.log_pmtu
        
        // populate t0 stage to stage data req_tx_sqcb_to_wqe_info_t for next stage
        CAPRI_RESET_TABLE_0_ARG()
        CAPRI_SET_FIELD2(SQCB_TO_WQE_T, log_pmtu, d.log_pmtu)
        CAPRI_SET_FIELD2(SQCB_TO_WQE_T, poll_in_progress, d.poll_in_progress)
        CAPRI_SET_FIELD2(SQCB_TO_WQE_T, color, d.color)
        CAPRI_SET_FIELD2(SQCB_TO_WQE_T, remaining_payload_bytes, r4)

        CAPRI_SET_FIELD2(TO_S4_P, spec_cindex, SPEC_SQ_C_INDEX)
        
        CAPRI_SET_FIELD2(TO_S5_P, wqe_addr, r2)
        CAPRI_SET_FIELD2(TO_S5_P, spec_cindex, SPEC_SQ_C_INDEX)
        
        // populate t0 PC and table address
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dummy_sqpt_process, r2)
        
        seq            c1, d.poll_in_progress, 0x1
        tblmincri.!c1  SPEC_SQ_C_INDEX, d.log_num_wqes, 1 
        // Revert speculative cindex if its far apart from actual cindex
        sub            r1, SPEC_SQ_C_INDEX, SQ_C_INDEX
        mincr          r1, d.log_num_wqes, 0
        blti           r1, SPEC_LEN, end1
        add            r1, SQ_C_INDEX, r0
        mincr.e        r1, d.log_num_wqes, 1
        tblwr          SPEC_SQ_C_INDEX, r1

pt_process:
        // log_num_wqe_per_page = (log_sq_page_size - log_wqe_size)
        // page_index = sq_c_index >> (log_num_wqe_per_page)
        sub            r2, d.log_sq_page_size, d.log_wqe_size
        srlv           r3, r1, r2
        
        // page_offset = ((sq_c_index & ((1 << log_num_wqe_per_page) -1)) << log_wqe_size)
        mincr          r1, r2, r0
        sll            r1, r1, d.log_wqe_size
        
        // page_seg_offset = page_index & 0x7
        and            r2, r3, CAPRI_SEG_PAGE_MASK
        
        // page_index = page_index & ~0x7
        // page_p = sqcb_p->pt_base_addr + (page_index * sizoef(u64))
        sub            r3, r3, r2
        sll            r5, d.pt_base_addr, PT_BASE_ADDR_SHIFT
        add            r3, r5, r3, CAPRI_LOG_SIZEOF_U64
        
        // remaining_payload_bytes = (1 << sqcb0_p->log_pmtu), to start with
        sll            r4, 1, d.log_pmtu
        
        // populate t0 stage to stage data req_tx_sqcb_to_wqe_info_t for next stage
        CAPRI_RESET_TABLE_0_ARG()
        CAPRI_SET_FIELD2(SQCB_TO_PT_T, page_offset, r1)
        CAPRI_SET_FIELD2(SQCB_TO_PT_T, remaining_payload_bytes, r4)
        // TODO Need to check for room in RRQ ring for Read/Atomic before
        // proceeding further. Otherwise recirc until there is room
        CAPRI_SET_FIELD2(SQCB_TO_PT_T, page_seg_offset, r2)
        //write pd, log_pmtu together
        CAPRI_SET_FIELD_RANGE2(SQCB_TO_PT_T, pd, log_pmtu, d.{pd...log_pmtu})
        CAPRI_SET_FIELD2(SQCB_TO_PT_T, poll_in_progress, d.poll_in_progress)
        CAPRI_SET_FIELD2(SQCB_TO_PT_T, color, d.color)
        
        // populate t0 PC and table address
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqpt_process, r3)

        CAPRI_SET_FIELD2(TO_S4_P, spec_cindex, SPEC_SQ_C_INDEX)
        
        CAPRI_SET_FIELD2(TO_S5_P, spec_cindex, SPEC_SQ_C_INDEX)
        
        seq            c1, d.poll_in_progress, 0x1
        tblmincri.!c1  SPEC_SQ_C_INDEX, d.log_num_wqes, 1 
        // Revert speculative cindex if its far apart from actual cindex
        sub            r1, SPEC_SQ_C_INDEX, SQ_C_INDEX
        mincr          r1, d.log_num_wqes, 0
        blti           r1, SPEC_LEN, end1
        add            r1, SQ_C_INDEX, r0
        mincr.e        r1, d.log_num_wqes, 1
        tblwr          SPEC_SQ_C_INDEX, r1

in_progress:
        // do not speculate for in_progress processing
        bcf            [c1 | c2], exit
        add            r1, r0, SQ_C_INDEX // Branch Delay Slot
        // Assert busy for multi-packet message as each packet has to continue
        // from where the previous packet has left off 
        tblwr          d.{busy...cb1_busy}, 0x3
        
        // load wqe using sqcb_p->wqe_addr
        
        // sge_offset = TXWQE_SGE_OFFSET + sqcb0_p->current_sge_id * sizeof(sge_t);
        add            r2, TXWQE_SGE_OFFSET, d.current_sge_id, LOG_SIZEOF_SGE_T
        // sge_p = sqcb0_p->curr_wqe_ptr + sge_offset
        add            r2, r2, d.curr_wqe_ptr
        
        // Use table 2 for sqsge_iterate_process in a consistent way, which will
        // then invoke sqsge_process in stage 3 using table 0
        CAPRI_RESET_TABLE_2_ARG()
        CAPRI_SET_FIELD2(WQE_TO_SGE_T, in_progress, d.in_progress)
        CAPRI_SET_FIELD2(WQE_TO_SGE_T, current_sge_id, d.current_sge_id)
        CAPRI_SET_FIELD2(WQE_TO_SGE_T, current_sge_offset, d.current_sge_offset)
        // num_valid_sges = sqcb0_p->num_sges = sqcb0_p->current_sge_id
        sub            r3, d.num_sges, d.current_sge_id 
        CAPRI_SET_FIELD2(WQE_TO_SGE_T, num_valid_sges, r3)
        // remaining_payload_bytes = (1 >> sqcb0_p->log_pmtu)
        sll            r4, 1, d.log_pmtu
        CAPRI_SET_FIELD2(WQE_TO_SGE_T, remaining_payload_bytes, r4)
        CAPRI_SET_FIELD2(WQE_TO_SGE_T, dma_cmd_start_index, REQ_TX_DMA_CMD_PYLD_BASE)
        
        CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_sqsge_iterate_process, r2)
        
        CAPRI_SET_FIELD2(TO_S2_P, wqe_addr, d.curr_wqe_ptr)
        
        CAPRI_SET_FIELD2(TO_S3_P, wqe_addr, d.curr_wqe_ptr)

        CAPRI_SET_FIELD2(TO_S4_P, spec_cindex, r1)
        
        CAPRI_SET_FIELD2(TO_S5_P, wqe_addr, d.curr_wqe_ptr)
        CAPRI_SET_FIELD2(TO_S5_P, spec_cindex, r1)

        mincr.e        r1, d.log_num_wqes, 1
        tblwr          SPEC_SQ_C_INDEX, r1
        
end1:
        nop.e
        nop

    .brcase        FC_RING_ID
        bbeq             d.cb1_busy, 1, exit
        // reset sched_eval_done 
        tblwr          d.ring_empty_sched_eval_done, 0 // Branch Delay Slot
        
        tblwr            d.cb1_busy, 1
        
        // set cindex same as pindex without ringing doorbell, as stage0
        // can get scheduled again while doorbell memwr is still in the queue
        // to be processed. This will cause credits to get updated again. Instead,
        // set cindex to pindex using tblwr. When the queue gets scheduled again, 
        // check for ring empty case and perform doorbell to eval scheduler at that
        // time
        tblwr          FC_C_INDEX, FC_P_INDEX
        
        // sqcb1_p
        CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_credits_process, r0)
        
        nop.e
        nop

    .brcase        SQ_BKTRACK_RING_ID
        crestore       [c2,c1], d.{busy...cb1_busy}, 0x3
        bcf            [c1 | c2], exit

        // reset sched_eval_done 
        tblwr          d.ring_empty_sched_eval_done, 0 // Branch Delay Slot
        // take both the busy flags
        tblwr          d.{busy...cb1_busy}, 0x3
        
        sslt           c1, r0, d.in_progress, d.bktrack_in_progress
        bcf            [c1], sq_bktrack1
        add            r1, r0, SQ_C_INDEX // Branch Delay Slot
        
        mincr          r1, d.log_num_wqes, -1
        seq            c2, r1, SQ_P_INDEX
        bcf            [c2], invalid_bktrack

sq_bktrack1:
        CAPRI_RESET_TABLE_0_ARG()
        
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, sq_c_index, r1)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, sq_p_index, SQ_P_INDEX)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, in_progress, d.in_progress)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, bktrack_in_progress, d.bktrack_in_progress)
        CAPRI_SET_FIELD_RANGE2(SQCB0_TO_SQCB2_T, current_sge_offset, num_sges, d.{current_sge_offset...num_sges})
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, update_credits, 0)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, bktrack, 1)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, pt_base_addr, d.pt_base_addr)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, sq_in_hbm, d.sq_in_hbm)
        
        add            r1, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES*2)
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqcb2_process, r1)
        
        // if (sqcb0_p->in_progress || sqcb0_p->bktracking)
        //     sqcb0_to_sqcb1_info_p->cur_wqe_addr = sqcb0_p->curr_wqe_ptr
        cmov           r1, c1, d.curr_wqe_ptr, r0
        
        //copy backtrack params to TO_STAGE: log_pmtu, log_sq_page_size, log_wqe_size, log_num_wqes
        
        CAPRI_SET_FIELD2(TO_S1_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S1_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})
        
        CAPRI_SET_FIELD2(TO_S2_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S2_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})
        
        CAPRI_SET_FIELD2(TO_S3_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S3_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})
        
        CAPRI_SET_FIELD2(TO_S4_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S4_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})
        
        CAPRI_SET_FIELD2(TO_S5_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S5_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})
        
        CAPRI_SET_FIELD2(TO_S6_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S6_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})
        
        nop.e
        nop

    .brcase        TIMER_RING_ID
        crestore       [c2,c1], d.{busy...cb1_busy}, 0x3
        bcf            [c1 | c2], exit

        // reset sched_eval_done 
        tblwr          d.ring_empty_sched_eval_done, 0 // Branch Delay Slot

        // take both the busy flags
        tblwr          d.{busy...cb1_busy}, 0x3
        
        sslt           c1, r0, d.in_progress, d.bktrack_in_progress
        bcf            [c1], sq_bktrack2
        add            r1, r0, SQ_C_INDEX // Branch Delay Slot
        
        mincr          r1, d.log_num_wqes, -1
        seq            c2, r1, SQ_P_INDEX
        bcf            [c2], invalid_bktrack

sq_bktrack2:
        CAPRI_RESET_TABLE_0_ARG()
        
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, sq_c_index, r1)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, sq_p_index, SQ_P_INDEX)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, in_progress, d.in_progress)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, bktrack_in_progress, d.bktrack_in_progress)
        CAPRI_SET_FIELD_RANGE2(SQCB0_TO_SQCB2_T, current_sge_offset, num_sges, d.{current_sge_offset...num_sges})
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, update_credits, 0)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, bktrack, 1)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, pt_base_addr, d.pt_base_addr)
        CAPRI_SET_FIELD2(SQCB0_TO_SQCB2_T, sq_in_hbm, d.sq_in_hbm)
        
        add            r1, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES*2)
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqcb2_process, r1)
        
        // if (sqcb0_p->in_progress || sqcb0_p->bktracking)
        //     sqcb0_to_sqcb1_info_p->cur_wqe_addr = sqcb0_p->curr_wqe_ptr
        cmov           r1, c1, d.curr_wqe_ptr, r0
        
        //copy backtrack params to TO_STAGE: log_pmtu, log_sq_page_size, log_wqe_size, log_num_wqes
        CAPRI_SET_FIELD2(TO_S1_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S1_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})

        CAPRI_SET_FIELD2(TO_S2_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S2_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})

        CAPRI_SET_FIELD2(TO_S3_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S3_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})

        CAPRI_SET_FIELD2(TO_S4_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S4_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})

        CAPRI_SET_FIELD2(TO_S5_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S5_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})

        CAPRI_SET_FIELD2(TO_S6_BKTRACK_P, wqe_addr, r1)
        CAPRI_SET_FIELD_RANGE2(TO_S6_BKTRACK_P, log_pmtu, log_num_wqes, d.{log_pmtu...log_num_wqes})

        nop.e
        nop

    .brcase        CNP_RING_ID
        // reset sched_eval_done 
        tblwr          d.ring_empty_sched_eval_done, 0
        
        tblwr          CNP_C_INDEX, CNP_P_INDEX
        add            r1, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqcb2_cnp_process, r1)
        nop.e
        nop

    .brcase        MAX_SQ_HOST_RINGS

        crestore  [c2], d.poll_for_work, 0x1
        crestore  [c1], d.poll_in_progress, 0x1
        #c2 - poll_for_work
        #c1 - poll_in_progress

        setcf c3, [c2 & !c1]
        bcf   [c3], poll_for_work
        tblwr.c3 d.poll_in_progress, 1 //BD Slot

        bbeq    d.ring_empty_sched_eval_done, 1, exit
        nop     //BD Slot                        
        
        // ring doorbell to re-evaluate scheduler
        DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r2, r3)
        tblwr   d.ring_empty_sched_eval_done, 1
        
        phvwr   p.common.p4_intr_global_drop, 1
        nop.e
        nop
    .brend

process_recirc:
    // reset sched_eval_done 
    tblwr          d.ring_empty_sched_eval_done, 0

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
    CAPRI_RESET_TABLE_0_ARG()
    //CAPRI_SET_FIELD2(SQCB_TO_WQE_T, in_progress, 0)
    //CAPRI_SET_FIELD2(SQCB_TO_WQE_T, current_sge_id, 0)
    //CAPRI_SET_FIELD2(SQCB_TO_WQE_T, current_sge_offset, 0)
    //CAPRI_SET_FIELD2(SQCB_TO_WQE_T, num_valid_sges, 0)
    //CAPRI_SET_FIELD2(SQCB_TO_WQE_T, remaining_payload_bytes, 0)
    //CAPRI_SET_FIELD2(SQCB_TO_WQE_T, wqe_ptr, d.curr_wqe_ptr)
    CAPRI_SET_FIELD2(SQCB_TO_WQE_T, log_pmtu, d.log_pmtu)
    //CAPRI_SET_FIELD2(SQCB_TO_WQE_T, li_fence_cleared, 0)

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dummy_sqpt_process, d.curr_wqe_ptr)

end:
    nop.e
    nop

invalid_bktrack:
    phvwr  p.common.p4_intr_global_drop, 1
    // TODO Error Disable the QP
    // fall through to unlock and exit

unlock_and_exit:
    tblwr          d.{busy...cb1_busy}, 0
    nop.e
    nop

exit:
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop
