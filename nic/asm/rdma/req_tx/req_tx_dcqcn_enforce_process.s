#include "req_tx.h"
#include "rqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct dcqcn_cb_t d;
struct req_tx_s4_t2_k k;

// r4 is pre-loaded with cur timestamp. Use r4 for CUR_TIMESTAMP.
// NOTE: Non-RTL - feeding timestamp from dcqcn_cb since model doesn't have timestamps.

#if defined (HAPS) || defined (HW)
#define CUR_TIMESTAMP r4
#else
#define CUR_TIMESTAMP d.cur_timestamp
#endif

#define IN_P t2_s2s_sqcb_write_back_info

#define IN_TO_S_P to_s4_dcqcn_bind_mw_info

#define TO_S_STATS_INFO_P to_s7_stats_info

#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info
#define SQCB_WRITE_BACK_INFO_RD t2_s2s_sqcb_write_back_info_rd

#define K_SPEC_MSG_PSN CAPRI_KEY_RANGE(IN_TO_S_P, mr_cookie_msg_psn_sbit0_ebit7, mr_cookie_msg_psn_sbit16_ebit31)
#define K_SPEC_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, host_addr_spec_enable)
#define K_SPEC_RESET  CAPRI_KEY_FIELD(IN_TO_S_P, spec_reset)
#define K_PKT_LEN     CAPRI_KEY_RANGE(IN_TO_S_P, packet_len_sbit0_ebit7, packet_len_sbit8_ebit13)
#define K_S2S_DATA    k.{common_t2_s2s_s2s_data_sbit0_ebit7...common_t2_s2s_s2s_data_sbit136_ebit159}

#define SECS_IN_KSEC         1000
#define NUM_TOKENS_ACQUIRED  r6
#define NUM_TOKENS_REQUIRED  r5

#define C_LAST_PKT           c1
#define C_NON_PKT_WQE        c4
#define C_TEMP_1             c2
#define C_TEMP_2             c3

%%
    .param rdma_num_clock_ticks_per_us
    .param req_tx_write_back_process
    .param req_tx_add_headers_process
    .param req_tx_sqcb2_write_back_process

.align
req_tx_dcqcn_enforce_process:

    // Pin dcqcn_enforce to stage 4
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], bubble_to_next_stage
    
    // Skip this stage if congestion_mgmt is disabled.
    seq           C_TEMP_1, CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_type), 0 //delay slot
    bcf           [C_TEMP_1], load_write_back
    nop // BD Slot

    bbne          K_SPEC_RESET, 1, spec_check
    seq           C_NON_PKT_WQE, CAPRI_KEY_FIELD(IN_P, non_packet_wqe), 1   // BD Slot

    tblwr         d.sq_cindex, K_GLOBAL_SPEC_CINDEX
    b             skip_spec_check
    tblwr.!C_NON_PKT_WQE    d.sq_msg_psn, K_SPEC_MSG_PSN    // BD Slot

spec_check:
    bbne          K_SPEC_ENABLE, 1, skip_msg_psn_check
    seq           C_TEMP_2, K_SPEC_MSG_PSN, d.sq_msg_psn    // BD Slot
    bcf           [!C_NON_PKT_WQE & !C_TEMP_2], spec_fail
skip_msg_psn_check:
    // if speculative cindex matches cindex, then this wqe is being
    // processed in the right order and state update is allowed. Otherwise
    // discard and continue with speculation until speculative cindex
    // matches current cindex. 
    seq           C_TEMP_1, K_GLOBAL_SPEC_CINDEX, d.sq_cindex // BD Slot
    bcf           [!C_TEMP_1], spec_fail
skip_spec_check:
    seq           C_LAST_PKT, CAPRI_KEY_FIELD(IN_P, last_pkt), 1  // BD Slot

    bbeq          CAPRI_KEY_FIELD(IN_P, poll_failed), 1, poll_fail 

    /* Rate enforcement logic.
     * This is done in 2 steps.
     * 1. Replenish tokens based on time elapsed since last_sched_timestamp.
     * 2. Enforce rate to check if there are enough tokens to allow current PHV.
     * Note: Unit of token here is bit. Each token corresponds to 1 bit of data.                     
     */
     // Check if we have reached max-tokens. If so, skip token-replenish.                             
     slt           C_TEMP_2, d.cur_avail_tokens, d.token_bucket_size //BD-slot 
     bcf           [!C_TEMP_2], rate_enforce             

token_replenish:                                  
    sub           r1, CUR_TIMESTAMP, d.last_sched_timestamp // BD-Slot
    add           r1, r1, d.delta_ticks_last_sched 

    // Calculate elapsed-time-in-us since last scheduled.
    div           r3, r1, rdma_num_clock_ticks_per_us 

    // rate-enforced is in Mbps. DCQCN algo will feed rate in Mbps granularity!
    mul           NUM_TOKENS_ACQUIRED, d.rate_enforced, r3 
 
    // Update last-sched-timestamp and delta-ticks only if tokens are acquired in this stage. 
    seq           C_TEMP_1, NUM_TOKENS_ACQUIRED, 0     
    bcf           [C_TEMP_1], rate_enforce
    add           r3, NUM_TOKENS_ACQUIRED, d.cur_avail_tokens // BD-slot

    mod           r2, r1, rdma_num_clock_ticks_per_us                                                
    tblwr         d.delta_ticks_last_sched, r2    
    tblwr         d.last_sched_timestamp, CUR_TIMESTAMP 

    // Replenish tokens in bucket.
    add           r3, NUM_TOKENS_ACQUIRED, d.cur_avail_tokens 
    slt           C_TEMP_2, r3, d.token_bucket_size
    add.!C_TEMP_2 r3, d.token_bucket_size, r0 
    tblwr         d.cur_avail_tokens, r3  

    // If it's non packet wqe, skip rate enforce for both Rome and Dcqcn
    bbeq          CAPRI_KEY_FIELD(IN_P, non_packet_wqe), 1, skip_add_headers

rate_enforce:
    // Calculate num-tokens-required for current pkt and check with available tokens
    add           NUM_TOKENS_REQUIRED, r0, K_PKT_LEN, 3
    slt           C_TEMP_2, d.cur_avail_tokens, NUM_TOKENS_REQUIRED

    bcf           [C_TEMP_2],  drop_phv
    nop //branch delay slot 

    // Deduct cur-pkt-tokens from avail-tokens
    sub           r2, d.cur_avail_tokens, NUM_TOKENS_REQUIRED 
    tblwr         d.cur_avail_tokens, r2

    // Increment DCQCN byte-counter by pkt-len and trigger algorithm if byte-counter threshold is reached.
    add           r2, K_PKT_LEN, d.cur_byte_counter
    tblwr         d.cur_byte_counter, r2
    slt           C_TEMP_1, d.cur_byte_counter, d.byte_counter_thr
    seq           C_TEMP_2, d.max_rate_reached, 1
    bcf           [C_TEMP_1 | C_TEMP_2], skip_dcqcn_doorbell
    nop
    // Reset cur-byte-counter, incr byte counter expiry count and ring dcqcn doorbell to update rate.
    tblwr         d.cur_byte_counter, 0
    tblmincri     d.byte_counter_exp_cnt, 0x10, 1 // byte_counter_exp_cnt is 16-bit value. 
    phvwr         CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, rp_num_byte_threshold_db), 1
    DOORBELL_INC_PINDEX(K_GLOBAL_LIF, Q_TYPE_RDMA_RQ, K_GLOBAL_QID, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)

skip_dcqcn_doorbell:            
    // Increment sq_cindex after successful rate-enforcement/speculative-check.
    tblmincri.C_LAST_PKT    d.sq_cindex, d.log_sq_size, 1
    bcf                     [C_NON_PKT_WQE], load_write_back
    tblwr.C_LAST_PKT        d.sq_msg_psn, 0     // BD Slot
    // Increment sq_msg_psn only for First, Mid non-NPG packets
    tblmincri.!C_LAST_PKT   d.sq_msg_psn, 24, 1

load_write_back:
    // DCQCN rate-enforcement passed. Load stage 5 for write-back/add_headers.
    SQCB2_ADDR_GET(r2)  
    bbeq          CAPRI_KEY_FIELD(IN_P, non_packet_wqe), 1, skip_add_headers
    // Same k info as write_back is passed to add_headers as well
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_add_headers_process, r2)

skip_add_headers:
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqcb2_write_back_process, r2)

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1 | C_TEMP_1], exit
    nop           // Branch Delay Slot

    //invoke the same routine, but with valid header_template_addr as d[] vector
    CAPRI_GET_TABLE_2_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, k.common_te2_phv_table_addr)

exit:
    nop.e
    nop

poll_fail:
spec_fail:
drop_phv:
#if !(defined (HAPS) || defined (HW))
    /* 
     * Feeding new cur_timestamp for next iteration to simulate accumulation of tokens. 
     * Below code is for testing on model only since there are no timestamps on model.
     * Here clock is moved by 100000 ticks for the next iteration.
     */
    add         r1, CUR_TIMESTAMP, 100000
    tblwr       d.cur_timestamp, r1
    tblmincri   d.num_sched_drop, 8, 1 // Increment num_sched_drop by 1
#endif

    // DCQCN rate-enforcement failed. Drop PHV. Loading writeback to adjust spec_cindex
    phvwr CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, rate_enforce_failed), 1
    SQCB2_ADDR_GET(r2)
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_add_headers_process, r2)
