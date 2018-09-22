#include "capri.h"
#include "common_defines.h"
#include "nic/p4/common/defines.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct req_tx_s0_t0_k k;

#define SQCB_TO_PT_P t0_s2s_sqcb_to_pt_info
#define SQCB_TO_WQE_P t0_s2s_sqcb_to_wqe_info
#define WQE_TO_SGE_P t2_s2s_wqe_to_sge_info
#define SQCB0_TO_SQCB2_P t0_s2s_sqcb0_to_sqcb2_info
#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info
#define WQE_TO_FRPMR_LKEY_T0 t0_s2s_sqwqe_to_lkey_frpmr_info
#define WQE_TO_FRPMR_LKEY_T1 t1_s2s_sqwqe_to_lkey_frpmr_info

#define SPEC_LEN 32
#define LOG_SPEC_LEN 5

#define TO_S1_FENCE_P           to_s1_fence_info
#define TO_S2_SQWQE_P           to_s2_sqwqe_info
#define TO_S3_SQSGE_P           to_s3_sqsge_info
#define TO_S4_DCQCN_BIND_MW_P   to_s4_dcqcn_bind_mw_info
#define TO_S1_DCQCN_BIND_MW_P   to_s1_dcqcn_bind_mw_info
#define TO_S5_SQCB_WB_P         to_s5_sqcb_wb_info

#define TO_S1_BT_P to_s1_bt_info
#define TO_S2_BT_P to_s2_bt_info
#define TO_S3_BT_P to_s3_bt_info
#define TO_S4_BT_P to_s4_bt_info
#define TO_S5_BT_P to_s5_bt_info
#define TO_S6_BT_P to_s6_bt_info


%%
    .param    req_tx_sqpt_process
    .param    req_tx_dummy_sqpt_process
    .param    req_tx_sqsge_iterate_process
    .param    req_tx_credits_process
    .param    req_tx_bktrack_sqcb2_process
    .param    req_tx_sqcb2_cnp_process
    .param    req_tx_timer_expiry_process
    .param    req_tx_sq_drain_feedback_process
    .param    req_tx_sqcb2_fence_process
    .param    req_tx_dcqcn_enforce_process
    .param    req_tx_bind_mw_rkey_process

.align
req_tx_sqcb_process:
    // Bypass state ccheck for recirc pkts as state has already been checked
    // in the previous pass.
    seq            c1, CAPRI_TXDMA_INTRINSIC_RECIRC_COUNT, 0
    bcf            [!c1], process_recirc

    // If QP is not in RTS state, do no process any event. Branch to check for
    // drain state and process only if SQ has to be drained till a specific WQE
    seq            c7, d.state, QP_STATE_RTS
    bcf            [!c7], check_state
    tblwr.c7       d.sq_drained, 0 // Branch Delay Slot

process_req_tx:
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
    brpri          r7[MAX_SQ_DOORBELL_RINGS-1: 0], [CNP_PRI, TIMER_PRI, SQ_BKTRACK_PRI, SQ_PRI]
    nop

    .brcase        SQ_RING_ID

        crestore [c2,c1], d.{busy...cb1_busy}, 0x3
        bcf            [c1 | c2], exit
        nop // Branch Delay Slot
  
        bbeq           d.poll_in_progress, 1, exit
        nop // Branch Delay Slot
        
        bbeq           d.congestion_mgmt_enable, 0, process_send

        phvwrpair CAPRI_PHV_FIELD(TO_S3_SQSGE_P, header_template_addr), d.header_template_addr, \
                  CAPRI_PHV_FIELD(TO_S3_SQSGE_P, congestion_mgmt_enable), d.congestion_mgmt_enable //BD-Slot

        bbeq           d.dcqcn_rl_failure, 0, process_send
        phvwr          CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, congestion_mgmt_enable), 1  // Branch Delay Slot
        // Reset spec-cindex to cindex and resend packet on dcqcn-rl-failure.
        tblwr          SPEC_SQ_C_INDEX, SQ_C_INDEX
        tblwr          d.dcqcn_rl_failure, 0

process_send:
        bbeq           d.frpmr_in_progress, 1, frpmr
        nop // Branch Delay Slot

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

        // header_template_addr is needed to load hdr_template_process in fast-path.
        phvwrpair      CAPRI_PHV_FIELD(TO_S2_SQWQE_P, header_template_addr), d.header_template_addr, \
                       CAPRI_PHV_FIELD(TO_S2_SQWQE_P, fast_reg_rsvd_lkey_enable), d.priv_oper_enable // BD-slot

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
        phvwrpair CAPRI_PHV_FIELD(SQCB_TO_WQE_P, log_pmtu), d.log_pmtu, \
                  CAPRI_PHV_FIELD(SQCB_TO_WQE_P, poll_in_progress), d.poll_in_progress
        phvwrpair CAPRI_PHV_FIELD(SQCB_TO_WQE_P, remaining_payload_bytes), r4, \
                  CAPRI_PHV_FIELD(SQCB_TO_WQE_P, color), d.color
        phvwr     CAPRI_PHV_FIELD(SQCB_TO_WQE_P, current_sge_offset), d.read_req_adjust

        phvwrpair CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, header_template_addr_or_pd), d.pd, \
                  CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, spec_cindex), SPEC_SQ_C_INDEX 
 
        phvwrpair CAPRI_PHV_FIELD(TO_S5_SQCB_WB_P, wqe_addr), r2, \
                  CAPRI_PHV_FIELD(TO_S5_SQCB_WB_P, spec_cindex), SPEC_SQ_C_INDEX

        phvwr     CAPRI_PHV_FIELD(TO_S3_SQSGE_P, priv_oper_enable), d.priv_oper_enable
                  
        
        // populate t0 PC and table address
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dummy_sqpt_process, r2)
        
        seq.e          c1, d.poll_in_progress, 0x1
        tblmincri.!c1  SPEC_SQ_C_INDEX, d.log_num_wqes, 1 

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
        phvwrpair CAPRI_PHV_FIELD(SQCB_TO_PT_P, page_offset), r1, \
                  CAPRI_PHV_FIELD(SQCB_TO_PT_P, remaining_payload_bytes), r4
        // TODO Need to check for room in RRQ ring for Read/Atomic before
        // proceeding further. Otherwise recirc until there is room
        //write pd, log_pmtu together
        phvwrpair CAPRI_PHV_FIELD(SQCB_TO_PT_P, page_seg_offset), r2, \
                  CAPRI_PHV_RANGE(SQCB_TO_PT_P, pd, log_pmtu), d.{pd...log_pmtu}
        phvwrpair CAPRI_PHV_FIELD(SQCB_TO_PT_P, poll_in_progress), d.poll_in_progress, \
                  CAPRI_PHV_FIELD(SQCB_TO_PT_P, color), d.color
        phvwr     CAPRI_PHV_FIELD(SQCB_TO_PT_P, read_req_adjust), d.read_req_adjust
        // populate t0 PC and table address
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqpt_process, r3)

        phvwr     CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, spec_cindex), SPEC_SQ_C_INDEX
        phvwr     CAPRI_PHV_FIELD(TO_S5_SQCB_WB_P, spec_cindex), SPEC_SQ_C_INDEX

        phvwr     CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, header_template_addr_or_pd), d.pd        

        phvwr     CAPRI_PHV_FIELD(TO_S3_SQSGE_P, priv_oper_enable), d.priv_oper_enable

        seq.e          c1, d.poll_in_progress, 0x1
        tblmincri.!c1  SPEC_SQ_C_INDEX, d.log_num_wqes, 1 

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
        phvwrpair CAPRI_PHV_FIELD(WQE_TO_SGE_P, in_progress), d.in_progress, \
                  CAPRI_PHV_FIELD(WQE_TO_SGE_P, current_sge_id), d.current_sge_id
        // num_valid_sges = sqcb0_p->num_sges = sqcb0_p->current_sge_id
        sub            r3, d.num_sges, d.current_sge_id 
        phvwrpair CAPRI_PHV_FIELD(WQE_TO_SGE_P, num_valid_sges), r3, \
                  CAPRI_PHV_FIELD(WQE_TO_SGE_P, current_sge_offset), d.current_sge_offset
        // remaining_payload_bytes = (1 >> sqcb0_p->log_pmtu)
        sll            r4, 1, d.log_pmtu
        phvwrpair CAPRI_PHV_FIELD(WQE_TO_SGE_P, remaining_payload_bytes), r4, \
                  CAPRI_PHV_FIELD(WQE_TO_SGE_P, dma_cmd_start_index), REQ_TX_DMA_CMD_PYLD_BASE
        
        CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_sqsge_iterate_process, r2)
        
        phvwr CAPRI_PHV_FIELD(TO_S2_SQWQE_P, wqe_addr), d.curr_wqe_ptr
        //          CAPRI_PHV_FIELD(TO_S3_SQSGE_P, wqe_addr), d.curr_wqe_ptr

        phvwrpair CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, header_template_addr_or_pd), d.pd, \
                  CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, spec_cindex), r1
        
        phvwrpair CAPRI_PHV_FIELD(TO_S5_SQCB_WB_P, wqe_addr), d.curr_wqe_ptr, \
                  CAPRI_PHV_FIELD(TO_S5_SQCB_WB_P, spec_cindex), r1

        mincr.e        r1, d.log_num_wqes, 1
        tblwr          SPEC_SQ_C_INDEX, r1
        
end1:
        nop.e
        nop

    #.brcase        FC_RING_ID
    #    bbeq             d.cb1_busy, 1, exit
    #    // reset sched_eval_done 
    #    tblwr          d.ring_empty_sched_eval_done, 0 // Branch Delay Slot
    #    
    #    tblwr            d.cb1_busy, 1
    #    
    #    // set cindex same as pindex without ringing doorbell, as stage0
    #    // can get scheduled again while doorbell memwr is still in the queue
    #    // to be processed. This will cause credits to get updated again. Instead,
    #    // set cindex to pindex using tblwr. When the queue gets scheduled again, 
    #    // check for ring empty case and perform doorbell to eval scheduler at that
    #    // time
    #    tblwr          FC_C_INDEX, FC_P_INDEX
    #    
    #    // sqcb1_p
    #    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_credits_process, r0)
    #    
    #    nop.e
    #    nop

    .brcase        SQ_BKTRACK_RING_ID
        crestore       [c2,c1], d.{busy...cb1_busy}, 0x3
        bcf            [c1 | c2], exit

        // reset sched_eval_done 
        tblwr          d.ring_empty_sched_eval_done, 0 // Branch Delay Slot

        // start backtrack process only if there is no outstanding
        // phvs being speculated. If sqcb0's c_index is same as spec_cindex
        // then there is no phvs in the pipeline. In case of in_progress
        // processing, spec_sq_cindex is always sq_c_index+1 and speculation
        // is disabled. So backracking can start in the middle of in_progress
        // processing, when busy flags are not set
        sne            c1, SQ_C_INDEX, SPEC_SQ_C_INDEX
        sne            c2, d.in_progress, 1
        bcf            [c1 & c2] , exit
        sslt           c1, r0, d.in_progress, d.bktrack_in_progress // Branch Delay Slot

        // take both the busy flags
        tblwr          d.{busy...cb1_busy}, 0x3
        tblwr          d.bktrack_in_progress, 1
        
        bcf            [c1], sq_bktrack1
        add            r1, r0, SQ_C_INDEX // Branch Delay Slot
        mincr          r1, d.log_num_wqes, -1
        seq            c2, r1, SQ_P_INDEX
        bcf            [c2], invalid_bktrack

sq_bktrack1:
        CAPRI_RESET_TABLE_0_ARG()
        
        phvwrpair CAPRI_PHV_FIELD(SQCB0_TO_SQCB2_P, sq_c_index), r1, \
                  CAPRI_PHV_FIELD(SQCB0_TO_SQCB2_P, in_progress), d.in_progress
        phvwr     CAPRI_PHV_FIELD(SQCB0_TO_SQCB2_P, sq_p_index), SQ_P_INDEX //not eligible for phvwrpair
        phvwrpair CAPRI_PHV_FIELD(SQCB0_TO_SQCB2_P, bktrack_in_progress), d.bktrack_in_progress, \
                  CAPRI_PHV_RANGE(SQCB0_TO_SQCB2_P, current_sge_offset, num_sges), d.{current_sge_offset...num_sges}
        phvwrpair CAPRI_PHV_FIELD(SQCB0_TO_SQCB2_P, update_credits), 0, \
                  CAPRI_PHV_FIELD(SQCB0_TO_SQCB2_P, bktrack), 1
        phvwrpair CAPRI_PHV_FIELD(SQCB0_TO_SQCB2_P, pt_base_addr), d.pt_base_addr, \
                  CAPRI_PHV_FIELD(SQCB0_TO_SQCB2_P, sq_in_hbm), d.sq_in_hbm
        
        add            r1, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES*2)
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqcb2_process, r1)
        
        // if (sqcb0_p->in_progress || sqcb0_p->bktracking)
        //     sqcb0_to_sqcb1_info_p->cur_wqe_addr = sqcb0_p->curr_wqe_ptr
        cmov           r1, c1, d.curr_wqe_ptr, r0
        
        //copy backtrack params to TO_STAGE: log_pmtu, log_sq_page_size, log_wqe_size, log_num_wqes
        
        phvwrpair CAPRI_PHV_FIELD(TO_S1_BT_P, wqe_addr), r1, \
                  CAPRI_PHV_RANGE(TO_S1_BT_P, log_pmtu, log_num_wqes), d.{log_pmtu...log_num_wqes}

        phvwrpair CAPRI_PHV_FIELD(TO_S2_BT_P, wqe_addr), r1, \
                  CAPRI_PHV_RANGE(TO_S2_BT_P, log_pmtu, log_num_wqes), d.{log_pmtu...log_num_wqes}

        phvwrpair CAPRI_PHV_FIELD(TO_S3_BT_P, wqe_addr), r1, \
                  CAPRI_PHV_RANGE(TO_S3_BT_P, log_pmtu, log_num_wqes), d.{log_pmtu...log_num_wqes}

        phvwrpair CAPRI_PHV_FIELD(TO_S4_BT_P, wqe_addr), r1, \
                  CAPRI_PHV_RANGE(TO_S4_BT_P, log_pmtu, log_num_wqes), d.{log_pmtu...log_num_wqes}

        phvwrpair.e CAPRI_PHV_FIELD(TO_S5_BT_P, wqe_addr), r1, \
                    CAPRI_PHV_RANGE(TO_S5_BT_P, log_pmtu, log_num_wqes), d.{log_pmtu...log_num_wqes}

        phvwrpair CAPRI_PHV_FIELD(TO_S6_BT_P, wqe_addr), r1, \
                  CAPRI_PHV_RANGE(TO_S6_BT_P, log_pmtu, log_num_wqes), d.{log_pmtu...log_num_wqes}
        
    .brcase        TIMER_RING_ID
        // reset sched_eval_done 
        tblwr          d.ring_empty_sched_eval_done, 0
        tblwr          SQ_TIMER_C_INDEX, SQ_TIMER_P_INDEX

        // If SQ is already being bktracked then just restart the timer
        // and drop phv.
        bbeq           d.bktrack_in_progress, 1, restart_timer
        CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_timer_expiry_process, r0)
        nop.e
        nop

restart_timer:
        CAPRI_START_SLOW_TIMER(r1, r2, CAPRI_TXDMA_INTRINSIC_LIF, \
                               CAPRI_TXDMA_INTRINSIC_QTYPE, \
                               CAPRI_TXDMA_INTRINSIC_QID, TIMER_RING_ID, 10)
        phvwr.e        p.common.p4_intr_global_drop, 1
        nop

    .brcase        CNP_RING_ID
        // reset sched_eval_done 
        tblwr          d.ring_empty_sched_eval_done, 0
        
        tblwr          CNP_C_INDEX, CNP_P_INDEX
        add            r1, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqcb2_cnp_process, r1)
        nop.e
        nop

    .brcase        MAX_SQ_DOORBELL_RINGS

        crestore  [c2], d.poll_for_work, 0x1
        crestore  [c1], d.poll_in_progress, 0x1
        #c2 - poll_for_work
        #c1 - poll_in_progress

        setcf c3, [c7 & c2 & !c1]
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

    // Revert spec_cindex to next sq_cindex upon recirc of the current one
    add            r1, r0, SQ_C_INDEX // Branch Delay Slot
    mincr          r1, d.log_num_wqes, 1
    tblwr          SPEC_SQ_C_INDEX, r1

    seq            c1, CAPRI_APP_DATA_RECIRC_REASON, REQ_TX_RECIRC_REASON_SGE_WORK_PENDING
    bcf            [c1], process_sge_recirc
    phvwr          p.common.p4_intr_recirc, 0 // Branch Delay Slot
   
    seq            c1, CAPRI_APP_DATA_RECIRC_REASON, REQ_TX_RECIRC_REASON_BIND_MW
    bcf            [c1], process_bind_mw_recirc
    nop            // Branch Delay Slot

process_sge_recirc:
    // nothing to be done here, table 3 is programmed to execute req_tx_sqsge_process
    phvwr.e        CAPRI_PHV_FIELD(TO_S1_DCQCN_BIND_MW_P, header_template_addr_or_pd), d.pd

    nop

process_bind_mw_recirc:
    //CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_bind_mw_rkey_process, r0)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r0)
    nop.e
    nop
    
fence:

    // Reset spec-cindex to cindex to re-process wqe.
    tblwr       SPEC_SQ_C_INDEX, SQ_C_INDEX

    // Setup to-stage info.
    phvwr       CAPRI_PHV_FIELD(TO_S2_SQWQE_P, header_template_addr), d.header_template_addr 
    phvwrpair   CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, header_template_addr_or_pd), d.pd, \
                CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, spec_cindex), SPEC_SQ_C_INDEX
    phvwr       CAPRI_PHV_FIELD(TO_S1_FENCE_P, wqe_addr), d.curr_wqe_ptr
    phvwrpair   CAPRI_PHV_FIELD(TO_S5_SQCB_WB_P, wqe_addr), d.curr_wqe_ptr, \
                CAPRI_PHV_FIELD(TO_S5_SQCB_WB_P, spec_cindex), SPEC_SQ_C_INDEX                

    // Set up s2s info.
    CAPRI_RESET_TABLE_0_ARG()

    // remaining_payload_bytes = (1 << sqcb0_p->log_pmtu), to start with
    sll            r4, 1, d.log_pmtu

    phvwrpair   CAPRI_PHV_FIELD(SQCB_TO_WQE_P, log_pmtu), d.log_pmtu, \
                CAPRI_PHV_FIELD(SQCB_TO_WQE_P, poll_in_progress), d.poll_in_progress
    phvwrpair   CAPRI_PHV_FIELD(SQCB_TO_WQE_P, remaining_payload_bytes), r4, \
                CAPRI_PHV_FIELD(SQCB_TO_WQE_P, color), d.color
    phvwr       CAPRI_PHV_FIELD(SQCB_TO_WQE_P, current_sge_offset), d.read_req_adjust


    // Invoke sqcb1 to fetch rrq_cindex
    add         r2, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqcb2_fence_process, r2)
    tblmincri   SPEC_SQ_C_INDEX, d.log_num_wqes, 1 

end:
    nop.e
    nop

frpmr:
    // Reset spec_cindex and trigger dcqcn/sqcb_write_back/sqcb2_write_back to update cindex-copy everywhere.
    tblwr      SPEC_SQ_C_INDEX, SQ_C_INDEX

    // Its ok to reset the flag here. cindex-copy update is not expected to fail!
    tblwr      d.frpmr_in_progress, 0

    phvwr      CAPRI_PHV_FIELD(TO_S4_DCQCN_BIND_MW_P, spec_cindex), SPEC_SQ_C_INDEX
    phvwr      CAPRI_PHV_FIELD(TO_S5_SQCB_WB_P, spec_cindex), SPEC_SQ_C_INDEX

    // Below flags are required to increment cindex and load sqcb2_write_back in stage-5.
    phvwrpair  CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, last_pkt), 1 , CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, non_packet_wqe), 1
    tblmincri  SPEC_SQ_C_INDEX, d.log_num_wqes, 1
    // congestion_mgmt_enable flag and header_template_addr to-stage3 info is already copied earlier in the program.
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r0)

    //Load dummy-sqpt to load sqwqe in stage 2. wqe is checked out to know lkey to load in stage4 for state update.
    phvwr     CAPRI_PHV_FIELD(SQCB_TO_WQE_P, frpmr_lkey_state_upd), 1
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dummy_sqpt_process, d.curr_wqe_ptr)

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

check_state:
    // If in QP_STATE_SQD or QP_STATE_SQD_ON_ERR, start procesing
    // sq until the drain cindex
    slt           c7, d.state, QP_STATE_SQD
    bcf           [!c7], process_sq_drain

    // One of Init, Reset, RTR, ERR or SQ_ERR states. So disable scheduler
    // bit until modify_qp updates the state to RTS
    DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(CAPRI_TXDMA_INTRINSIC_LIF, \
                                         CAPRI_TXDMA_INTRINSIC_QTYPE, \
                                         CAPRI_TXDMA_INTRINSIC_QID, \
                                         SQ_RING_ID, r1, r2)

    // On transitioning to QP_STATE_ERR, if flush_rq is set, send flush
    // feedback msg to RQ so that local resp_rx can transition RQ to
    // QP_STATE_ERR and post flush err to its CQ
    seq           c1, d.state, QP_STATE_ERR
    seq           c2, d.flush_rq, 1 
    bcf           [!c1 | !c2], exit
flush_rq:
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_ERR_FEEDBACK) // Branch Delay Slot
    add            r1, r0, offsetof(struct req_tx_phv_t, p4_to_p4plus)
    phvwrp         r1, 0, CAPRI_SIZEOF_RANGE(struct req_tx_phv_t, p4_intr_global, p4_to_p4plus), r0
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, p4_intr_global, p4_to_p4plus, 2)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, rdma_feedback, rdma_feedback, 1)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)

    phvwrpair      p.p4_intr_global.tm_iport, TM_PORT_INGRESS, p.p4_intr_global.tm_oport, TM_PORT_DMA
    phvwrpair      p.p4_intr_global.tm_iq, 0, p.p4_intr_global.lif, CAPRI_TXDMA_INTRINSIC_LIF
    RQCB0_ADDR_GET(r1)
//    phvwrpair      p.p4_intr_rxdma.intr_qid, CAPRI_TXDMA_INTRINSIC_QID, p.p4_intr_rxdma.intr_qstate_addr, r1
    phvwr          p.p4_intr_rxdma.intr_qid, CAPRI_TXDMA_INTRINSIC_QID
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET

    phvwrpair      p.p4_intr_rxdma.intr_qtype, Q_TYPE_RDMA_RQ, p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, RESP_RX_FLAG_ERR_DIS_QP
    phvwri         p.p4_to_p4plus.table0_valid, 1

    phvwrpair.e    p.rdma_feedback.feedback_type, RDMA_COMPLETION_FEEDBACK, \
                   p.rdma_feedback.completion.status, CQ_STATUS_WQE_FLUSHED_ERR

    // unset flush_rq bit so that flush feedback msg is sent only once upon
    // transitioning to QP_STATE_ERR
    tblwr          d.flush_rq, 0
 

process_sq_drain:
    // Flush all wqes in SQ ring below drain cindex in SQD or SQD_ON_ERR state.
    // If spec_cindex is above drain cindex but its not SQ ring event, then
    // process the event otherwise cease SQ processing. Eventually upon
    // receiving acks for all the drained wqes an asynchronous event
    // will be raised to notify driver about drain completion.
    sle            c7, SPEC_SQ_C_INDEX, d.sqd_cindex
    seq            c2, d.in_progress, 1
    seq            c3, d.fence, 1
    seq            c4, d.frpmr_in_progress, 1
    sne            c5, r7[MAX_SQ_DOORBELL_RINGS:1], 0
    bcf            [c7 | c2 | c3 | c4 | c5], process_req_tx
    nop            // Branch Delay Slot

    // Upon draining SQ, send feedback to RxDMA to denote completion of
    // SQ drain in TxDMA which includes all packet and non-packet generating
    // wqes. RxDMA on reeciving this feedback will mark completion of
    // non-packet generating wqes and wait for pending responses for all packet
    // generating wqes. Once all responses are received an async event is raised
    // to denote sq drain completion. If feedback was already sent, which could
    // happen if there was backtrack for packet generating wqes or if a new
    // wqe is posted by user, then skip sending feedback msg again as RxDMA
    // has already recorded the fact in sqcb1 that non-packet gernating wqes
    // are drained in TxDMA
    bbeq           d.sq_drained, 1, drop

    seq            c2, d.dcqcn_rl_failure, 1 // Branch Delay Slot
    tblwr.c2       SPEC_SQ_C_INDEX, SQ_C_INDEX
    tblwr.c2       d.dcqcn_rl_failure, 0

    seq            c2, SPEC_SQ_C_INDEX, SQ_C_INDEX // Branch Delay Slot
    bcf            [!c2], drop
    nop            // Branch Delay Slot

drain_feedback:
    tblwr          d.sq_drained, 1
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_sq_drain_feedback_process, r0)

drop:
#ifndef HAPS
    // Disable schceduler bit for SQ in model run to prevent model from
    // getting into infinite loop.

    DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(CAPRI_TXDMA_INTRINSIC_LIF, \
                                         CAPRI_TXDMA_INTRINSIC_QTYPE, \
                                         CAPRI_TXDMA_INTRINSIC_QID, \
                                         SQ_RING_ID, r1, r2)
#endif
    phvwr.e        p.common.p4_intr_global_drop, 1
    nop

