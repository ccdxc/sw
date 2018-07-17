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

%%
    .param  resp_rx_cqcb_process
    .param  resp_rx_stats_process
    .param  resp_rx_recirc_mpu_only_process

.align
resp_rx_rqcb1_write_back_process:
    //seq             c1, k.to_stage.s3.wb1.my_token_id, d.nxt_to_go_token_id
    seq             c1, CAPRI_KEY_FIELD(IN_TO_S_P, my_token_id), d.nxt_to_go_token_id
    bcf             [!c1], recirc
    crestore        [c3, c2], CAPRI_KEY_RANGE(IN_TO_S_P, incr_nxt_to_go_token_id, incr_c_index), 0x3 // BD Slot
    # c3 - incr_nxt_to_go_token_id, c2 - incr_c_index
    tbladd.c3       d.nxt_to_go_token_id, 1 

    // if NAK/RNR NAK in stage 0, we set skip_completion to 1
    bbeq            CAPRI_KEY_FIELD(IN_TO_S_P, skip_completion), 1, invoke_stats
    CAPRI_SET_TABLE_2_VALID(0) // BD Slot

    tblmincri.c2    PROXY_RQ_C_INDEX, d.log_num_wqes, 1 // Exit Slot
    bbeq            K_GLOBAL_FLAG(_only), 1, skip_updates_for_only
    tblwr           d.in_progress, CAPRI_KEY_FIELD(IN_TO_S_P, in_progress)   //BD Slot

    // If atomic request, set busy to 0
    add             GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    crestore        [c3, c2], GLOBAL_FLAGS, (RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA)
    # c3: cswap, c2: fna
    setcf           c1, [c3 | c2]
    # c1: atomic
    tblwr.c1        d.busy, 0

    // updates for multi-packet case
    tblwr       d.current_sge_offset, CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit15, current_sge_offset_sbit16_ebit31)
    tblwr       d.current_sge_id, CAPRI_KEY_RANGE(IN_TO_S_P, current_sge_id_sbit0_ebit6, current_sge_id_sbit7_ebit7)
    crestore    [c3, c2], CAPRI_KEY_RANGE(IN_TO_S_P, update_wqe_ptr, update_num_sges), 0x3
    #c3 - update_wqe_ptr
    #c2 - update_num_sges
    tblwr.c3    d.curr_wqe_ptr, K_CURR_WQE_PTR
    tblwr.c2    d.num_sges, CAPRI_KEY_RANGE(IN_TO_S_P, num_sges_sbit0_ebit6, num_sges_sbit7_ebit7)

    
skip_updates_for_only:
    
    // load cqcb only if completion flag is set
    bbeq    K_GLOBAL_FLAG(_completion), 0, invoke_stats
    CAPRI_SET_TABLE_2_VALID(0) // BD Slot

    RESP_RX_CQCB_ADDR_GET(CQCB_ADDR, d.cq_id) 
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_cqcb_process, CQCB_ADDR)

invoke_stats:
    // invoke stats as mpu only
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_stats_process, r0)

recirc:
    // fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_recirc_mpu_only_process, r0)

    phvwr   p.common.p4_intr_recirc, 1
    phvwr   p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_INORDER_WORK_DONE
    // invalidate table ?
    CAPRI_SET_TABLE_2_VALID(0)

exit:
    nop.e
    nop
