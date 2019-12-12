#include "resp_tx.h"
#include "rqcb.h"

struct resp_tx_phv_t p;
struct dcqcn_cb_t d;

// Note: This stage doesn't have any stage-to-stage info. k is used only to access to-stage info.
struct resp_tx_s4_t1_k k;

// r4 is pre-loaded with cur timestamp. Use r4 for CUR_TIMESTAMP.
// NOTE: Non-RTL - feeding timestamp from dcqcn_cb since model doesn't have timestamps.

#if defined (HAPS) || defined (HW)
#define CUR_TIMESTAMP r4
#else
#define CUR_TIMESTAMP d.cur_timestamp
#endif

#define SECS_IN_KSEC         1000
#define NUM_TOKENS_ACQUIRED  r6
#define NUM_TOKENS_REQUIRED  r5

#define RQCB0_WB_INFO_P t1_s2s_rqcb0_write_back_info

#define IN_TO_S4_P  to_s4_dcqcn_info
#define K_PKT_LEN CAPRI_KEY_RANGE(IN_TO_S4_P, packet_len_sbit0_ebit3, packet_len_sbit12_ebit13)
#define TO_S_STATS_INFO_P to_s7_stats_info
%%
    .param rdma_num_clock_ticks_per_us
    .param resp_tx_rqcb0_write_back_process

.align
resp_tx_dcqcn_enforce_process:
    
    // Pin dcqcn_enforce to stage 4
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], bubble_to_next_stage
    
    // Skip this stage if congestion_mgmt is disabled.
    seq           c2, CAPRI_KEY_FIELD(IN_TO_S4_P, congestion_mgmt_type), 0 //delay slot
    bcf           [c2], load_write_back

    seq           c1, CAPRI_KEY_FIELD(IN_TO_S4_P, resp_rl_failure), 1   // BD Slot
    seq           c2, d.resp_rl_failure, 1
    bcf           [c2 & !c1], drop_phv
    tblwr.c1      d.resp_rl_failure, 0
    /* Rate enforcement logic.
     * This is done in 2 steps.
     * 1. Replenish tokens based on time elapsed since last_sched_timestamp.
     * 2. Enforce rate to check if there are enough tokens to allow current PHV.
     * Note: Unit of token here is bit. Each token corresponds to 1 bit of data.
     */

    // Check if we have reached max-tokens. If so, skip token-replenish.
    slt           c3, d.cur_avail_tokens, d.token_bucket_size
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
    bcf           [c2 | c4], load_write_back
    nop

ring_dcqcn_doorbell:
    // Reset cur-byte-counter, incr byte counter expiry count and ring dcqcn doorbell to update rate.
    tblwr         d.cur_byte_counter, 0
    tblmincri     d.byte_counter_exp_cnt, 0x10, 1 // byte_counter_exp_cnt is 16-bit value. 
    phvwr         CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, rp_num_byte_threshold_db), 1
    DOORBELL_INC_PINDEX(K_GLOBAL_LIF,  K_GLOBAL_QTYPE, K_GLOBAL_QID, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)
    
load_write_back:            
    // DCQCN rate-enforcement passed. Load stage 5 for write-back.
    RQCB0_ADDR_GET(r2)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb0_write_back_process, r2)

    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    //seq           c2, k.to_stage.s3.dcqcn.congestion_mgmt_type, 0
    // even though it seems to be referring to "to_s4" data, actually it is 
    // looking at "to_s3". Unfortunately, this confusion cannot be avoided
    // unless we write separate asm programs for s3 and s4.
    seq           c2, CAPRI_KEY_FIELD(IN_TO_S4_P, congestion_mgmt_type), 0
    bcf           [!c1 | c2], exit
    nop           // Branch Delay Slot

    CAPRI_GET_TABLE_1_K(resp_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, CAPRI_KEY_RANGE(IN_TO_S4_P, dcqcn_cb_addr_sbit0_ebit31, dcqcn_cb_addr_sbit32_ebit33))

exit:
    nop.e
    nop

drop_phv:

    tblwr         d.resp_rl_failure, 1
    // DCQCN rate-enforcement failed. Drop PHV and load rqcb1.
    phvwr         p.common.p4_intr_global_drop, 1 

    CAPRI_SET_FIELD2(RQCB0_WB_INFO_P, rate_enforce_failed, 1)

    RQCB0_ADDR_GET(r2)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb0_write_back_process, r2)

#if !(defined (HAPS) || defined (HW))
    /* 
     * Feeding new cur_timestamp for next iteration to simulate accumulation of tokens. 
     * Below code is for testing on model only since there are no timestamps on model.
     * Here clock is moved by 8192000 ticks for the next iteration.
     */
    mul         r1, 8192, 1000 // 8192 = 1024*8, since pMTU = 1024
    add         r1, r1, CUR_TIMESTAMP 
    tblwr       d.cur_timestamp, r1
    tblmincri   d.num_sched_drop, 8, 1 // Increment num_sched_drop by 1
#endif

    nop.e
    nop
