#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s4_t0_k k;
struct key_entry_aligned_t d;

#define IN_P t0_s2s_sqlkey_to_rkey_mw_info

#define K_ACC_CTRL         CAPRI_KEY_FIELD(IN_P, acc_ctrl)
#define K_VA               CAPRI_KEY_RANGE(IN_P, va_sbit0_ebit7, va_sbit56_ebit63)
#define K_LEN              CAPRI_KEY_RANGE(IN_P, len_sbit0_ebit7, len_sbit24_ebit31)
#define K_ZBVA             CAPRI_KEY_FIELD(IN_P, zbva)
#define K_LOG_PAGE_SIZE    CAPRI_KEY_RANGE(IN_P, log_page_size_sbit0_ebit4, log_page_size_sbit5_ebit7)
#define K_USER_KEY         CAPRI_KEY_FIELD(IN_P, new_r_key_key)
#define K_MW_PT_BASE       CAPRI_KEY_RANGE(IN_P, mw_pt_base_sbit0_ebit7, mw_pt_base_sbit24_ebit31)
#define K_MW_TYPE          CAPRI_KEY_FIELD(IN_P, mw_type)

%%

.align
req_tx_bind_mw_rkey_process:
    // TODO PD check

    // Pin bind_mw_sqlkey to stage 4, same as sqlkey_process so that
    // key_entry can be locked and updated.
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_4
    bcf            [!c1], bubble_to_next_stage

    // If bind_mw, rkey should allow type1. If post_send_bind_mw, 
    // rkey should allow type2. In case rkey was allocated without 
    // type qualifier then it allows either type 1 or type 2 mw bind
    and            r2, K_MW_TYPE, d.type
    beq            r2, r0, mw_type_disallowed
    nop            // Branch Delay Slot

    beqi           r2, MR_TYPE_MW_TYPE_2, type2_mw_bind
type1_mw_bind:
    // type1 mw bind is allowed if current state is free or valid but
    // not if invalid
    seq            c1, d.state, KEY_STATE_INVALID
    bcf            [c1], invalid_mw_state

    seq            c1, K_ZBVA, 1 // Branch Delay Slot
    bcf            [c1], invalid_zbva
    nop            // Branch Delay Slot
    
    tblwr          d.type, MR_TYPE_MW_TYPE_1
    b              update_key
    // Type 1 MW cannot be invalidated by local/remote invalidate, hence do not
    // set MR_FLAG_INV
    tblwr          d.flags, MR_FLAG_UKEY_EN // Branch Delay Slot

type2_mw_bind:
    // type2 mw bind is allowed only if current state is free but not if 
    // valid or invalid
    seq            c1, d.state, KEY_STATE_FREE
    bcf            [!c1], invalid_mw_state

    // type 2 mw bind cannot be performed with zero length to unbind the MW
    seq            c1, K_LEN, 0 // Branch Delay Slot
    bcf            [c1], invalid_len
    nop            // Branch Delay Slot

    tblwr          d.type, MR_TYPE_MW_TYPE_2
    // Type2 MW can be invalidated by local/remote invalidate so set MR_FLAG_INV
    tblwr          d.flags, (MR_FLAG_INV_EN|MR_FLAG_UKEY_EN)

update_key:
    tblwr          d.state, KEY_STATE_VALID
    tblwr          d.user_key, K_USER_KEY 
    tblwr          d.acc_ctrl, K_ACC_CTRL
    tblwr          d.log_page_size, K_LOG_PAGE_SIZE
    tblwr          d.base_va, K_VA
    tblwr          d.pt_base, K_MW_PT_BASE
    tblwr          d.len, K_LEN
    tblwr          d.qp, K_GLOBAL_QID
    or             r1, r0, K_ZBVA, LOG_MR_FLAG_ZBVA
    tblor.e        d.flags, r1
    
    CAPRI_SET_TABLE_0_VALID(0)

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1], exit
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_256_BITS)

exit:
    nop.e
    nop

mw_type_disallowed:
invalid_mw_state:
invalid_len:
invalid_zbva:
    phvwrpair      p.rdma_feedback.feedback_type, RDMA_COMPLETION_FEEDBACK, \
                   p.{rdma_feedback.completion.status...rdma_feedback.completion.error}, \
                   (CQ_STATUS_MEM_MGMT_OPER_ERR << 1 | 1)
    phvwr.e          CAPRI_PHV_FIELD(phv_global_common, error_disable_qp),  1
    CAPRI_SET_TABLE_0_VALID(0)
