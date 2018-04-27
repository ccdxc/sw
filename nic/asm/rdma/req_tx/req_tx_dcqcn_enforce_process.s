#include "req_tx.h"
#include "rqcb.h"
#include "common_defines.h"

struct req_tx_phv_t p;
struct dcqcn_cb_t d;
struct req_tx_s3_t2_k k;

// r4 is pre-loaded with cur timestamp. Use r4 for CUR_TIMESTAMP.
// NOTE: Non-RTL - feeding timestamp from dcqcn_cb since model doesn't have timestamps.

#define CUR_TIMESTAMP d.cur_timestamp

#define TO_S5_P to_s5_sq_to_stage

#define IN_P t2_s2s_sqcb_write_back_info
#define IN_TO_S_P to_s3_sq_to_stage

#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info
#define SQCB_WRITE_BACK_INFO_RD t3_s2s_sqcb_write_back_info_rd

#define K_SPEC_CINDEX CAPRI_KEY_RANGE(IN_TO_S_P, spec_cindex_sbit0_ebit7, spec_cindex_sbit8_ebit15)
#define K_PKT_LEN     CAPRI_KEY_RANGE(IN_TO_S_P, packet_len_sbit0_ebit7, packet_len_sbit8_ebit13)
#define K_S2S_DATA    k.{common_t2_s2s_s2s_data_sbit0_ebit7...common_t2_s2s_s2s_data_sbit152_ebit159}
#define K_HEADER_TEMPLATE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, header_template_addr_sbit0_ebit7, header_template_addr_sbit24_ebit31)

#define SECS_IN_KSEC         1000
#define NUM_TOKENS_ACQUIRED  r6
#define NUM_TOKENS_REQUIRED  r5

%%
    .param rdma_num_clock_ticks_per_us
    .param req_tx_write_back_process
    .param req_tx_add_headers_process
    .param req_tx_load_hdr_template_process

.align
req_tx_dcqcn_enforce_process:
    
    // Pin dcqcn_enforce to stage 4
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], bubble_to_next_stage
    
    // Skip this stage if congestion_mgmt is disabled.
    seq           c2, CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_enable), 0 //delay slot
    bcf           [c2], load_write_back

    // if speculative cindex matches cindex, then this wqe is being
    // processed in the right order and state update is allowed. Otherwise
    // discard and continue with speculation until speculative cindex
    // matches current cindex. 
    seq            c1, K_SPEC_CINDEX, d.sq_cindex // BD-slot
    bcf            [!c1], spec_fail
    nop

    bbeq          CAPRI_KEY_FIELD(IN_P, poll_failed), 1, poll_fail 

    /* Rate enforcement logic.
     * This is done in 2 steps.
     * 1. Replenish tokens based on time elapsed since last_sched_timestamp.
     * 2. Enforce rate to check if there are enough tokens to allow current PHV.
     * Note: Unit of token here is bit. Each token corresponds to 1 bit of data.                     
     */
     // Check if we have reached max-tokens. If so, skip token-replenish.                             
     slt           c3, d.cur_avail_tokens, d.token_bucket_size //BD-slot 
     bcf           [!c3], rate_enforce             

token_replenish:                                  
    sub           r1, CUR_TIMESTAMP, d.last_sched_timestamp // BD-Slot
    add           r1, r1, d.delta_ticks_last_sched 

    // Calculate elapsed-time-in-us since last scheduled.
    div           r3, r1, rdma_num_clock_ticks_per_us 

    // rate-enforced is in Mbps. DCQCN algo will feed rate in Mbps granularity!
    mul           NUM_TOKENS_ACQUIRED, d.rate_enforced, r3 
 
    // Update last-sched-timestamp and delta-ticks only if tokens are acquired in this stage. 
    seq           c1, NUM_TOKENS_ACQUIRED, 0     
    bcf           [c1], rate_enforce
    add           r3, NUM_TOKENS_ACQUIRED, d.cur_avail_tokens // BD-slot

    mod           r2, r1, rdma_num_clock_ticks_per_us                                                
    tblwr         d.delta_ticks_last_sched, r2    
    tblwr         d.last_sched_timestamp, CUR_TIMESTAMP 

    // Replenish tokens in bucket.
    add           r3, NUM_TOKENS_ACQUIRED, d.cur_avail_tokens 
    slt           c3, r3, d.token_bucket_size
    add.!c3       r3, d.token_bucket_size, r0 
    tblwr         d.cur_avail_tokens, r3  

rate_enforce:
    // Calculate num-tokens-required for current pkt and check with available tokens
    add           NUM_TOKENS_REQUIRED, r0, K_PKT_LEN, 3
    slt           c3, d.cur_avail_tokens, NUM_TOKENS_REQUIRED

    bcf           [c3],  drop_phv
    nop //branch delay slot 

    // Deduct cur-pkt-tokens from avail-tokens
    sub           r2, d.cur_avail_tokens, NUM_TOKENS_REQUIRED 
    tblwr         d.cur_avail_tokens, r2

    // Increment DCQCN byte-counter by pkt-len and trigger algorithm if byte-counter threshold is reached.
    add           r2, K_PKT_LEN, d.cur_byte_counter
    tblwr         d.cur_byte_counter, r2
    slt           c2, d.cur_byte_counter, d.byte_counter_thr
    seq           c4, d.max_rate_reached, 1
    bcf           [c2 | c4], skip_dcqcn_doorbell
    nop
    // Reset cur-byte-counter, incr byte counter expiry count and ring dcqcn doorbell to update rate.
    tblwr         d.cur_byte_counter, 0
    tblmincri     d.byte_counter_exp_cnt, 0x10, 1 // byte_counter_exp_cnt is 16-bit value. 
    DOORBELL_INC_PINDEX(K_GLOBAL_LIF, Q_TYPE_RDMA_RQ, K_GLOBAL_QID, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)

skip_dcqcn_doorbell:            
    // Increment sq_cindex after successful rate-enforcement/speculative-check.
    seq             c1, CAPRI_KEY_FIELD(IN_P, last_pkt), 1  
    tblmincri.c1    d.sq_cindex, 12, 1  // TODO: Hardcoding log_sq_size to 12 for now. This has to come from HAL.

load_write_back:
    // DCQCN rate-enforcement passed. Load stage 5 for write-back
    SQCB0_ADDR_GET(r2)
    //It is assumed that hdr_template_inline flag is passed untouched to next table-2.
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_write_back_process, r2)

    SQCB2_ADDR_GET(r2)
    // Same k info as write_back is passed to add_headers as well
    phvwr          p.common.common_t3_s2s_s2s_data, K_S2S_DATA
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_add_headers_process, r2)


    bbne          CAPRI_KEY_FIELD(IN_P, hdr_template_inline), 1, skip_hdr_template_inline
    sll           r2, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT //BD slot
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_load_hdr_template_process, r2)

skip_hdr_template_inline:
    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1 | c2], exit
    nop           // Branch Delay Slot

    add           r2, HDR_TEMPLATE_T_SIZE_BYTES, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT 
    //invoke the same routine, but with valid header_template_addr as d[] vector
    CAPRI_GET_TABLE_2_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r2)
exit:
    nop.e
    nop

poll_fail:
spec_fail:
drop_phv:
    // DCQCN rate-enforcement failed. Drop PHV. Loading writeback to adjust spec_cindex
    phvwr CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, rate_enforce_failed), 1

    SQCB0_ADDR_GET(r2)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_write_back_process, r2)

    SQCB2_ADDR_GET(r2)
    phvwr       p.common.common_t3_s2s_s2s_data, K_S2S_DATA
    phvwr       CAPRI_PHV_FIELD(SQCB_WRITE_BACK_INFO_RD, rate_enforce_failed), 1
    phvwr       CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, rate_enforce_failed), 1

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_add_headers_process, r2)

    /* 
     * Feeding new cur_timestamp for next iteration to simulate accumulation of tokens. 
     * Below code is for testing on model only since there are no timestamps on model.
     * Here clock is moved by 100000 ticks for the next iteration.
     */
    add         r1, CUR_TIMESTAMP, 100000
    tblwr       d.cur_timestamp, r1
    tblmincri   d.num_sched_drop, 8, 1 // Increment num_sched_drop by 1
    nop.e
    nop
