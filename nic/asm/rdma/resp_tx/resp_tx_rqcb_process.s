#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb0_t d;
struct rdma_stage0_table_k k;

#define RQCB_TO_RQCB2_P t0_s2s_rqcb_to_rqcb2_info
#define TO_S3_P to_s3_dcqcn_info
#define TO_S4_P to_s4_dcqcn_info
#define TO_S5_P to_s5_rqcb1_wb_info

#define RSQWQE_P            r1
#define RQCB2_P             r2
#define NEW_RSQ_C_INDEX     r5
#define DCQCNCB_ADDR        r6

%%
    .param      resp_tx_rqcb2_process
    .param      resp_tx_ack_process
    .param      resp_tx_dcqcn_rate_process
    .param      resp_tx_dcqcn_timer_process

resp_tx_rqcb_process:

    .brbegin
    brpri           r7[MAX_RQ_RINGS-1:0], [DCQCN_TIMER_PRI, DCQCN_RATE_COMPUTE_PRI, RSQ_BT_PRI, ACK_NAK_PRI, RSQ_PRI, RQ_PRI]
    nop

    .brcase         RQ_RING_ID
        // reset sched_eval_done
        tblwr           d.ring_empty_sched_eval_done, 0

        //to stop scheduler ringing for ever, artificially move c_index to p_index. 
        //take copy of pindex into proxy_pindex, so that 
        //resp_rx can use this variable from rqcb1 to compare
        //against proxy_cindex for queue full/empty conditions
        //proxy_c_index would track the real c_index
    
        tblwr           RQ_C_INDEX, RQ_P_INDEX
        add             r2, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES
        add             r2, r2, FIELD_OFFSET(rqcb1_t, proxy_pindex)
        // we do not need to load RQCB1 to populate this value.
        // we can use memwr
        // There is always a possibility of RxDMA seeing this
        // change in a delayed manner, irrespective of whether we 
        // update using tblwr, memwr or DMA.
        memwr.h         r2, RQ_P_INDEX
        phvwr.e         p.common.p4_intr_global_drop, 1
        nop             //Exit Slot

    .brcase         RSQ_RING_ID
        // reset sched_eval_done
        tblwr           d.ring_empty_sched_eval_done, 0

        bbeq            d.read_rsp_lock, 1, exit    
        add             DCQCNCB_ADDR, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT    //BD Slot

        // Pass congestion_mgmt_enable flag to stages 3 and 4.
        CAPRI_SET_FIELD2(TO_S3_P, congestion_mgmt_enable, d.congestion_mgmt_enable)
        CAPRI_SET_FIELD2(TO_S3_P, dcqcn_cb_addr, DCQCNCB_ADDR)

        CAPRI_SET_FIELD2(TO_S4_P, congestion_mgmt_enable, d.congestion_mgmt_enable)

        tblwr           d.read_rsp_lock, 1
        sll             RSQWQE_P, d.rsq_base_addr, RSQ_BASE_ADDR_SHIFT
        add             RSQWQE_P, RSQWQE_P, RSQ_C_INDEX, LOG_SIZEOF_RSQWQE_T

        add             NEW_RSQ_C_INDEX, r0, RSQ_C_INDEX
        mincr           NEW_RSQ_C_INDEX, d.log_rsq_size, 1

        // TBD: can we move this to write back ?
        phvwr           p.bth.dst_qp, d.dst_qp

        // send NEW_RSQ_C_INDEX to stage 5 (writeback)
        CAPRI_SET_FIELD2(TO_S5_P, new_c_index, NEW_RSQ_C_INDEX)

        //TBD: we can avoid passing serv_type ?
        CAPRI_RESET_TABLE_0_ARG()
        CAPRI_SET_FIELD2(RQCB_TO_RQCB2_P, curr_read_rsp_psn, d.curr_read_rsp_psn)
        CAPRI_SET_FIELD2(RQCB_TO_RQCB2_P, rsqwqe_addr, RSQWQE_P)
        CAPRI_SET_FIELD2(RQCB_TO_RQCB2_P, log_pmtu, d.log_pmtu)
        CAPRI_SET_FIELD2(RQCB_TO_RQCB2_P, serv_type, d.serv_type)
        CAPRI_SET_FIELD2(RQCB_TO_RQCB2_P, header_template_addr, d.header_template_addr)
        CAPRI_SET_FIELD2(RQCB_TO_RQCB2_P, header_template_size, d.header_template_size)
        CAPRI_SET_FIELD2(RQCB_TO_RQCB2_P, read_rsp_in_progress, d.read_rsp_in_progress)
        

        add             RQCB2_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb2_process, RQCB2_P)

        nop.e
        nop

    .brcase         ACK_NAK_RING_ID
        // reset sched_eval_done
        tblwr           d.ring_empty_sched_eval_done, 0

        // overwrite ACK_NAK_C_INDEX with ACK_NAK_P_INDEX
        tblwr           ACK_NAK_C_INDEX, ACK_NAK_P_INDEX

        add             DCQCNCB_ADDR, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT    //BD Slot

        // Pass congestion_mgmt_enable flag to stages 3 and 4.
        CAPRI_SET_FIELD2(TO_S3_P, congestion_mgmt_enable, d.congestion_mgmt_enable)
        CAPRI_SET_FIELD2(TO_S3_P, dcqcn_cb_addr, DCQCNCB_ADDR)

        CAPRI_SET_FIELD2(TO_S4_P, congestion_mgmt_enable, d.congestion_mgmt_enable)

        // send serv_type and ack processing flag to stage 5 (writeback)
        //TBD: can we move setting ack_nak_process bit to ack_process program ?
        //now that writeback loads RQCB0, we can avoid passing serv_type
        CAPRI_SET_FIELD2(TO_S5_P, ack_nak_process, 1)

        add             RQCB2_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_ack_process, RQCB2_P)
    
        nop.e
        nop

    .brcase         RSQ_BT_RING_ID
        // reset sched_eval_done
        tblwr           d.ring_empty_sched_eval_done, 0

        // TBD : migrate to new approach
        b               exit
        nop             //BD Slot

    .brcase         DCQCN_RATE_COMPUTE_RING_ID
        // reset sched_eval_done
        tblwr           d.ring_empty_sched_eval_done, 0

        // Increment c-index for rate-compute-ring.
        tblmincri       DCQCN_RATE_COMPUTE_C_INDEX, 16, 1


        // Increment timer-c-index and pass to stage 4. This is used to stop dcqcn-timer on reaching max-qp-rate.
        add             r3, r0, DCQCN_TIMER_C_INDEX
        mincr           r3, 16, 1 // c_index is 16 bit
        CAPRI_SET_FIELD2(TO_S4_P, new_timer_cindex, r3)

        add             DCQCNCB_ADDR, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_rate_process, DCQCNCB_ADDR)
    
        phvwr.e     p.common.p4_intr_global_drop, 1
        nop //Exit Slot

    .brcase         DCQCN_TIMER_RING_ID
        // reset sched_eval_done
        tblwr          d.ring_empty_sched_eval_done, 0
        // Increment c-index of timer-ring.
        tblmincri       DCQCN_TIMER_C_INDEX, 16, 1

        add             DCQCNCB_ADDR, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_dcqcn_timer_process, DCQCNCB_ADDR)

        phvwr.e     p.common.p4_intr_global_drop, 1
        nop //Exit Slot

    .brcase         MAX_RQ_RINGS
    // all rings empty case
        bbeq        d.ring_empty_sched_eval_done, 1, exit
        nop         //BD Slot

        // ring doorbell to re-evaluate scheduler
        DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r2, r3)
        tblwr       d.ring_empty_sched_eval_done, 1

        phvwr.e     p.common.p4_intr_global_drop, 1
        nop         //Exit Slot

    .brend
    
exit:
    phvwr.e     p.common.p4_intr_global_drop, 1
    nop

//check_backtrack_q:
//    seq         c1, RSQ_BT_C_INDEX, RSQ_BT_P_INDEX
//    seq         c2, d.adjust_rsq_c_index_in_progress, 0
//
//    // if backtrack ring is invoked, but adjust_rsq_c_index_in_progress is not set
//    // (which is set in RXDMA), then ignore backtrack ring and check for other work
//    // It is possible that backtrack ring was already invoked which set
//    // adjust_rsq_c_index_in_progress to 0, but might not have made backtrack_c_index     // equal to backtrack_p_index yet (due to DMA delays) and scheduler gave one
//    // more opportunity. 
//    bcf         [c1 | c2], check_rq
//    tblwr       d.adjust_rsq_c_index_in_progress, 0 //BD Slot
//
//backtrack_q:
//    // reset all the in_progress variables
//    tblwr       d.read_rsp_in_progress, 0
//    tblwr       d.curr_read_rsp_psn, 0
//    tblwr       d.read_rsp_lock, 0
//
//    add         RQCB1_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES
//    CAPRI_SET_FIELD(r4, BT_ADJUST_INFO_T, adjust_rsq_c_index, d.adjust_rsq_c_index)
//    CAPRI_SET_FIELD(r4, BT_ADJUST_INFO_T, rsq_bt_p_index, RSQ_BT_P_INDEX)
//
//    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rsq_backtrack_adjust_process, RQCB1_P)
//
//    nop.e
//    nop
