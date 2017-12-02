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
#define RQCB_TO_RQCB1_CNP_T struct resp_tx_rqcb_to_cnp_info_t
#define ACK_INFO_T struct resp_tx_rqcb_to_ack_info_t
#define BT_ADJUST_INFO_T struct resp_tx_rsq_backtrack_adjust_info_t 

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
    .param      resp_tx_ack_process
    .param      resp_tx_rsq_backtrack_adjust_process
    .param      resp_tx_rqcb1_cnp_process

resp_tx_rqcb_process:
    // copy intrinsic to global
    add            r1, r0, offsetof(struct phv_, common_global_global_data)

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

    // Process CNP packet ring first as its the highest priority.
    seq            c1, CNP_C_INDEX, CNP_P_INDEX
    bcf            [c1], check_backtrack_q

    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_CNP_T, new_c_index, CNP_P_INDEX)
    add         RQCB1_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, 1, LOG_CB_UNIT_SIZE_BYTES  #RQCB1 address
    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rqcb1_cnp_process)
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
    nop.e
    nop

check_rsq:
    // set DMA cmd ptr   (dma cmd idx with in flit is zero)
    TXDMA_DMA_CMD_PTR_SET(RESP_TX_DMA_CMD_START_FLIT_ID, 0)

    seq         c1, RSQ_C_INDEX, RSQ_P_INDEX
    bcf         [c1], check_ack_nak
    add         RQCB1_P, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES //BD Slot

rsq:
    add         RSQWQE_P, d.rsq_base_addr, RSQ_C_INDEX, LOG_SIZEOF_RSQWQE_T

    add         NEW_RSQ_C_INDEX, r0, RSQ_C_INDEX
    mincr       NEW_RSQ_C_INDEX, d.log_rsq_size, 1

    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, rsqwqe_addr, RSQWQE_P)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, serv_type, d.serv_type)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, log_pmtu, d.log_pmtu)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, new_rsq_c_index, NEW_RSQ_C_INDEX)

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
    CAPRI_SET_FIELD(r4, ACK_INFO_T, serv_type, d.serv_type)
    CAPRI_SET_FIELD(r4, ACK_INFO_T, new_c_index, ACK_NAK_P_INDEX)

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_ack_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB1_P)
    
exit:
    nop.e
    nop
