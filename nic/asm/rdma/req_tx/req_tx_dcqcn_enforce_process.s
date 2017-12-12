#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct dcqcn_cb_t d;
struct req_tx_add_headers_process_k_t k;

// r4 is pre-loaded with cur timestamp. Use r4 for CUR_TIMESTAMP.
// NOTE: Feeding timestamp from dcqcn_cb for now since model doesn't have timestamps.
#define CUR_TIMESTAMP d.cur_timestamp
#define TO_STAGE_T struct req_tx_to_stage_t

#define SECS_IN_KSEC         1000
#define NUM_TOKENS_ACQUIRED  r4
#define NUM_TOKENS_REQUIRED  r5

%%
    .param rdma_num_clock_ticks_per_us
    .param req_tx_write_back_process

.align
req_tx_dcqcn_enforce_process:
    
    // Pin dcqcn_enforce to stage 4
    mfspr         r1, spr_mpuid
    seq           c1, r1[6:2], STAGE_4
    bcf           [!c1], bubble_to_next_stage
    
    // Skip this stage if congestion_mgmt is disabled.
    seq           c2, k.to_stage.sq.congestion_mgmt_enable, 0 //delay slot
    bcf           [c2], load_write_back
    nop

    /* Rate enforcement logic.
     * This is done in 2 steps.
     * 1. Replenish tokens based on time elapsed since last_sched_timestamp.
     * 2. Enforce rate to check if there are enough tokens to allow current PHV.
     */
    sub           r2, CUR_TIMESTAMP, d.last_sched_timestamp
    //diff in us since last scheduled
    div           r3, r2, rdma_num_clock_ticks_per_us 

    mul           r2, d.rate_enforced, r3 // rate-enforced is in kbps
    // r3 has number of tokens accumulated since last sched.
    divi          NUM_TOKENS_ACQUIRED, r2, SECS_IN_KSEC    
  
    seq           c1, NUM_TOKENS_ACQUIRED, 0     
    tblwr.!c1     d.last_sched_timestamp, CUR_TIMESTAMP 

    add           r3, NUM_TOKENS_ACQUIRED, d.cur_avail_tokens 
    tblwr         d.cur_avail_tokens, r3  // Replenish tokens in bucket.

    add           r3, k.to_stage.sq.packet_len, r0
    muli          NUM_TOKENS_REQUIRED, r3, 8 // Convert pkt_len to bits.
    slt           c3, d.cur_avail_tokens, NUM_TOKENS_REQUIRED

    bcf           [c3],  drop_phv
    nop                                                        //branch delay slot 
    sub           r2, d.cur_avail_tokens, NUM_TOKENS_REQUIRED // deduct cur-pkt-tokens from avail-tokens
    tblwr         d.cur_avail_tokens, r2

load_write_back:            
    // DCQCN rate-enforcement passed. Load stage 5 for write-back
    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_write_back_process)
    SQCB0_ADDR_GET(r2)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r6, r2)

    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[6:2], STAGE_3
    bcf           [!c1 | c2], exit
    nop           // Branch Delay Slot

    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS)
exit:
    nop.e
    nop

drop_phv:
    // DCQCN rate-enforcement failed. Drop PHV. Loading writeback to adjust spec_cindex
    CAPRI_GET_STAGE_5_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.rate_enforce_failed, 1)

    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_write_back_process)
    SQCB0_ADDR_GET(r2)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r6, r2)

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
