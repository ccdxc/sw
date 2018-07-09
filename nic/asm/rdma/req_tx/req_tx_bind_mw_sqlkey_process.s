#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s4_t0_k k;
struct key_entry_aligned_t d;

#define IN_P t0_s2s_sqwqe_to_lkey_mw_info
#define IN_TO_S_P  to_s4_sq_to_stage

#define K_ACC_CTRL CAPRI_KEY_FIELD(IN_P, acc_ctrl)
#define K_VA       CAPRI_KEY_RANGE(IN_P, va_sbit0_ebit7, va_sbit56_ebit63)
#define K_LEN      CAPRI_KEY_RANGE(IN_P, len_sbit0_ebit7, len_sbit24_ebit31)
#define K_ZBVA     CAPRI_KEY_FIELD(IN_P, zbva)
#define K_R_KEY    CAPRI_KEY_RANGE(IN_P, r_key_sbit0_ebit7, r_key_sbit24_ebit31)
#define K_MW_TYPE  CAPRI_KEY_FIELD(IN_P, mw_type) 
#define K_HEADER_TEMPLATE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, header_template_addr_sbit0_ebit7, header_template_addr_sbit24_ebit31)

#define SQLKEY_TO_RKEY_MW_INFO_P t0_s2s_sqlkey_to_rkey_mw_info
#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info


%%
    .param    req_tx_bind_mw_rkey_process
    .param    req_tx_dcqcn_enforce_process

.align
req_tx_bind_mw_sqlkey_process:
    // Pin bind_mw_sqlkey to stage 4, same as sqlkey_process so that
    // key_entry can be locked and updated.
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_4
    bcf            [!c1], bubble_to_next_stage

    // if memory region is not in valid state or doesn't allow memory window or
    // is zero based virtual address region, then do not allow memory window
    // binding to this region
    sne            c1, d.state, KEY_STATE_VALID // Branch Delay Slot
    ARE_ALL_FLAGS_SET_B(c2, d.flags, MR_FLAG_MW_EN)
    ARE_ALL_FLAGS_SET_B(c3, d.flags, MR_FLAG_ZBVA)
    bcf            [c3 | !c2 | c1], invalid_mr

    // if memory region doesn't have local write permission, do not allow
    // binding of memory window with access permission remote write/atomic
    IS_ANY_FLAG_SET_B(c1, K_ACC_CTRL, ACC_CTRL_REMOTE_WRITE|ACC_CTRL_REMOTE_ATOMIC) // Branch Delay Slot
    ARE_ALL_FLAGS_SET_B(c2, d.acc_ctrl, ACC_CTRL_LOCAL_WRITE)
    bcf            [c1 & !c2], invalid_acc_ctrl
    
    // if VA based, base_va of memory window is va passed in bind_mw wqe.
    // if ZBVA, base_va is MR's base_va + offset passed in bind_mw wqe
    // r1 = MW base_va
    seq            c1, K_ZBVA, 1 // Branch Delay Slot
    add.c1         r1, d.base_va, K_VA
    add.!c1        r1, K_VA, r0

    // check if memory window is subset of memory region
    //  ((MW base_va >= MR base_va) and
    //   (MW base_va + MW len) <= (MR base_va + MR len))
    add            r2, d.len, d.base_va
    slt            c2, r1, d.base_va
    sslt           c3, r2, r1, K_LEN
    bcf            [c2 | c3], invalid_va

    // log_pt_seg_size = log_pt_num_entries_per_Cacheline + log_page_size
    add            r4, LOG_HBM_NUM_PT_ENTRIES_PER_CACHELINE, d.log_page_size //Branch Delay Slot

    // pt_seg_offset = ((MW base_va - MR base_va) +
    //                  (MR base_va % pt_seg_size) -
    //                  (MW base_va % pt_seg_size)) >> log_page_size
    sub            r2, r1, d.base_va
    add            r3, d.base_va, r0
    mincr          r3, r4, r0
    mincr          r1, r4, r0
    add            r2, r2, r3
    sub            r2, r2, r1
    srl            r2, r2, d.log_page_size

    // mw_pt_base = MW pt_base + pt_seg_offset
    add            r2, d.pt_base, r2

    // Reuse parameters passed from sqwqe stage and overwrite only those
    // needed by bind_mw_rkey_process
    phvwrpair  CAPRI_PHV_FIELD(SQLKEY_TO_RKEY_MW_INFO_P, mw_pt_base), r2, \
               CAPRI_PHV_FIELD(SQLKEY_TO_RKEY_MW_INFO_P, log_page_size), d.log_page_size          

    KT_BASE_ADDR_GET2(r1, r2)
    add            r2, K_R_KEY, r0
    KEY_ENTRY_ADDR_GET(r1, r1, r2)

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_PC(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_bind_mw_rkey_process, r1)

    nop.e
    nop

invalid_mr:
invalid_acc_ctrl:
invalid_va:
    phvwrpair      p.rdma_feedback.feedback_type, RDMA_COMPLETION_FEEDBACK, \
                   p.{rdma_feedback.completion.status...rdma_feedback.completion.error}, \
                   (CQ_STATUS_MEM_MGMT_OPER_ERR << 1 | 1)

    phvwr.e        CAPRI_PHV_FIELD(phv_global_common, error_disable_qp),  1
    CAPRI_SET_TABLE_0_VALID(0)    

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1], exit
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_256_BITS) 

exit:
    nop.e
    nop
