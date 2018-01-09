#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct rqcb0_t d;
struct rdma_stage0_table_k k;

#define RQCB_TO_RQCB1_T struct resp_tx_rqcb_to_rqcb1_info_t
#define ACK_INFO_T struct resp_tx_rqcb_to_ack_info_t
#define BT_ADJUST_INFO_T struct resp_tx_rsq_backtrack_adjust_info_t 
#define TO_STAGE_T struct resp_tx_to_stage_t

#define RSQWQE_P            r1
#define RQCB1_P             r2
#define RAW_TABLE_PC        r3
#define NEW_RSQ_C_INDEX     r5

//r7 is pre-loaded with Qstate-ring-not-empty 8-bit flags, one for each ring
#define RING_NOT_EMPTY r7 

#define DB_ADDR             r2
#define DB_DATA             r3

%%
    .param      resp_tx_rqcb1_process
    .param      resp_tx_rsq_backtrack_adjust_process
    .param      resp_tx_rqcb1_dcqcn_rate_process
    .param      resp_tx_ack_process

resp_tx_rqcb_process:
    // are all rings empty ?
    seq            c1, r7[MAX_RQ_RINGS-1:0], r0
    bcf            [c1], all_rings_empty

    // copy intrinsic to global
    add            r1, r0, offsetof(struct phv_, common_global_global_data) //BD Slot

    // reset ring_empty_counter
    tblwr          d.ring_empty_counter, 0

    // enable below code after spr_tbladdr special purpose register is available in capsim
    #mfspr         r1, spr_tbladdr
    #srl           r1, r1, SQCB_ADDR_SHIFT
    #CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, cb_addr, r1)
    add            r2, r0, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    srl            r2, r2, RQCB_ADDR_SHIFT

    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, r2)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)

    CAPRI_GET_TABLE_0_ARG(resp_tx_phv_t, r4)
    //TODO: migrate to efficient way of demuxing work (based on r7)

    // Process DCQCN algo ring first as its the highest priority.
    seq         c1, DCQCN_RATE_COMPUTE_C_INDEX, DCQCN_RATE_COMPUTE_P_INDEX
    bcf         [c1], check_dcqcn_timer_q
    nop

    // Increment c-index and pass to stage 4
    add         r2, r0, DCQCN_RATE_COMPUTE_C_INDEX
    mincr       r2, 16, 1 // c_index is 16 bit
    // Increment timer-c-index and pass to stage 4. This is used to stop dcqcn-timer on reaching max-qp-rate.
    add         r3, r0, DCQCN_TIMER_C_INDEX
    mincr       r3, 16, 1 // c_index is 16 bit

    CAPRI_GET_STAGE_4_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s4.dcqcn.new_cindex, r2)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s4.dcqcn.new_timer_cindex, r3)


    add         RQCB1_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, 1, LOG_CB_UNIT_SIZE_BYTES  #RQCB1 address
    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rqcb1_dcqcn_rate_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB1_P)
    
    nop.e
    nop

check_dcqcn_timer_q:
    seq         c1, DCQCN_TIMER_C_INDEX, DCQCN_TIMER_P_INDEX
    bcf         [c1], check_backtrack_q
    nop

    // Increment c-index and pass to stage 4
    add         r2, r0, DCQCN_TIMER_C_INDEX
    mincr       r2, 16, 1 // c_index is 16 bit field
    CAPRI_GET_STAGE_4_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s4.dcqcn.new_cindex, r2)

    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, timer_event_process, 1)
    add         RQCB1_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, 1, LOG_CB_UNIT_SIZE_BYTES  #RQCB1 address
    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rqcb1_dcqcn_rate_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB1_P)

    nop.e
    nop

check_backtrack_q:
    seq         c1, RSQ_BT_C_INDEX, RSQ_BT_P_INDEX
    seq         c2, d.adjust_rsq_c_index_in_progress, 0

    // if backtrack ring is invoked, but adjust_rsq_c_index_in_progress is not set
    // (which is set in RXDMA), then ignore backtrack ring and check for other work
    // It is possible that backtrack ring was already invoked which set
    // adjust_rsq_c_index_in_progress to 0, but might not have made backtrack_c_index     // equal to backtrack_p_index yet (due to DMA delays) and scheduler gave one
    // more opportunity. 
    bcf         [c1 | c2], check_rq
    tblwr       d.adjust_rsq_c_index_in_progress, 0

backtrack_q:
    add         RQCB1_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES //BD Slot
    CAPRI_SET_FIELD(r4, BT_ADJUST_INFO_T, adjust_rsq_c_index, d.adjust_rsq_c_index)
    CAPRI_SET_FIELD(r4, BT_ADJUST_INFO_T, rsq_bt_p_index, RSQ_BT_P_INDEX)

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rsq_backtrack_adjust_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB1_P)

    nop.e
    nop

check_rq:
    seq         c1, RQ_C_INDEX, RQ_P_INDEX
    bcf         [c1], check_rsq
    nop         //BD Slot
    
rq:
    //TODO: to stop scheduler ringing for ever, artificially move c_index to p_index. 
    //      proxy_c_index would track the real c_index
    DOORBELL_WRITE_CINDEX(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, RQ_RING_ID, RQ_P_INDEX, DB_ADDR, DB_DATA) 
    phvwr.e   p.common.p4_intr_global_drop, 1
    nop //Exit Slot

check_rsq:
    // Pass congestion_mgmt_enable flag to stages 3 and 4.
    CAPRI_GET_STAGE_3_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s3.rsq_rkey.congestion_mgmt_enable, d.congestion_mgmt_enable)
    CAPRI_GET_STAGE_4_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s4.dcqcn.congestion_mgmt_enable, d.congestion_mgmt_enable)

    // set DMA cmd ptr   (dma cmd idx with in flit is zero)
    TXDMA_DMA_CMD_PTR_SET(RESP_TX_DMA_CMD_START_FLIT_ID, 0)

    seq         c1, RSQ_C_INDEX, RSQ_P_INDEX
    bcf         [c1], check_ack_nak
    add         RQCB1_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES //BD Slot

rsq:
    sll         RSQWQE_P, d.rsq_base_addr, RSQ_BASE_ADDR_SHIFT
    add         RSQWQE_P, RSQWQE_P, RSQ_C_INDEX, LOG_SIZEOF_RSQWQE_T

    add         NEW_RSQ_C_INDEX, r0, RSQ_C_INDEX
    mincr       NEW_RSQ_C_INDEX, d.log_rsq_size, 1

    // send NEW_RSQ_C_INDEX to stage 5 (writeback)
    CAPRI_GET_STAGE_5_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s5.rqcb1_wb.new_c_index, NEW_RSQ_C_INDEX)

    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, rsqwqe_addr, RSQWQE_P)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, serv_type, d.serv_type)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, log_pmtu, d.log_pmtu)

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rqcb1_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB1_P)

    nop.e
    nop

check_ack_nak:
    seq         c1, ACK_NAK_C_INDEX, ACK_NAK_P_INDEX
    bcf         [c1], exit
    nop

ack_nak:
    // overwrite ACK_NAK_C_INDEX with ACK_NAK_P_INDEX
    tblwr   ACK_NAK_C_INDEX, ACK_NAK_P_INDEX

    // Pass congestion_mgmt flag to stage 3 dcqcn-mpu-only. This is used to decide whether to load dcqcn_cb in stage 4 or mpu-only-dcqcn.
    CAPRI_GET_STAGE_3_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s3.dcqcn.congestion_mgmt_enable, d.congestion_mgmt_enable)

    // send new_c_index,serv_type and ack processing flag to stage 5 (writeback)
    CAPRI_GET_STAGE_5_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s5.rqcb1_wb.ack_nak_process, 1)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s5.rqcb1_wb.ack_nack_serv_type, d.serv_type)

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_ack_process) 
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB1_P)
    
exit:
    nop.e
    nop

all_rings_empty:
    seq     c1, d.ring_empty_counter, 0
    bcf     [!c1], skip_doorbell_eval
    nop     //BD Slot

    // ring doorbell to re-evaluate scheduler
    DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r2, r3)

skip_doorbell_eval:
    phvwr.e     p.common.p4_intr_global_drop, 1
    tblmincri   d.ring_empty_counter, 8, 1  //Exit Slot
