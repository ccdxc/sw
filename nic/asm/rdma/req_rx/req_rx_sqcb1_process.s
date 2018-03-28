#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct sqcb1_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define SQCB1_TO_RRQWQE_P t0_s2s_sqcb1_to_rrqwqe_info
#define ECN_INFO_P t3_s2s_ecn_info

%%

    .param    req_rx_rrqwqe_process
    .param    req_rx_cqcb_process
    .param    req_rx_dcqcn_ecn_process

.align
req_rx_sqcb1_process:

    // copy cur_timestamp loaded in r4 into phv to DMA ack_timestamp
    // into sqcb2 for valid aeth packet
    phvwr          p.ack_timestamp, r4

    // Check pending_recirc_pkts_max
    sub            r1, d.token_id, d.nxt_to_go_token_id
    mincr          r1, 8, r0
    bgti           r1, PENDING_RECIR_PKTS_MAX, recirc_cnt_exceed

    // get token_id for this packet
    phvwr          p.my_token_id, d.token_id // Branch Delay Slot
    
    // check if its this packet's turn, if not recirc
    seq            c1, d.token_id, d.nxt_to_go_token_id
    bcf            [!c1], recirc
    tbladd         d.token_id, 1 // Branch Delay Slot

    //Check if ECN bits are set in Packet and congestion management is enabled.                      
    sne            c5, k.rdma_bth_ecn, 3
    sne            c6, d.congestion_mgmt_enable, 1
    bcf            [c5 | c6], process_rx_pkt

    // Load dcqcn_cb to store timestamps and trigger Doorbell to generate CNP.
    CAPRI_RESET_TABLE_3_ARG()
    CAPRI_SET_FIELD2(ECN_INFO_P, p_key, CAPRI_APP_DATA_BTH_P_KEY)

    add     r1, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT //dcqcn_cb addr
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_dcqcn_ecn_process, r1)

process_rx_pkt:
    add            r1, r0, CAPRI_APP_DATA_RAW_FLAGS
    beqi           r1, REQ_RX_FLAG_RDMA_FEEDBACK, process_feedback
    // Get SQCB2 base address 
    add            r7, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES*2) // Branch Delay Slot

    // initialize cqwqe 
    // Initialize cqwqe to success initially
    phvwrpair      p.cqwqe.status, CQ_STATUS_SUCCESS, p.cqwqe.qp, CAPRI_RXDMA_INTRINSIC_QID // Branch Delay Slot

    // remaining_payload_bytes = p4_to_p4plus_rdma_hdr_p->payload_size
    add            r2, CAPRI_APP_DATA_PAYLOAD_LEN, r0

    // check for service type
    srl            r3, CAPRI_APP_DATA_BTH_OPCODE, BTH_OPC_SVC_SHIFT
    seq            c1, r3, d.service
    bcf            [!c1], invalid_serv_type

    ARE_ALL_FLAGS_SET(c3, r1, REQ_RX_FLAG_ACK) // Branch Delay Slot
    bcf            [!c3], atomic 
    seq            c4, RRQ_P_INDEX, RRQ_C_INDEX // Branch Delay Slot

ack:
    // remaining_payload_bytes != 0
    bne            r2, r0, invalid_pyld_len
    nop            // Branch Delay Slot
    b              check_psn

atomic:
    ARE_ALL_FLAGS_SET(c1, r1, REQ_RX_FLAG_ATOMIC_AETH)
    bcf            [!c1], read
    nop            // Branch Delay Slot

    // remaining_payload_bytes != 0 
    bne            r2, r0, invalid_pyld_len
    nop

    bcf            [c4], rrq_empty
    nop            // Branch Delay Slot

    b              check_psn
    nop            // Branch Delay Slot

read:
    ARE_ALL_FLAGS_SET(c1, r1, REQ_RX_FLAG_READ_RESP)
    bcf            [!c1], invalid_opcode

    // remaining_payload_bytes != pmtu
    sll            r3, 1, d.log_pmtu
    sne            c1, r3, r2
    IS_ANY_FLAG_SET(c2, r1, REQ_RX_FLAG_MIDDLE|REQ_RX_FLAG_FIRST)
    bcf            [c1 & c2], invalid_pyld_len

    // remaining_payload_bytes > pmtu
    slt            c1, r3, r2 // Branch Delay Slot
    IS_ANY_FLAG_SET(c2, r1, REQ_RX_FLAG_ONLY|REQ_RX_FLAG_LAST)
    bcf            [c1 & c2], invalid_pyld_len

    // remaining_payload_bytes < 1
    slt            c1, r2, 1 // Branch Delay Slot
    ARE_ALL_FLAGS_SET(c2, r1, REQ_RX_FLAG_LAST)
    bcf            [c1 & c2], invalid_pyld_len

    phvwr          p.cqwqe.op_type, OP_TYPE_READ // Branch Delay Slot

    bcf            [c4], rrq_empty
    nop            // Branch Delay Slot

check_psn:

    // TODO Check valid PSN

    // bth.psn >= sqcb1_p->rexmit_psn, valid response
    scwlt24        c1, CAPRI_APP_DATA_BTH_PSN, d.rexmit_psn

    ARE_ALL_FLAGS_SET(c6, r1, REQ_RX_FLAG_AETH)
    bcf            [!c6 & c1], duplicate_read_resp_mid
    // rexmit_psn is resp psn + 1
    add            r5, CAPRI_APP_DATA_BTH_PSN, 1 // Branch Delay Slot

    // skip ack sanity checks if there is no aeth hdr
    bcf            [!c6], post_rexmit_psn
    //phvwr        p.rexmit_psn, r1 // Branch Delay Slot
    //tblwr.!c2      d.rexmit_psn, r1 // Branch Delay Slot
    mincr          r5, 24, 0
 
check_ack_sanity:
    // if (msn >= sqcb1_p->ssn) invalid_pkt_msn
    scwle24        c3, d.ssn, CAPRI_APP_DATA_AETH_MSN
    bcf            [c3], invalid_pkt_msn

    add            r3, CAPRI_APP_DATA_AETH_SYNDROME, r0

    ARE_ALL_FLAGS_SET(c2, r1, REQ_RX_FLAG_ACK) // Branch Delay Slot
    bcf            [!c1], process_aeth

    IS_MASKED_VAL_EQUAL_B(c5, r3, SYNDROME_MASK, ACK_SYNDROME) // Branch Delay Slot

    // bth.psn < sqcb1_p->rexmit_psn, duplicate and not unsolicited p_ack, drop
    bcf            [!c2 | !c5], duplicate_ack

    // unsolicited ack i.e. duplicate of most recent p_ack is allowed
    sub            r4, d.rexmit_psn, -1  // Branch Delay Slot
    mincr          r4, 24, r0
    seq            c3, r4, CAPRI_APP_DATA_BTH_PSN
    bcf            [!c3], duplicate_ack

process_aeth:
    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_MSN_CREDITS)

    bcf            [!c5], post_rexmit_psn
    //phvwr.!c5      p.rexmit_psn, CAPRI_APP_DATA_BTH_PSN // Branch Delay Slot
    //tblwr          d.rexmit_psn, CAPRI_APP_DATA_BTH_PSN // Branch Delay Slot
    add.!c5        r5, r0, CAPRI_APP_DATA_BTH_PSN // Branch Delay Slot

    //tblmincri      d.rexmit_psn, 24, 1
    // pass last completed msn to avoid posting completion again
    add            r3, d.msn, 0 
    tblwr          d.msn, CAPRI_APP_DATA_AETH_MSN
    tblwr          d.credits, CAPRI_APP_DATA_AETH_SYNDROME[4:0]

    // if (sqcb1_p->lsn != ((1 << (sqcb1_p->credits >> 1)) + sqcb1_p->msn))
    //     doorbell_incr_pindex(fc_ring_id) 
    DECODE_ACK_SYNDROME_CREDITS(r2, CAPRI_APP_DATA_AETH_SYNDROME, c1)
    mincr          r2, 24, CAPRI_APP_DATA_AETH_MSN
    sne            c1, d.lsn, r2

post_credits:
    // dma_cmd - msn and credits
    add            r4, r7, SQCB2_MSN_OFFSET
    DMA_HBM_PHV2MEM_SETUP(r6, msn, credits, r4)
    bcf            [!c1], post_rexmit_psn
    phvwrpair      p.msn, CAPRI_APP_DATA_AETH_MSN, p.credits, CAPRI_APP_DATA_AETH_SYNDROME[4:0]

    // dma_cmd - fc_ring db data
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_FC_DB)
    PREPARE_DOORBELL_INC_PINDEX(CAPRI_RXDMA_INTRINSIC_LIF,
                                CAPRI_RXDMA_INTRINSIC_QTYPE,
                                CAPRI_RXDMA_INTRINSIC_QID,
                                FC_RING_ID, r1, r2)
    phvwr          p.db_data1, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, db_data1, db_data1, r1)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)

post_rexmit_psn:
    bcf            [c3], unsolicited_ack
    phvwr          p.rexmit_psn, r5 // Branch Delay Slot
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_REXMIT_PSN)
    bcf            [!c6], dma_rexmit_psn_only
    add            r4, r7, SQCB2_REXMIT_PSN_OFFSET
    // if valid ack, update rexmit_psn as well as ack timestamp in sqcb2
    DMA_HBM_PHV2MEM_SETUP(r6, rexmit_psn, ack_timestamp, r4)
    b              set_arg
    nop            // Branch Delay Slot

dma_rexmit_psn_only:
    // if read_resp_mid, update only rexmit_psn and not ack timestamp
    DMA_HBM_PHV2MEM_SETUP(r6, rexmit_psn, rexmit_psn, r4)

set_arg:

    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, cur_sge_offset, d.rrqwqe_cur_sge_offset)
    CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, cur_sge_id, d.rrqwqe_cur_sge_id)
    CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, rrq_in_progress, d.rrq_in_progress)
    CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, cq_id, d.cq_id)
    CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, e_rsp_psn, d.e_rsp_psn)
    CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, msn, r3)
    CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, rexmit_psn, r5)
    CAPRI_SET_FIELD2_C(SQCB1_TO_RRQWQE_P, rrq_empty, 1, c4)
    //CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, timer_active, d.timer_active)
    CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, dma_cmd_start_index, REQ_RX_RDMA_PAYLOAD_DMA_CMDS_START)
    add            r2, RRQ_C_INDEX, r0
    mincr          r2, d.log_rrq_size, 1
    CAPRI_SET_FIELD2(SQCB1_TO_RRQWQE_P, rrq_cindex, r2)

    sll            r5, d.rrq_base_addr, RRQ_BASE_ADDR_SHIFT
    add            r5, r5, RRQ_C_INDEX, LOG_RRQ_WQE_SIZE
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqwqe_process, r5)

    nop.e
    nop

unsolicited_ack:
    // if its unsolicted ack, just post credits, msn and exit, CQ posting not needed
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)
    CAPRI_SET_TABLE_0_VALID(0)
    tblsub         d.token_id, 1
    tblmincri      d.token_id, SIZEOF_TOKEN_ID_BITS, 0 
    nop.e
    nop

duplicate_read_resp_mid:
duplicate_ack:
recirc_cnt_exceed:
recirc:
invalid_pkt_msn:
invalid_serv_type:
invalid_pyld_len:
invalid_opcode:
rrq_empty:
exit:
    CAPRI_SET_TABLE_0_VALID(0)
    tblsub        d.token_id, 1
    tblmincri     d.token_id, SIZEOF_TOKEN_ID_BITS, 0
    phvwr         p.common.p4_intr_global_drop, 1

    nop.e
    nop

process_feedback:
    seq            c1, k.rdma_ud_feedback_feedback_type, RDMA_UD_FEEDBACK
    bcf            [!c1], exit

ud_feedback:
    phvwr          p.cqwqe.op_type, k.rdma_ud_feedback_optype // Branch Delay Slot
    RDMA_UD_FEEDBACK_WRID(r7)
    phvwrpair      p.cqwqe.id.wrid, r7, p.cqwqe.status, k.rdma_ud_feedback_status

    // No writeback, hence decrement token_id
    tblsub         d.token_id, 1
    tblmincri      d.token_id, SIZEOF_TOKEN_ID_BITS, 0 

    CAPRI_RESET_TABLE_2_ARG()

    CAPRI_SET_TABLE_0_VALID(0)

    //REQ_RX_CQCB_ADDR_GET(r1, d.cq_id) TODO CQCB needs to be pushed to stage 5
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, req_rx_cqcb_process, r1)

    nop.e
    nop
