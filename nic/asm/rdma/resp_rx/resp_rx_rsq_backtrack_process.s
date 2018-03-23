#include "capri.h"
#include "types.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s1_t0_k k;
struct rsqwqe_d_t d;

#define RSQWQE_CMP_FULL_MATCH   0
#define RSQWQE_CMP_SUBSET_MATCH 1
#define RSQWQE_CMP_NO_MATCH     2

#define CMP_RESULT      r1
#define RQCB0_P         r2
#define RSQWQE_P        r2
#define RAW_TABLE_PC    r5
#define NEW_INDEX       r6

#define ADJUST_C_INDEX_F    c6 
#define ADJUST_P_INDEX_F    c7

#define ADJUST_INFO_T struct resp_rx_rsq_backtrack_adjust_info_t
#define RSQ_BT_INFO_T struct resp_rx_rsq_backtrack_info_t 
#define RSQ_BT_TO_S_INFO_T struct resp_rx_to_stage_backtrack_info_t


%%
    .param  resp_rx_rsq_backtrack_adjust_process

.align
resp_rx_rsq_backtrack_process:

    //TBD: migrate to new logic

//    //copy to_stage params to the next stage
//    CAPRI_GET_STAGE_NEXT_ARG(resp_rx_phv_t, r1)
//    #copy va, r_key, len from to-stage-current to to-stage-next
//    CAPRI_SET_FIELD_RANGE(r1, RSQ_BT_TO_S_INFO_T, va, len, k.{to_stage.s0.backtrack.va...to_stage.s0.backtrack.len})
//
//req_compare:
//    
//    // if op_type doesn't match, result = NO_MATCH
//    seq     c1, d.read_or_atomic, k.args.read_or_atomic
//    bcf     [!c1], compare_done
//    add     CMP_RESULT, r0, RSQWQE_CMP_NO_MATCH //BD Slot
//
//    seq     c2, d.read_or_atomic, RSQ_OP_TYPE_READ
//    bcf     [!c2], compare_atomic
//    seq     c3, d.psn, k.args.search_psn //BD Slot
//
//compare_read:
//    // if r_key mismatch, result = NO_MATCH
//    seq     c4, d.read.r_key, k.to_stage.s0.backtrack.r_key
//    bcf     [!c4], compare_done
//    add     CMP_RESULT, r0, RSQWQE_CMP_NO_MATCH //BD Slot
//
//    // if psn, va and len matches, result = FULL_MATCH
//    seq     c5, d.read.va, k.to_stage.s0.backtrack.va
//    seq     c6, d.read.len, k.to_stage.s0.backtrack.len
//    bcf     [c3 & c5 & c6], compare_done
//    add     CMP_RESULT, r0, RSQWQE_CMP_FULL_MATCH //BD Slot
//
//    // if received <va,len> is not within original read's range, result = NO_MATCH
//    slt     c1, k.to_stage.s0.backtrack.va, d.read.va
//    add     r2, k.to_stage.s0.backtrack.va, k.to_stage.s0.backtrack.len
//    add     r3, d.read.va, d.read.len
//    slt     c2, r3, r2
//    bcf     [c1 | c2], compare_done
//    add     CMP_RESULT, r0, RSQWQE_CMP_NO_MATCH //BD Slot
//
//    // if received psn is not within orignal read's range, result = NO_MATCH
//    
//    // r2 = log_pmtu
//    //add     r2, r0, k.args.log_pmtu
//    // r3 = 1 << log_pmtu (i.e., pmtu)
//    sll     r3, 1, k.args.log_pmtu 
//    // r4 = len + pmtu 
//    add     r4, d.read.len, r3
//    // r4 = r4-1
//    sub     r4, r4, 1
//    // r4 = r4/pmtu
//    srl     r4, r4, k.args.log_pmtu 
//    
//    // valid psn range is [d.psn, d.psn+r4)
//    // note: lower bound is inclusive, upper bound is exclusive
//
//    //psn_hi = r4 + base_psn
//    mincr   r4, 24, d.psn
//    scwlt24 c1, k.args.search_psn, d.psn
//    scwle24 c2, r4, k.args.search_psn
//    bcf     [c1 | c2], compare_done
//    add     CMP_RESULT, r0, RSQWQE_CMP_NO_MATCH //BD Slot
//
//    //finally check if given va is consistent with given psn
//    // r3 = search_psn - d.psn
//    sub     r3, r0, d.psn
//    mincr   r3, 24, k.args.search_psn 
//    // r3 = r3 << log_pmtu
//    sll     r3, r3, k.args.log_pmtu 
//    // r4 = search_va - d.va
//    sub     r4, k.to_stage.s0.backtrack.va, d.read.va
//    
//    // is r4 same as r3 ? If so, result = SUBSET_MATCH, else NO_MATCH
//    seq     c1, r3, r4
//    b       compare_done
//    cmov    CMP_RESULT, c1, RSQWQE_CMP_SUBSET_MATCH, RSQWQE_CMP_NO_MATCH //BD Slot
//
//compare_atomic:
//    // if  psn, r_key and va matches, result = FULL_MATCH else NO_MATCH
//    seq     c4, d.atomic.r_key, k.to_stage.s0.backtrack.r_key
//    seq     c5, d.atomic.va, k.to_stage.s0.backtrack.va
//    setcf   c6, [c3 & c4 & c5]
//    cmov    CMP_RESULT, c6, RSQWQE_CMP_FULL_MATCH, RSQWQE_CMP_NO_MATCH
//
//compare_done:
//
//    // by default, adjust_c_index and adjust_p_index are FALSE
//    setcf   ADJUST_C_INDEX_F, [!c0]
//    setcf   ADJUST_P_INDEX_F, [!c0]
//
//    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)
//
//    seq     c1, k.args.walk, RSQ_EVAL_MIDDLE
//    bcf     [!c1], check_fwd_bwd
//    RQCB0_ADDR_GET(RQCB0_P)  //BD Slot
//
//eval_middle:
//    //TODO:
//    // is search_psn < wqe_p->psn ?
//    scwlt24 c1, k.args.search_psn, d.psn
//    // if this condition is true, that means what we are searching
//    // for is already executed (and that is why cindex has reached
//    // this far in the first place) and hence initiate backward search.
//    // if false, check if current wqe matches or need forward search.
//    bcf     [!c1], eval_curr_or_fwd
//    seq     c2, k.args.index, k.args.lo_index   //BD Slot
//    // if we reached bottom, then give up 
//    bcf     [c2], adjust_table_setup
//    add     NEW_INDEX, r0, k.args.index //BD Slot
//    mincr   NEW_INDEX, k.args.log_rsq_size, -1
//    
//    // walk, index
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, walk, RSQ_WALK_BACKWARD)
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, index, NEW_INDEX)
//
//    sll     RSQWQE_P, k.args.rsq_base_addr, RSQ_BASE_ADDR_SHIFT
//    b       backtrack_setup
//    add     RSQWQE_P, RSQWQE_P, NEW_INDEX, LOG_SIZEOF_RSQWQE_T //BD Slot
//    
//eval_curr_or_fwd:
//    // is cmp_result < no_match
//    // since no_match is defined as 2 and match/subset_match are defined as 0,1
//    // below condition will be TRUE if match/subset_match is found
//    // in such a case, we found a match in the middle, adjust_c_index to current index.
//    slt     c2, CMP_RESULT, RSQWQE_CMP_NO_MATCH
//    bcf     [c2], adjust_table_setup
//    setcf.c2    ADJUST_C_INDEX_F, [c0] //BD Slot
//
//    // did we reach top ? if so, give up
//    seq     c2, k.args.index, k.args.hi_index
//    bcf     [c2], adjust_table_setup
//    //otherwise, search forward
//    add     NEW_INDEX, r0, k.args.index     //BD Slot
//    mincr   NEW_INDEX, k.args.log_rsq_size, 1
//    
//    // walk, index
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, walk, RSQ_WALK_FORWARD)
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, index, NEW_INDEX)
//
//    sll     RSQWQE_P, k.args.rsq_base_addr, RSQ_BASE_ADDR_SHIFT
//    b       backtrack_setup
//    add     RSQWQE_P, RSQWQE_P, NEW_INDEX, LOG_SIZEOF_RSQWQE_T //BD Slot
//    
//check_fwd_bwd:
//    seq     c1, k.args.walk, RSQ_WALK_FORWARD
//    bcf     [c1], walk_forward
//    seq.c1  c2, CMP_RESULT, RSQWQE_CMP_FULL_MATCH // BD Slot
//
//walk_backward:
//    // if either full match or subset match, adjust_c_index = TRUE
//    slt         c2, CMP_RESULT, RSQWQE_CMP_NO_MATCH      
//    bcf         [c2], adjust_table_setup
//    setcf.c2    ADJUST_C_INDEX_F, [c0] //BD Slot
//
//    // if we reach end of backward search, simply go to next stage
//    // which will increment nxt_to_go_token_id
//    seq     c3, k.args.index, k.args.lo_index   //BD Slot
//    bcf     [c3], adjust_table_setup
//
//    //continue search backward
//    add     NEW_INDEX, r0, k.args.index     //BD Slot
//    mincr   NEW_INDEX, k.args.log_rsq_size, -1
//    
//    // walk, index
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, walk, RSQ_WALK_BACKWARD)
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, index, NEW_INDEX)
//
//    sll     RSQWQE_P, k.args.rsq_base_addr, RSQ_BASE_ADDR_SHIFT
//    b       backtrack_setup
//    add     RSQWQE_P, RSQWQE_P, NEW_INDEX, LOG_SIZEOF_RSQWQE_T //BD Slot
//    
//walk_forward:
//    // if we are walking forward from mid point,
//    // duplicate request should match exactly with
//    // original request
//    // if it matches, adjust_p_index = TRUE
//    bcf         [c2], adjust_table_setup
//    setcf.c2    ADJUST_P_INDEX_F, [c0]  //BD Slot
//
//    // if we reach end of forward search, simply go to next stage
//    // which will increment nxt_to_go_token_id
//    seq     c3, k.args.index, k.args.hi_index   //BD Slot
//    bcf     [c3], adjust_table_setup
//    // continue search forward
//    add     NEW_INDEX, r0, k.args.index     //BD Slot
//    mincr   NEW_INDEX, k.args.log_rsq_size, 1
//    
//    // walk, index
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, walk, RSQ_WALK_FORWARD)
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, index, NEW_INDEX)
//
//    sll     RSQWQE_P, k.args.rsq_base_addr, RSQ_BASE_ADDR_SHIFT
//    b       backtrack_setup
//    add     RSQWQE_P, RSQWQE_P, NEW_INDEX, LOG_SIZEOF_RSQWQE_T //BD Slot
//    
//adjust_table_setup:
//    CAPRI_SET_FIELD_C(r4, ADJUST_INFO_T, adjust_c_index, 1, ADJUST_C_INDEX_F)
//    CAPRI_SET_FIELD_C(r4, ADJUST_INFO_T, adjust_p_index, 1, ADJUST_P_INDEX_F)
//    CAPRI_SET_FIELD(r4, ADJUST_INFO_T, index, k.args.index)
//
//    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rsq_backtrack_adjust_process, RQCB0_P)
//
//    //TODO: enable below code once spr_mpuid register is available
//    //mfspr       spr_mpuid, r6
//    //seq         c1, r6[4:2], CAPRI_STAGE_LAST
//    //phvwr.c1    p.common.p4_intr_recirc, 1
//
//    nop.e
//    nop
//
//backtrack_setup:
//    // it is expected that whoever is branching to this label would
//    // already set fields: walk, index
//    // RSQWQE_P also need to be set as well
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, log_pmtu, k.args.log_pmtu)    
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, read_or_atomic, k.args.read_or_atomic)    
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, hi_index, k.args.hi_index)
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, lo_index, k.args.lo_index)
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, log_rsq_size, k.args.log_rsq_size)    
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, search_psn, k.args.search_psn)    
//    CAPRI_SET_FIELD(r4, RSQ_BT_INFO_T, rsq_base_addr, k.args.rsq_base_addr)    
//
//    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
//    
//    //ATTENTION: Here the goal is to invoke exactly the same program in the next 
//    // stage, but with different arguments and different table address. I cannot
//    // set RAW_TABLE_PC like below as string cannot be used as label and as well
//    // as param. Hence David suggested a workaround which is what is implemented
//    // below.
//    //CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rsq_backtrack_process)
//    bal     RAW_TABLE_PC, calculate_raw_table_pc
//    nop     //BD Slot
//
//calculate_raw_table_pc:
//    // $ here denores releative current instruction position
//    sub     RAW_TABLE_PC, RAW_TABLE_PC, $
//
//    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RSQWQE_P)
//
exit:
    nop.e
    nop
