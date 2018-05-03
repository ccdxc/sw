#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rsqwqe_d_t d;
struct resp_tx_s2_t0_k k;


#define RSQWQE_CMP_FULL_MATCH   0
#define RSQWQE_CMP_SUBSET_MATCH 1
#define RSQWQE_CMP_NO_MATCH     2

#define CMP_RESULT      r1
#define RAW_TABLE_PC    r4
#define RSQWQE_P        r5
#define RQCB0_P         r6
#define SEARCH_INDEX    r7

#define RSQ_BT_K_P          t0_s2s_bt_info
#define RSQ_BT_TO_S_K_P     to_s2_bt_info
#define RSQ_BT_TO_S_INFO_T  struct resp_tx_to_stage_bt_info_t
#define WB_INFO_P           t1_s2s_rqcb0_bt_write_back_info
#define K_FIRST_WQE         CAPRI_KEY_FIELD(RSQ_BT_K_P, first_wqe)
#define K_SEARCH_PSN        CAPRI_KEY_RANGE(RSQ_BT_K_P, psn_sbit0_ebit7, psn_sbit16_ebit23)
#define K_OP_TYPE           CAPRI_KEY_FIELD(RSQ_BT_K_P, read_or_atomic)
#define K_R_KEY             CAPRI_KEY_RANGE(RSQ_BT_K_P, r_key_sbit0_ebit23, r_key_sbit24_ebit31)
#define K_VA                CAPRI_KEY_RANGE(RSQ_BT_K_P, va_sbit0_ebit31, va_sbit56_ebit63)
#define K_LEN               CAPRI_KEY_RANGE(RSQ_BT_K_P, len_sbit0_ebit7, len_sbit24_ebit31)
#define K_LOG_RSQ_SIZE      CAPRI_KEY_FIELD(RSQ_BT_TO_S_K_P, log_rsq_size)
#define K_LOG_PMTU          CAPRI_KEY_RANGE(RSQ_BT_TO_S_K_P, log_pmtu_sbit0_ebit2, log_pmtu_sbit3_ebit4)
#define K_RSQ_BASE_ADDR     CAPRI_KEY_RANGE(RSQ_BT_TO_S_K_P, rsq_base_addr_sbit0_ebit5, rsq_base_addr_sbit30_ebit31)
#define K_SEARCH_INDEX      CAPRI_KEY_RANGE(RSQ_BT_TO_S_K_P, search_index_sbit0_ebit5, search_index_sbit14_ebit15)
#define K_END_INDEX         CAPRI_KEY_RANGE(RSQ_BT_TO_S_K_P, end_index_sbit0_ebit5, end_index_sbit14_ebit15)
#define K_BT_CINDEX         CAPRI_KEY_RANGE(RSQ_BT_TO_S_K_P, bt_cindex_sbit0_ebit5, bt_cindex_sbit14_ebit15)

%%
    
    .param  resp_tx_rqcb0_bt_write_back_process

resp_tx_rsqwqe_bt_process:

    //TODO: remove first wqe variable
    // is search_psn less than rsqwqe.psn ?
    // if so, we need to check previous rsqwqe.
    scwlt24     c7, K_SEARCH_PSN, d.psn
    bcf         [c7], go_back_1

    // search psn is at least equal or above current rsqwqe.
    // start comparing with current rsqwqe. if it doesn't match either fully
    // or partially, we can terminate the backtracking. Because,
    // - either the duplicate request is sent with inconsistent params compared to
    //   what we have in rsq.
    // - OR the duplicate request is somewhere between rsq_cindex and rsq_pindex, which
    //   means we haven't reached that request yet (may be resp_tx processing is slow). 
    //   
    // in both cases it is ok to terminate backtrack. In first case standard mandates
    // to drop it. in second case, we will eventually get there and there is nothing to
    // 'back' track.

req_compare:                                                                                 
    
    // if op_type doesn't match, result = NO_MATCH                                           
    seq     c1, d.read_or_atomic, K_OP_TYPE         //BD Slot
    bcf     [!c1], bt_end_entry_not_found

    seq     c2, d.read_or_atomic, RSQ_OP_TYPE_READ  //BD Slot
    bcf     [!c2], compare_atomic
    seq     c3, d.psn, K_SEARCH_PSN                 //BD Slot

compare_read:                                                                                
    // if r_key mismatch, result = NO_MATCH                                                  
    seq     c4, d.read.r_key, K_R_KEY
    bcf     [!c4], bt_end_entry_not_found

    // if psn, va and len matches, result = FULL_MATCH                                       
    seq     c5, d.read.va, K_VA                     //BD Slot
    seq     c6, d.read.len, K_LEN
    bcf     [c3 & c5 & c6], bt_end_full_entry_found

    // if received <va,len> is not within original read's range, result = NO_MATCH           
    slt     c1, K_VA, d.read.va                     //BD Slot
    add     r2, K_VA, K_LEN                      
    add     r3, d.read.va, d.read.len
    slt     c2, r3, r2
    bcf     [c1 | c2], bt_end_entry_not_found

    // if received psn is not within orignal read's range, result = NO_MATCH
    // r4 = (pmtu - 1) + len
    sll     r3, 1, K_LOG_PMTU                       //BD Slot
    add     r4, r3, d.read.len
    sub     r4, r4, 1

    // r4 = r4/pmtu
    srl     r4, r4, K_LOG_PMTU 
    
    // valid psn range is [d.psn, d.psn+r4)
    // note: lower bound is inclusive, upper bound is exclusive

    //psn_hi = r4 + base_psn
    mincr   r4, 24, d.psn
    scwlt24 c1, K_SEARCH_PSN, d.psn
    scwle24 c2, r4, K_SEARCH_PSN
    bcf     [c1 | c2], bt_end_entry_not_found

    //finally check if given va is consistent with given psn
    // r3 = search_psn - d.psn
    sub     r3, r0, d.psn                           //BD Slot
    mincr   r3, 24, K_SEARCH_PSN 
    // r3 = r3 << log_pmtu
    sll     r3, r3, K_LOG_PMTU
    // r4 = search_va - d.va
    sub     r4, K_VA, d.read.va

    // is r4 same as r3 ? If so, result = SUBSET_MATCH, else NO_MATCH
    seq     c1, r3, r4

    bcf     [c1], bt_end_subset_entry_found
    nop     //BD Slot
    
    b       bt_end_entry_not_found
    nop     //BD Slot

compare_atomic:
    // if  psn, r_key and va matches, result = FULL_MATCH else NO_MATCH
    seq     c4, d.atomic.r_key, K_R_KEY 
    seq     c5, d.atomic.va, K_VA 
    bcf     [c3 & c4 & c5], bt_end_full_entry_found
    nop     //BD Slot
    b       bt_end_entry_not_found
    nop     //BD Slot
    
go_back_1:
    add         SEARCH_INDEX, r0, K_SEARCH_INDEX
    mincr       SEARCH_INDEX, K_LOG_RSQ_SIZE, -1

    // see if we have reached boundary 
    seq         c1, SEARCH_INDEX, K_END_INDEX
    bcf         [c1], bt_end_entry_not_found

    // see if we have reached STAGE_6. If so, yield by logging progress
    mfspr       r2, spr_mpuid       //BD Slot
    seq         c1, r2[4:2], STAGE_6
    bcf         [c1], bt_yield
    nop         //BD Slot

    // we haven't reached STAGE_6 and still rsq available to explore,
    // go back 1
    sll         RSQWQE_P, K_RSQ_BASE_ADDR, RSQ_BASE_ADDR_SHIFT
    add         RSQWQE_P, RSQWQE_P, SEARCH_INDEX, LOG_SIZEOF_RSQWQE_T

    // stage-to-stage info is carried as is by default. so do not need to repopulate
    // copy the to_stage info to next stage
    CAPRI_GET_STAGE_NEXT_ARG(resp_tx_phv_t, r1)
    CAPRI_SET_FIELD_RANGE(r1, RSQ_BT_TO_S_INFO_T, log_rsq_size, bt_in_progress, CAPRI_KEY_RANGE(RSQ_BT_TO_S_K_P, log_rsq_size, bt_in_progress))

    // use david's hack to invoke the same program again in next stage 
    bal     RAW_TABLE_PC, calculate_raw_table_pc
    // update the search_index field with new value
    CAPRI_SET_FIELD(r1, RSQ_BT_TO_S_INFO_T, search_index, SEARCH_INDEX) //BD Slot

calculate_raw_table_pc:
    // $ here denores releative current instruction position
    sub     RAW_TABLE_PC, RAW_TABLE_PC, $

    // fire table and exit
    //CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, RAW_TABLE_PC, RSQWQE_P)
    CAPRI_GET_TABLE_0_K_NO_VALID(resp_tx_phv_t, r3)
    CAPRI_NEXT_TABLE_I_READ_E(r3, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, RAW_TABLE_PC, RSQWQE_P)



bt_yield:
    // we didn't find the request so far, but reached STAGE_6. 
    // still few more rsq entries need to be checked.
    // instead of recirc (which holds on to phv and impacts other flows and 
    // also has a disadvantage of limiting number of rsq entries that can be 
    // backtracked), yied and let scheduler invoke us again. 
    // that means, set bt_in_progress to 1 and also remember bt_rsq_cindex.
    
    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_RESET_TABLE_1_ARG()
    // read_rsp_in_progress = 0
    // bt_in_progress = 1
    // update_read_rsp_in_progress = 0
    // update_rsq_cindex = 0
    // update_bt_cindex = 0
    // update_bt_in_progress = 1
    // update_bt_rsq_cindex = 1
    phvwrpair   CAPRI_PHV_RANGE(WB_INFO_P, read_rsp_in_progress, update_bt_rsq_cindex), \
                0x23, \
                CAPRI_PHV_FIELD(WB_INFO_P, bt_rsq_cindex), \
                SEARCH_INDEX

    RQCB0_ADDR_GET(RQCB0_P)
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb0_bt_write_back_process, RQCB0_P)
    
    
bt_end_entry_not_found:
    // we didn't find the request and reached end of rsq. 
    // terminate backtrack as if nothing happened.
    // that means, do not touch read_rsp_in_progress/rsq_cindex/curr_read_rsp_psn values.
    // simply update bt ring cindex (so that it is muted) and set bt_in_progress to 0.

    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_RESET_TABLE_1_ARG()
    // read_rsp_in_progress = 0
    // bt_in_progress = 0
    // update_read_rsp_in_progress = 0
    // update_rsq_cindex = 0
    // update_bt_cindex = 1
    // update_bt_in_progress = 1
    // update_bt_rsq_cindex = 0 
    phvwrpair   CAPRI_PHV_RANGE(WB_INFO_P, read_rsp_in_progress, update_bt_rsq_cindex), \
                0x6, \
                CAPRI_PHV_FIELD(WB_INFO_P, bt_cindex), \
                K_BT_CINDEX

    RQCB0_ADDR_GET(RQCB0_P)
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb0_bt_write_back_process, RQCB0_P)
    

bt_end_full_entry_found:
    // we found a complete match. 
    // terminate backtrack and set rsq_cindex to point to the SEARCH_INDEX so that
    // rsq processing logic kicks in from this point.

    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_RESET_TABLE_1_ARG()
    // read_rsp_in_progress = 0
    // bt_in_progress = 0
    // update_read_rsp_in_progress = 0
    // update_rsq_cindex = 1
    // update_bt_cindex = 1
    // update_bt_in_progress = 1
    // update_bt_rsq_cindex = 0 
    phvwrpair   CAPRI_PHV_RANGE(WB_INFO_P, read_rsp_in_progress, update_bt_rsq_cindex), \
                0xE, \
                CAPRI_PHV_FIELD(WB_INFO_P, rsq_cindex), \
                K_SEARCH_INDEX
    CAPRI_SET_FIELD2(WB_INFO_P, bt_cindex, K_BT_CINDEX)

    RQCB0_ADDR_GET(RQCB0_P)
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb0_bt_write_back_process, RQCB0_P)
    

bt_end_subset_entry_found:
    // we found a subset match. 
    // terminate backtrack and set rsq_cindex to point to the SEARCH_INDEX and
    // also set curr_read_rsp_psn to search_psn and set the read_rsp_in_progress to 1 
    // so that rsq processing logic kicks in from this point.

    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_RESET_TABLE_1_ARG()
    // read_rsp_in_progress = 1
    // bt_in_progress = 0
    // update_read_rsp_in_progress = 1
    // update_rsq_cindex = 1
    // update_bt_cindex = 1
    // update_bt_in_progress = 1
    // update_bt_rsq_cindex = 0 
    phvwrpair   CAPRI_PHV_RANGE(WB_INFO_P, read_rsp_in_progress, update_bt_rsq_cindex), \
                0x5E, \
                CAPRI_PHV_FIELD(WB_INFO_P, rsq_cindex), \
                K_SEARCH_INDEX
    CAPRI_SET_FIELD2(WB_INFO_P, bt_cindex, K_BT_CINDEX)

    RQCB0_ADDR_GET(RQCB0_P)
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqcb0_bt_write_back_process, RQCB0_P)
