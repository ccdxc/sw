#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct sqcb0_t d;
//struct req_rx_s0_k_t k;
struct rdma_stage0_table_k k;

#define SQCB0_TO_SQCB1_T struct req_rx_sqcb0_to_sqcb1_info_t
#define SQCB0_TO_STAGE_T struct req_rx_to_stage_t

%%

    .param    req_rx_sqcb1_process

.align
req_rx_sqcb_process:

    // global fields
    add            r1, r0, offsetof(struct phv_, common_global_global_data)

    // qstate addr is available as instrinsic data
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(RQCB_ADDR_SHIFT))
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_RXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD_RANGE(r1, PHV_GLOBAL_COMMON_T, qid, qtype, CAPRI_RXDMA_INTRINSIC_QID_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, flags, CAPRI_APP_DATA_RAW_FLAGS)

    // set DMA CMD ptr
    RXDMA_DMA_CMD_PTR_SET(REQ_RX_DMA_CMD_START_FLIT_ID)

    // raw_flags
    add            r1, r0, CAPRI_APP_DATA_RAW_FLAGS
    // remaining_payload_bytes = p4_to_p4plus_rdma_hdr_p->payload_size
    add            r2, CAPRI_APP_DATA_PAYLOAD_LEN, r0
    
    // check for service type
    srl            r3, CAPRI_APP_DATA_BTH_OPCODE, BTH_OPC_SVC_SHIFT
    seq            c1, r3, d.service
    bcf            [!c1], invalid_serv_type


    ARE_ALL_FLAGS_SET(c3, r1, REQ_RX_FLAG_ACK)
    bcf            [!c3], atomic
    seq            c4, RRQ_P_INDEX, RRQ_C_INDEX // Branch Delay Slot

ack:
    // remaining_payload_bytes != 0
    bne            r2, r0, invalid_pyld_len
    nop
    b              next_stage_arg

atomic:
    ARE_ALL_FLAGS_SET(c1, r1, REQ_RX_FLAG_ATOMIC_ACK)
    bcf            [!c1], read
    nop

    // remaining_payload_bytes != 0 
    bne            r2, r0, invalid_pyld_len
    nop

    b              ring_not_empty

read:
    ARE_ALL_FLAGS_SET(c1, r1, REQ_RX_FLAG_READ_RESP)
    bcf            [!c1], invalid_opcode

    // remaining_payload_bytes != pmtu
    add            r3, r0, d.log_pmtu
    sllv           r3, 1, r3
    sne            c1, r3, r2
    IS_ANY_FLAG_SET(c2, r1, REQ_RX_FLAG_MIDDLE|REQ_RX_FLAG_FIRST)     
    bcf            [c1 & c2], invalid_pyld_len

    // remaining_payload_bytes > pmtu
    slt            c1, r3, r2 // Branch Delay Slot
    ARE_ALL_FLAGS_SET(c2, r1, REQ_RX_FLAG_ONLY)
    bcf            [c1 & c2], invalid_pyld_len

    // remaining_payload_bytes < 1
    slt            c1, r2, 1 // Branch Delay Slot
    ARE_ALL_FLAGS_SET(c2, r1, REQ_RX_FLAG_LAST)
    bcf            [c1 & c2], invalid_pyld_len

    phvwr          p.cqwqe.op_type, OP_TYPE_READ // Branch Delay Slot

ring_not_empty:
    bcf            [c4 & !c3], rrq_empty

    phvwr          p.cqwqe.qp, CAPRI_RXDMA_INTRINSIC_QID // Branch Delay Slot
    phvwr          p.cqwqe.status, CQ_STATUS_SUCCESS

next_stage_arg:

    CAPRI_GET_TABLE_0_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, remaining_payload_bytes, r2)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, rrq_cindex, RRQ_C_INDEX)
    cmov           r4, c4, 1, 0
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, rrq_empty, r4)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, dma_cmd_start_index, REQ_RX_RDMA_PAYLOAD_DMA_CMDS_START)
    CAPRI_SET_FIELD(r7, SQCB0_TO_SQCB1_T, need_credits, d.need_credits)

    CAPRI_GET_STAGE_1_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB0_TO_STAGE_T, msn, CAPRI_APP_DATA_AETH_MSN)
    CAPRI_SET_FIELD(r7, SQCB0_TO_STAGE_T, bth_psn, CAPRI_APP_DATA_BTH_PSN)
    CAPRI_SET_FIELD(r7, SQCB0_TO_STAGE_T, syndrome, CAPRI_APP_DATA_AETH_SYNDROME)

    CAPRI_GET_STAGE_2_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB0_TO_STAGE_T, msn, CAPRI_APP_DATA_AETH_MSN)
    CAPRI_SET_FIELD(r7, SQCB0_TO_STAGE_T, bth_psn, CAPRI_APP_DATA_BTH_PSN)
    CAPRI_SET_FIELD(r7, SQCB0_TO_STAGE_T, syndrome, CAPRI_APP_DATA_AETH_SYNDROME)
     

    CAPRI_GET_TABLE_0_K(req_rx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_rx_sqcb1_process)
    add            r1, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

    nop.e
    nop

invalid_pyld_len:
invalid_opcode:
rrq_empty:
invalid_serv_type:
    
    nop.e
    nop
