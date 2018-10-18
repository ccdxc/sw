#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s5_t2_k k;
struct sqcb2_t d;

#define TO_S7_STATS_P         to_s7_stats_info

%%
    .param req_tx_stats_process

.align
req_tx_timer_expiry_process:
    // Pin timer_expiry to stage 5, same as add_headers_process so that
    // sqcb2 can be locked and timer_on field can be updated depending
    // on the timer is being restarted or not
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_5
    bcf            [!c1], bubble_to_next_stage

    // If no response is expected then drop this spurious timer expiry event
    // Also unset timer_on and not restart timer. Timer will be restarted
    // upon transmission of next new request.
    scwle24        c1, d.rexmit_psn, d.exp_rsp_psn // Branch Delay Slot
    bcf            [!c1], spurious_expiry
    // Check if at this expiry event retransmit time has reached
    slt            c1, r4, d.last_ack_or_req_ts // Branch Delay Slot
    sub.c1         r1, d.last_ack_or_req_ts, r4
    sub.!c1        r1, r4, d.last_ack_or_req_ts

check_rnr_timeout:
    seq            c1, d.rnr_timeout, 0
    bcf            [c1], check_local_ack_timeout

rnr_timeout:
    sll            r2, 1, d.rnr_timeout // Branch Delay Slot
    // Ignore expiry event if retransmit time has not reached
    // TODO comment out this check for now as in model cur_timestamp
    // is not populated in r4
#ifdef HAPS
    blt            r1, r2, restart_timer
#endif
 
    // Infinite retries if retry_ctr is set to 7
    seq            c1, d.rnr_retry_ctr, 7 // Branch Delay Slot
    tblsub.!c1     d.rnr_retry_ctr, 1

    // Check rnr_retry_ctr for RNR
    seq            c1, d.rnr_retry_ctr, 0 // Branch Delay Slot
    bcf            [c1], err_completion
    phvwrpair.!c1  CAPRI_PHV_FIELD(TO_S7_STATS_P, timeout), 1, \
                   CAPRI_PHV_FIELD(TO_S7_STATS_P, timeout_rnr), 1 //BD Slot

    b              process_expiry
 
check_local_ack_timeout:
    seq            c1, d.local_ack_timeout, 0 //BD Slot
    bcf            [c1], spurious_expiry

local_ack_timeout:
    // 4.096 * (2 ^ local_ack_timeout) usec  = 1 << (12 + local_ack_timeout) nsec
    // Minimum local_ack_timeout that can be supported in capri is 2 (> 10usec)
    // local_ack_timeout in sqcb2 is programmed to include multiplication factor
    // of 4096 as well
    sll            r2, 4096, d.local_ack_timeout
    // Ignore expiry event if retransmit time has not reached
    // TODO comment out this check for now as in model cur_timestamp
    // is not populated in r4
#ifdef HAPS
    blt            r1, r2, restart_timer
#endif

    // Infinite retries if retry_ctr is set to 7
    seq            c1, d.err_retry_ctr, 7 // Branch Delay Slot
    tblsub.!c1     d.err_retry_ctr, 1

    // Check err_retry_ctr for retransmit timeout
    seq            c1, d.err_retry_ctr, 0 // Branch Delay Slot
    bcf            [c1], err_completion
    phvwrpair.!c1  CAPRI_PHV_FIELD(TO_S7_STATS_P, timeout), 1, \
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
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET
        
    phvwrpair      p.p4_intr_rxdma.intr_qtype, K_GLOBAL_QTYPE, p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, REQ_RX_FLAG_RDMA_FEEDBACK
    phvwri         p.p4_to_p4plus.table0_valid, 1

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
    CAPRI_START_SLOW_TIMER(r1, r2, K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, TIMER_RING_ID, 10)
    //phvwr.e   p.common.p4_intr_global_drop, 1
    nop.e
    nop

err_completion:
    // TODO post err completion
    phvwr.e        p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_3_VALID(0)
    
spurious_expiry:
    tblwr          d.timer_on, 0
    phvwr.e        p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_3_VALID(0)

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_4   
    bcf           [!c1], exit
    SQCB2_ADDR_GET(r1) // Branch Delay Slot

    // Invoke timer_expiry in the same stage & table as add_headers_process so
    // that timer_on field can be updated in a locked manner between these 
    // programs
    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

exit:
    nop.e
    nop
