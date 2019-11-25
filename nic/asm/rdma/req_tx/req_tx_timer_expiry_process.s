#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s5_t2_k k;
struct sqcb2_t d;

#define TO_S7_STATS_P         to_s7_stats_info

%%
    .param rdma_num_clock_ticks_per_us
    .param req_tx_stats_process
    .param req_tx_write_back_process

.align
req_tx_timer_expiry_process:
    // Pin timer_expiry to stage 5, same as add_headers_process so that
    // sqcb2 can be locked and timer_on field can be updated depending
    // on the timer is being restarted or not
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_5
    bcf            [!c1], bubble_to_next_stage

    // Check if at this expiry event retransmit time has reached
    slt            c1, r4, d.last_ack_or_req_ts // Branch Delay Slot
    // Handle rollover by adding max-timer-ticks to cur-timestamp. 
    // Capri supports 48-bit timestamp. Add ((1 << 48) - 1) to timestamp.
    add.c1         r4, r4, 1, 48
    subi.c1        r4, r4, 1
    sub            r1, r4, d.last_ack_or_req_ts

check_rnr_timeout:
    seq            c1, d.rnr_timeout, 0
    bcf            [c1], check_local_ack_timeout

rnr_timeout:
    DECODE_RNR_SYNDROME_TIMEOUT(r2, d.rnr_timeout, r3, c2) // Branch Delay Slot
    mul            r2, r2, 10
    mul            r2, r2, rdma_num_clock_ticks_per_us
    // Ignore expiry event if retransmit time has not reached
    // comment out this check for model as cur_timestamp
    // is not populated in r4
#if defined (HAPS) || defined (HW)
    slt            c3, r1, r2
    bcf            [c3], restart_timer
#endif
 
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_STATS_P, timeout), 1, \
                   CAPRI_PHV_FIELD(TO_S7_STATS_P, timeout_rnr), 1 //BD Slot

    b              process_expiry
    nop
check_local_ack_timeout:
    seq            c1, d.local_ack_timeout, 0
    bcf            [c1], spurious_expiry

local_ack_timeout:

    // If no response is expected then drop this spurious timer expiry event
    // Also unset timer_on and not restart timer. Timer will be restarted
    // upon transmission of next new request.
    scwle24        c1, d.rexmit_psn, d.exp_rsp_psn // Branch Delay Slot
    bcf            [!c1], spurious_expiry

    /*
     * 4.096 * (2 ^ local_ack_timeout) usec.
     * Rounded-off to 4 * (2 ^ local_ack_timeout) usec since floating-point is not supported.
     * Spec says - "The timeout condition should be detected in no less than the timeout
     * interval, Tr, and no more than four times the timeout interval, 4Tr."
     * Because of round-off, timer can expire before Tr, hence bumping it up by a factor of 2.
     * So actual timeout interval will be 8 * (2 ^ local_ack_timeout) usec.
     */

    sll            r2, 8, d.local_ack_timeout //BD-slot
    mul            r2, r2, rdma_num_clock_ticks_per_us

    // Ignore expiry event if retransmit time has not reached
    // comment out this check for model as cur_timestamp
    // is not populated in r4
  
#if defined (HAPS) || defined (HW)
    slt            c3, r1, r2
    bcf            [c3], restart_timer
#endif
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_STATS_P, timeout), 1, \
                   CAPRI_PHV_FIELD(TO_S7_STATS_P, timeout_local_ack), 1 //BD Slot
process_expiry:
    // send timer_expiry feedback msg to RxDMA so that RxDMA can
    // set qstate to bktrack_in_progress state and drop any 
    // subsequent acks from modifying the qstate while bktrack logic
    // is in progress.
    add            r1, r0, offsetof(struct req_tx_phv_t, p4_to_p4plus)
    phvwrp         r1, 0, CAPRI_SIZEOF_RANGE(struct req_tx_phv_t, p4_intr_global, p4_to_p4plus), r0
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_FEEDBACK) // Branch Delay Slot
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, p4_intr_global, p4_to_p4plus, 2)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, rdma_feedback, rdma_feedback, 1)
        
    phvwrpair      p.p4_intr_global.tm_iport, TM_PORT_INGRESS, p.p4_intr_global.tm_oport, TM_PORT_DMA
    phvwrpair      p.p4_intr_global.tm_iq, 0, p.p4_intr_global.lif, K_GLOBAL_LIF
    SQCB0_ADDR_GET(r1)
    phvwrpair      p.p4_intr_rxdma.intr_qid, K_GLOBAL_QID, p.p4_intr_rxdma.intr_qstate_addr, r1
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_REQ_FEEDBACK_SPLITTER_OFFSET
        
    phvwrpair      p.p4_intr_rxdma.intr_qtype, K_GLOBAL_QTYPE, p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, REQ_RX_FLAG_RDMA_FEEDBACK
    phvwri         p.{p4_to_p4plus.table0_valid...p4_to_p4plus.table1_valid}, 0x3

    // Fill timer_expiry feedback msg with txdma version of rexmit_psn. Compare
    // this with rexmit_psn on the rxdma side and post bktrack if its the same.
    // otherwise, ack was recived and request corresponding to that rexmit_psn
    // is not required to be retransmitted
    phvwrpair      p.rdma_feedback.feedback_type, RDMA_TIMER_EXPIRY_FEEDBACK, \
                   p.rdma_feedback.timer_expiry.rexmit_psn, d.rexmit_psn
    phvwrpair      p.rdma_feedback.timer_expiry.ssn, d.ssn, \
                   p.rdma_feedback.timer_expiry.tx_psn, d.tx_psn

    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)
  
    //invoke stats_process for Timeout case here
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_stats_process, r0)

restart_timer:
    CAPRI_START_FAST_TIMER(r1, r2, K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, TIMER_RING_ID, 100)
    CAPRI_SET_TABLE_2_VALID_CE(c0, 0)
    phvwr.c3   p.common.p4_intr_global_drop, 1 // Exit Slot

spurious_expiry:
    tblwr          d.timer_on, 0
    phvwr.e        p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_4   
    bcf           [!c1], exit
    SQCB2_ADDR_GET(r1) // Branch Delay Slot

    // Invoke timer_expiry in the same stage & table as add_headers_process so
    // that timer_on field can be updated in a locked manner between these 
    // programs
    CAPRI_GET_TABLE_2_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

exit:
    nop.e
    nop
