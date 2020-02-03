#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb0_t d;
struct rdma_stage0_table_k k;

#define RQCB_TO_RQCB2_P t0_s2s_rqcb_to_rqcb2_info
#define TO_S1_P to_s1_prefetch_info
#define TO_S3_P to_s3_dcqcn_info
#define TO_S4_P to_s4_dcqcn_info
#define TO_S4_RATE_TIMER_P to_s4_dcqcn_rate_timer_info
#define TO_S5_P to_s5_rqcb1_wb_info
#define TO_S6_P to_s6_rqpt_info
#define BT_TO_S_INFO_P to_s1_bt_info
#define RQCB_TO_DCQCN_CFG_P t1_s2s_dcqcn_config_info
#define RQCB_TO_PREFETCH_P t2_s2s_prefetch_info

#define RSQWQE_P            r1
#define RQCB2_P             r2
#define NEW_RSQ_C_INDEX     r5
#define DCQCNCB_ADDR        r6

%%
    .param      resp_tx_rqcb2_process
    .param      resp_tx_rqcb2_bt_process
    .param      resp_tx_ack_process
    .param      resp_tx_dcqcn_rate_process
    .param      resp_tx_dcqcn_timer_process
    .param      resp_tx_dcqcn_config_load_process
    .param      resp_tx_bt_mpu_only_process
    .param      resp_tx_setup_checkout_process

resp_tx_rqcb_process:

    bcf             [c2 | c3 | c7], table_error
    // are we in a state to process posted buffers?
    slt             c1, d.state, QP_STATE_INIT // BD Slot

    seq             c2, d.state, QP_STATE_ERR
    bcf             [c1 | c2], state_fail
    seq             c3, d.prefetch_en, 1 // BD Slot
    // c3: prefetch_en

    seq             c4, RQ_C_INDEX, RQ_P_INDEX
    // memwr timestamp into rqcb3 only if pindex != cindex
    bcf             [c3 | c4], timestamp_memwr_done
    add             r2, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (3*CB_UNIT_SIZE_BYTES) // BD Slot
    add             r2, r2, FIELD_OFFSET(rqcb3_t, resp_tx_timestamp)
    memwr.dx        r2, r4
    // memwr proxy_pindex if there are buffers posted, irrespective of which ring is scheduled
    add             r2, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES // BD Slot
    add             r2, r2, FIELD_OFFSET(rqcb1_t, proxy_pindex)
    memwr.hx        r2, RQ_P_INDEX

timestamp_memwr_done:
    .brbegin
    brpri           r7[MAX_RQ_RINGS-1:0], [DCQCN_TIMER_PRI, DCQCN_RATE_COMPUTE_PRI, BT_PRI, ACK_NAK_PRI, RSQ_PRI, RQ_PRI]
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

        // if prefetch is enabled, skip memwr of proxy pindex
        // in prefetch case, proxy pindex will be written using DMA
        // after WQE's are actually prefetched
        bcf             [c3], skip_memwr
        add             r2, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES // BD Slot
        add             r2, r2, FIELD_OFFSET(rqcb1_t, proxy_pindex)
        // we do not need to load RQCB1 to populate this value.
        // There is always a possibility of RxDMA seeing this
        // change in a delayed manner, irrespective of whether we
        // update using tblwr, memwr or DMA.
        memwr.hx        r2, RQ_P_INDEX
        phvwr.e         p.common.p4_intr_global_drop, 1
        nop // Exit Slot

skip_memwr:
        CAPRI_SET_FIELD2(TO_S6_P, invoke_stats, 1)

        seq             c2, d.serv_type, RDMA_SERV_TYPE_UD
        bcf             [!c2], prefetch
        CAPRI_SET_FIELD2(RQCB_TO_PREFETCH_P, cmd_eop, 1) // BD Slot

        phvwr.e         p.common.p4_intr_global_drop, 1
        nop // Exit Slot

    .brcase         RSQ_RING_ID
        // check if spec_color == curr_color
        bbeq            d.spec_color, d.curr_color, rsq_process
        // reset sched_eval_done
        tblwr           d.ring_empty_sched_eval_done, 0     // BD Slot

        tblmincri       d.spec_color, 1, 1
        tblwr           d.spec_read_rsp_psn, d.curr_read_rsp_psn
        phvwr           CAPRI_PHV_FIELD(TO_S4_P, resp_rl_failure), 1

rsq_process:
        add             DCQCNCB_ADDR, AH_ENTRY_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT    //BD Slot

        // Pass congestion_mgmt_type flag to stages 3 and 4.
        // congestion_mgmt_type: 0 means congestion mgmt is not enabled.
        sne             c1, d.congestion_mgmt_type, 0 //BD-Slot
        phvwrpair.c1    CAPRI_PHV_FIELD(TO_S3_P, dcqcn_cb_addr), DCQCNCB_ADDR, \
                        CAPRI_PHV_FIELD(TO_S3_P, congestion_mgmt_type), d.congestion_mgmt_type
        // Pass congestion_mgmt_type flag to stages 4.
        // the first bit of congestion_mgmt_type indicates DCQCN or ROME for now
        CAPRI_SET_FIELD2_C(TO_S4_P, congestion_mgmt_type, d.congestion_mgmt_type, c1)

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
        phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_RQCB2_P, rsqwqe_addr), RSQWQE_P, \
                    CAPRI_PHV_FIELD(RQCB_TO_RQCB2_P, curr_read_rsp_psn), d.spec_read_rsp_psn

        phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_RQCB2_P, log_pmtu), d.log_pmtu, \
                    CAPRI_PHV_FIELD(RQCB_TO_RQCB2_P, serv_type), d.serv_type

        phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_RQCB2_P, header_template_addr), d.header_template_addr, \
                    CAPRI_PHV_FIELD(RQCB_TO_RQCB2_P, header_template_size), d.header_template_size

        // incr spec_psn 
        tblmincri       d.spec_read_rsp_psn, 24, 1

        add             RQCB2_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)
        CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb2_process, RQCB2_P)

    .brcase         ACK_NAK_RING_ID
        // reset sched_eval_done
        tblwr           d.ring_empty_sched_eval_done, 0

        // overwrite ACK_NAK_C_INDEX with ACK_NAK_P_INDEX
        tblwr           ACK_NAK_C_INDEX, ACK_NAK_P_INDEX

        add             DCQCNCB_ADDR, AH_ENTRY_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT    //BD Slot

        // Pass congestion_mgmt_type flag to stages 3 and 4.
        // congestion_mgmt_type: 0 means congestion mgmt is not enabled.
        sne             c1, d.congestion_mgmt_type, 0 //BD-Slot
        phvwrpair.c1    CAPRI_PHV_FIELD(TO_S3_P, dcqcn_cb_addr), DCQCNCB_ADDR, \
                        CAPRI_PHV_FIELD(TO_S3_P, congestion_mgmt_type), d.congestion_mgmt_type
        CAPRI_SET_FIELD2_C(TO_S4_P, congestion_mgmt_type, d.congestion_mgmt_type, c1)

        // send serv_type and ack processing flag to stage 5 (writeback)
        //TBD: can we move setting ack_nak_process bit to ack_process program ?
        //now that writeback loads RQCB0, we can avoid passing serv_type
        CAPRI_SET_FIELD2(TO_S5_P, ack_nak_process, 1)

        add             RQCB2_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_ack_process, RQCB2_P)
    
        seq             c2, d.serv_type, RDMA_SERV_TYPE_UD
        bcf             [!c2 & c3], prefetch
        nop // BD Slot

        nop.e
        nop // Exit Slot

    .brcase         BT_RING_ID
        // reset sched_eval_done
        tblwr           d.ring_empty_sched_eval_done, 0

        // if we are already backtracking a request, ignore further scheduling opportunities
        bbeq            d.bt_lock, 1, exit
        add             RQCB2_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)    //BD Slot

        bbeq            d.bt_in_progress, 1, skip_drain
        // since there is no read_rsp_lock, we need to make sure
        // that BT is processed after the PHV's that are already in the pipeline
        seq             c7, d.drain_in_progress, 1 // BD Slot

        bcf             [!c7], start_drain
        seq             c6, d.drain_done, 1 // BD Slot

        bcf             [!c6], exit
        setcf           c5, [c7 & c6] // BD Slot
        tblwr.c5        d.drain_in_progress, 0
        tblwr.c5        d.drain_done, 0

skip_drain:
        // take the lock such that further scheduling opportunities are ignored
        tblwr           d.bt_lock, 1

        // check if spec_color == curr_color
        seq             c7, d.spec_color, d.curr_color
        tblmincri.!c7   d.spec_color, 1, 1
        tblwr.!c7       d.spec_read_rsp_psn, d.curr_read_rsp_psn

        add             r6, r0, RSQ_C_INDEX

        seq             c1, RSQ_P_INDEX, RSQ_C_INDEX

        // if RSQ is empty, we need to start from cindex-1
        mincr.c1        r6, d.log_rsq_size, -1

        // overwrite the rsqwqe cindex from cb state if bt is in progress
        seq             c2, d.bt_in_progress, 1
        add.c2          r6, r0, d.bt_rsq_cindex

bt_in_progress:

        CAPRI_SET_FIELD2(BT_TO_S_INFO_P, log_rsq_size, d.log_rsq_size)
        CAPRI_SET_FIELD2(BT_TO_S_INFO_P, log_pmtu, d.log_pmtu)
        CAPRI_SET_FIELD2(BT_TO_S_INFO_P, rsq_base_addr, d.rsq_base_addr)
        CAPRI_SET_FIELD2(BT_TO_S_INFO_P, bt_cindex, BT_P_INDEX)
        // we want to end the search when we reach here
        CAPRI_SET_FIELD2(BT_TO_S_INFO_P, end_index, RSQ_P_INDEX)
        // we want to start search from here
        CAPRI_SET_FIELD2(BT_TO_S_INFO_P, search_index, r6)

        CAPRI_SET_FIELD2(BT_TO_S_INFO_P, curr_read_rsp_psn, d.spec_read_rsp_psn)
        //CAPRI_SET_FIELD2(BT_TO_S_INFO_P, read_rsp_in_progress, d.read_rsp_in_progress)
        CAPRI_SET_FIELD2(BT_TO_S_INFO_P, bt_in_progress, d.bt_in_progress)

        //load rqcb2 to get psn to backtrack to (which is copied by rxdma)
        CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb2_bt_process, RQCB2_P)


    .brcase         DCQCN_RATE_COMPUTE_RING_ID
        // reset sched_eval_done
        tblwr           d.ring_empty_sched_eval_done, 0

        // Increment c-index for rate-compute-ring.
        tblmincri       DCQCN_RATE_COMPUTE_C_INDEX, 16, 1

        phvwrpair       CAPRI_PHV_FIELD(RQCB_TO_DCQCN_CFG_P, dcqcn_config_id), d.dcqcn_cfg_id, CAPRI_PHV_FIELD(RQCB_TO_DCQCN_CFG_P, dcqcn_rate_timer_toggle), 1

        // Increment timer-c-index and pass to stage 4. This is used to stop dcqcn-timer on reaching max-qp-rate.
        add             r3, r0, DCQCN_TIMER_C_INDEX
        mincr           r3, 16, 1 // c_index is 16 bit
        CAPRI_SET_FIELD2(TO_S4_RATE_TIMER_P, new_timer_cindex, r3)

        add             DCQCNCB_ADDR, AH_ENTRY_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_rate_process, DCQCNCB_ADDR)
        CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_config_load_process, r0)

        phvwr.e         p.common.p4_intr_global_drop, 1
        nop // Exit Slot

    .brcase         DCQCN_TIMER_RING_ID
        // reset sched_eval_done
        tblwr          d.ring_empty_sched_eval_done, 0
        // Increment c-index of timer-ring.
        tblmincri       DCQCN_TIMER_C_INDEX, 16, 1

        phvwrpair       CAPRI_PHV_FIELD(RQCB_TO_DCQCN_CFG_P, dcqcn_config_id), d.dcqcn_cfg_id, CAPRI_PHV_FIELD(RQCB_TO_DCQCN_CFG_P, dcqcn_rate_timer_toggle), r0
        add             DCQCNCB_ADDR, AH_ENTRY_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
        CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_dcqcn_timer_process, DCQCNCB_ADDR)
        CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_config_load_process, r0)

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

prefetch:
    add             RQCB2_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)

    phvwrpair       CAPRI_PHV_FIELD(RQCB_TO_PREFETCH_P, log_rq_page_size), d.log_rq_page_size, \
                    CAPRI_PHV_FIELD(RQCB_TO_PREFETCH_P, log_wqe_size), d.log_wqe_size

    add             r1, r0, RQ_P_INDEX
    phvwrpair       CAPRI_PHV_FIELD(RQCB_TO_PREFETCH_P, pt_base_addr), d.pt_base_addr, \
                    CAPRI_PHV_FIELD(RQCB_TO_PREFETCH_P, rq_pindex), r1

    CAPRI_SET_FIELD2(RQCB_TO_PREFETCH_P, log_num_wqes, d.log_num_wqes)

    // load setup_checkout_process
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_setup_checkout_process, RQCB2_P) // Exit Slot
   
start_drain:
    tblwr       d.drain_in_progress, 1 
    // load an mpu only program as marker phv
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_bt_mpu_only_process, r0)

exit:
    phvwr.e     p.common.p4_intr_global_drop, 1
    nop

state_fail:
    // Disable scheduler bit until modify_qp updates the state to RTR
    DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(CAPRI_TXDMA_INTRINSIC_LIF, \
                                         CAPRI_TXDMA_INTRINSIC_QTYPE, \
                                         CAPRI_TXDMA_INTRINSIC_QID, \
                                         RQ_RING_ID, r1, r2)
    phvwr.e     p.common.p4_intr_global_drop, 1
    nop         //Exit Slot

table_error:
    // TODO add LIF stats
    phvwr.e        p.common.p4_intr_global_drop, 1
    nop // Exit Slot
