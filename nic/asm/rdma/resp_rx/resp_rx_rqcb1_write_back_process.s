#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
//struct resp_rx_rqcb1_write_back_process_k_t k;
struct resp_rx_s5_t2_k k;
struct rqcb1_t d;

#define IN_TO_S_P   to_s5_wb1_info
#define IN_P        t2_s2s_rqcb1_write_back_info

#define DMA_CMD_BASE r1
#define GLOBAL_FLAGS r2
#define TMP r3
#define DB_ADDR r4
#define DB_DATA r5
#define RQCB1_ADDR r6
#define T2_ARG r7

#define CQCB_ADDR r6
#define KT_BASE_ADDR r6
#define KEY_ADDR r6
#define NEW_RSQ_P_INDEX r6

#define K_CURR_WQE_PTR CAPRI_KEY_FIELD(IN_TO_S_P, curr_wqe_ptr)
#define K_ASYNC_EVENT_OR_ERROR CAPRI_KEY_FIELD(IN_TO_S_P, async_event_or_error)
#define K_INV_RKEY CAPRI_KEY_RANGE(IN_P, inv_r_key_sbit0_ebit15, inv_r_key_sbit24_ebit31)

%%
    .param  resp_rx_cqcb_process
    .param  resp_rx_stats_process
    .param  resp_rx_recirc_mpu_only_process
    .param  resp_rx_inv_rkey_process

.align
resp_rx_rqcb1_write_back_process:
    // check if qp is already in error disable state. if so, drop the phv instead of recirc
    seq             c2, d.state, QP_STATE_ERR
    bcf             [c2], phv_drop
    CAPRI_SET_TABLE_2_VALID(0)  //BD Slot

    bbeq            CAPRI_KEY_FIELD(IN_TO_S_P, feedback), 1, process_feedback
    seq             c1, CAPRI_KEY_FIELD(IN_TO_S_P, my_token_id), d.nxt_to_go_token_id //BD Slot

    bcf             [!c1], recirc
    crestore        [c2, c1], CAPRI_KEY_RANGE(IN_TO_S_P, incr_nxt_to_go_token_id, incr_c_index), 0x3 // BD Slot
    # c2 - incr_nxt_to_go_token_id, c1 - incr_c_index
    

    // check if we need to put QP to error disable state
    // if any of the previous states have encountered fatal error, they will
    // assert error_disable_qp flag to 1.

    bbeq            K_GLOBAL_FLAG(_error_disable_qp), 1, error_disable_qp
    crestore [c7, c6, c5, c4, c3], K_GLOBAL_FLAGS, \
            (RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA | RESP_RX_FLAG_READ_REQ | RESP_RX_FLAG_ONLY | RESP_RX_FLAG_LAST) //BD Slot
    // c7: cswap, c6: fna, c5: read, c4: only, c3: last

    tbladd.c2       d.nxt_to_go_token_id, 1

    // if NAK/RNR NAK in stage 0, we set soft_nak to 1
    // soft_nak means we don't need to move the qp to error disable state upon
    // encountering this soft error, but at the same time we shouldn't be 
    // incrementing msn etc. as it is not successfully completed yet.
    bbeq            CAPRI_KEY_FIELD(IN_TO_S_P, soft_nak), 1, invoke_stats
    // copy msn info for soft_nak case. 
    //TODO: avoid copying msn 2 times for fast path. optimize later, 
    phvwr           p.s1.ack_info.aeth.msn, d.msn   //BD Slot

    tblmincri.c1    PROXY_RQ_C_INDEX, d.log_num_wqes, 1

    // increment msn for successful atomic/read/last/only msgs
    setcf           c1, [c7 | c6 | c5 | c4 | c3]
    tblmincri.c1    d.msn, 24, 1

    // is it atomic req ?
    // added this instruction so that d.msn is not immediately used after
    // previous instruction update
    setcf           c1, [c7 | c6]

    // eventually we need to move the ack logic into writeback.
    // for now only msn update is moved here.
    phvwr           p.s1.ack_info.aeth.msn, d.msn

    bcf             [c4], skip_updates_for_only
    tblwr           d.in_progress, CAPRI_KEY_FIELD(IN_TO_S_P, in_progress)   //BD Slot

    // If atomic request, set busy to 0
    # c1: atomic
    tblwr.c1        d.busy, 0

    // updates for multi-packet case
    tblwr       d.current_sge_offset, CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit15, current_sge_offset_sbit16_ebit31)
    tblwr       d.current_sge_id, CAPRI_KEY_FIELD(IN_TO_S_P, current_sge_id)
    crestore    [c3, c2], CAPRI_KEY_RANGE(IN_TO_S_P, update_wqe_ptr, update_num_sges), 0x3
    #c3 - update_wqe_ptr
    #c2 - update_num_sges
    tblwr.c3    d.curr_wqe_ptr, K_CURR_WQE_PTR
    tblwr.c2    d.num_sges, CAPRI_KEY_FIELD(IN_TO_S_P, num_sges)

    
skip_updates_for_only:
check_completion:
    
    // load cqcb only if completion flag is set OR async_event_or_error posting is needed
    crestore [c1], K_ASYNC_EVENT_OR_ERROR, 0x1
    bbeq.!c1 K_GLOBAL_FLAG(_completion), 0, invoke_stats
    RESP_RX_CQCB_ADDR_GET(CQCB_ADDR, d.cq_id) //BD Slot

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_cqcb_process, CQCB_ADDR)

invoke_stats:
    // if inv_rkey flag is set, invoke inv_rkey_process 
    // by loading appopriate key entry
    bbne    CAPRI_KEY_FIELD(IN_TO_S_P, inv_rkey), 1, skip_inv_rkey
    KT_BASE_ADDR_GET2(KT_BASE_ADDR, TMP) // BD slot
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, K_INV_RKEY)

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_256_BITS, resp_rx_inv_rkey_process, KEY_ADDR)

skip_inv_rkey:
    // invoke stats as mpu only
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_stats_process, r0)

recirc:
    phvwr   p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_INORDER_WORK_DONE   //BD Slot
    phvwr   p.common.p4_intr_recirc, 1  

    // fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_recirc_mpu_only_process, r0)

phv_drop:
    phvwr.e     p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)  //Exit slot

error_disable_qp:
    // move the state to error and then check for completion.
    // unless an error can be clearly associated with a receive WQE, we 
    // are not supposed to generate cqwqe. This is controlled by completion
    // flag.
    // TODO: we need to handle affiliated and unaffiliated async errors as well

    // eventually we need to move the nak logic into writeback.
    // for now only msn update is moved here.
    phvwr       p.s1.ack_info.aeth.msn, d.msn
    // fall thru

// currently there is only one type of feedback phv which takes qp to error disable.
process_feedback:
    b           check_completion
    tblwr       d.state, QP_STATE_ERR   //BD Slot

