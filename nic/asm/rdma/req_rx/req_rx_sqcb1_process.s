#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct sqcb1_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define SQCB1_TO_RRQWQE_P t0_s2s_sqcb1_to_rrqwqe_info
#define ECN_INFO_P t3_s2s_ecn_info
#define RRQWQE_TO_CQ_P t2_s2s_rrqwqe_to_cq_info
#define SQCB1_TO_TIMER_EXPIRY_P t3_s2s_sqcb1_to_timer_expiry_info
#define SQCB1_WRITE_BACK_P t3_s2s_sqcb1_write_back_info

#define TO_S3_P to_s3_to_stage

#define TOKEN_ID r6

%%

    .param    req_rx_rrqwqe_process
    .param    req_rx_cqcb_process
    .param    req_rx_dcqcn_ecn_process
    .param    req_rx_recirc_mpu_only_process
    .param    req_rx_timer_expiry_process
    .param    req_rx_dummy_sqcb1_write_back_process

.align
req_rx_sqcb1_process:

    add            r1, r0, CAPRI_APP_DATA_RAW_FLAGS 

    // is this a new packet or recirc packet
    seq            c1, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    bcf            [!c1], recirc_pkt
    cmov           TOKEN_ID, c1, d.token_id, CAPRI_APP_DATA_RECIRC_TOKEN_ID // BD Slot

    // Do not check and increment token_id for feedback phv unlike
    // response packets. Feedback phvs can be processed out of order from
    // response packets and can be completed in 1 pass. Hence there is no
    // need to allocate token id and recirc these phvs 
    beqi           r1, REQ_RX_FLAG_RDMA_FEEDBACK, process_feedback
    // Initialize cqe to success initially
    phvwrpair      p.cqe.status, CQ_STATUS_SUCCESS, p.cqe.qid, CAPRI_RXDMA_INTRINSIC_QID // Branch Delay Slot

    // If bktrack is in progress do not process any response packets to
    // avoid updating CB state while bktrack logic is updating the same
    bbeq            d.bktrack_in_progress[0], 1, drop_packet

    // copy cur_timestamp loaded in r4 into phv to DMA ack_timestamp
    // into sqcb2 for valid aeth packet
    phvwr          p.ack_timestamp, r4 // Branch Delay slot

    // get token_id for this packet
    phvwr          p.common.rdma_recirc_token_id, TOKEN_ID
    phvwr          CAPRI_PHV_FIELD(TO_S3_P, my_token_id), TOKEN_ID

    // recirc if work_not_done_recirc_cnt != 0
    seq            c2, d.work_not_done_recirc_cnt, 0 
    bcf            [!c2], recirc_for_turn

    tbladd         d.token_id, 1 // Branch Delay Slot

process_recirc_work_not_done:
    crestore    [c3, c2], r1, (REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_ONLY) 
    setcf       c7, [c3 & !c2]

    // skip_token_id_check if fresh packet except Read-resp FML, OR 
    // recirc packet with reason other than work_not_done.
    // recirc packets with any other reason do not come here
    bcf         [c1 & !c7], skip_token_id_check  // c1 is set initially if (recirc_cnt = 0)

token_id_check:
    // Slow path: token id check is mandatory if fresh packet with Read-resp FML, OR
    // recirc packet with reason work_not_done
    seq         c2, TOKEN_ID, d.nxt_to_go_token_id // BD slot
    bcf         [!c2], recirc_for_turn

skip_token_id_check:
    //Check if ECN bits are set in Packet and congestion management is enabled.                      
    sne            c5, k.rdma_bth_ecn, 3  // BD-Slot
    sne            c6, d.congestion_mgmt_enable, 1
    bcf            [c5 | c6], process_rx_pkt

    // Load dcqcn_cb to store timestamps and trigger Doorbell to generate CNP.
    add     r2, AH_ENTRY_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT //dcqcn_cb addr //BD Slot
    CAPRI_RESET_TABLE_3_ARG()
    phvwr   CAPRI_PHV_FIELD(ECN_INFO_P, p_key), CAPRI_APP_DATA_BTH_P_KEY

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_dcqcn_ecn_process, r2)

process_rx_pkt:
    // Get SQCB2 base address 
    add            r7, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES*2) 

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

    nop
    //phvwr          p.cqe.op_type, OP_TYPE_READ // Branch Delay Slot

    bcf            [c4], rrq_empty
    nop            // Branch Delay Slot

check_psn:
    // Update max_tx_psn/ssn to the maximum forward progress
    // that has been made so that response to retried requests 
    // that have PSN logically greater than PSN of the retried
    // request can be accepted. This would happen if receiver
    // receives all the request but acks sent by receiver
    // was not received by the requester
    scwlt24        c1, d.max_tx_psn, d.tx_psn
    tblwr.c1       d.max_tx_psn, d.tx_psn
     
    scwlt24        c1, d.max_ssn, d.ssn
    tblwr.c1       d.max_ssn, d.ssn

    // TODO Check valid PSN

check_duplicate_read_resp_mid:
    // bth.psn >= sqcb1_p->rexmit_psn, valid response
    scwlt24        c1, CAPRI_APP_DATA_BTH_PSN, d.rexmit_psn

    ARE_ALL_FLAGS_SET(c6, r1, REQ_RX_FLAG_AETH)
    bcf            [!c6 & c1], duplicate_read_resp_mid
    ARE_ALL_FLAGS_SET(c2, r1, REQ_RX_FLAG_ACK) // Branch Delay Slot

    // skip ack sanity checks if there is no aeth hdr
    bcf            [!c6], post_rexmit_psn
 
check_msn:
    // if (msn >= sqcb1_p->ssn) invalid_pkt_msn
    scwle24        c3, d.max_ssn, CAPRI_APP_DATA_AETH_MSN
    bcf            [c3], invalid_pkt_msn
    add            r3, CAPRI_APP_DATA_AETH_SYNDROME, r0 // Branch Delay Slot

    bcf            [!c1], process_aeth
    IS_MASKED_VAL_EQUAL_B(c5, r3, SYNDROME_MASK, ACK_SYNDROME) // Branch Delay Slot

check_duplicate_resp:
    // bth.psn < sqcb1_p->rexmit_psn, duplicate and not unsolicited p_ack, drop
    bcf            [!c2 | !c5], duplicate_resp

    // unsolicited ack i.e. duplicate of most recent p_ack is allowed
    sub            r4, d.rexmit_psn, -1  // Branch Delay Slot
    mincr          r4, 24, r0
    seq            c3, r4, CAPRI_APP_DATA_BTH_PSN
    bcf            [!c3], duplicate_resp

process_aeth:
    bcf            [!c5], post_msn_credits
    phvwr          p.credits, d.credits // Branch Delay Slot

    tblwr          d.credits, CAPRI_APP_DATA_AETH_SYNDROME[4:0]

    // if (sqcb1_p->lsn != ((1 << (sqcb1_p->credits >> 1)) + sqcb1_p->msn))
    //     doorbell_incr_pindex(fc_ring_id) 
    DECODE_ACK_SYNDROME_CREDITS(r2, CAPRI_APP_DATA_AETH_SYNDROME, c1)
    mincr          r2, 24, CAPRI_APP_DATA_AETH_MSN
    sne            c1, d.lsn, r2

post_msn_credits:
    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_MSN_CREDITS)
    // dma_cmd - msn and credits
    add            r4, r7, SQCB2_MSN_OFFSET
    DMA_HBM_PHV2MEM_SETUP(r6, msn, credits, r4)
    bcf            [!c1], post_rexmit_psn
    phvwr          p.credits, d.credits

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
    phvwr          p.err_retry_ctr, d.err_retry_count
    bcf            [c3], unsolicited_ack
    phvwr          p.rnr_retry_ctr, d.rnr_retry_count

    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_REXMIT_PSN)
    add            r4, r7, SQCB2_REXMIT_PSN_OFFSET
    // if valid ack, update rexmit_psn as well as ack timestamp, err_retry_ctr
    // and rnr_retry_ctr in sqcb2
    DMA_HBM_PHV2MEM_SETUP(r6, rexmit_psn, ack_timestamp, r4)

set_arg:

    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, cur_sge_offset), d.rrqwqe_cur_sge_offset, \
              CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, rexmit_psn), r5
    phvwrpair CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, cur_sge_id), d.rrqwqe_cur_sge_id, \
              CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, rrq_in_progress), d.rrq_in_progress
    phvwrpair CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, cq_id), d.cq_id, \
              CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, e_rsp_psn_or_ssn), d.e_rsp_psn
    phvwrpair CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, msn), d.msn, \
              CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, dma_cmd_start_index), REQ_RX_RDMA_PAYLOAD_DMA_CMDS_START
    phvwrpair.c4  CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, e_rsp_psn_or_ssn), d.ssn, \
              CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, rrq_empty), 1
    //phvwr CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, timer_active), d.timer_active
    add            r2, RRQ_C_INDEX, r0
    mincr          r2, d.log_rrq_size, 1
    phvwr      CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, rrq_cindex), r2

    sll            r5, d.rrq_base_addr, RRQ_BASE_ADDR_SHIFT
    add            r5, r5, RRQ_C_INDEX, LOG_RRQ_WQE_SIZE
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqwqe_process, r5)

    nop.e
    nop

unsolicited_ack:
    // if its unsolicted ack, just post credits, msn and exit, CQ posting not needed
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)
    // Load dummy-write-back in stage1 which eventually loads sqcb1-write-back in stage3 to increment nxt-to-go-token-id and drop pvh.
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_dummy_sqcb1_write_back_process, r0)
    nop.e
    nop

duplicate_read_resp_mid:
duplicate_resp:
invalid_pkt_msn:
invalid_serv_type:
invalid_pyld_len:
invalid_opcode:
rrq_empty:
exit:
    // Load dummy-write-back in stage1 which eventually loads sqcb1-write-back in stage3 to increment nxt-to-go-token-id and drop pvh.
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_dummy_sqcb1_write_back_process, r0)
    nop.e
    nop

drop_feedback:
    CAPRI_SET_TABLE_0_VALID(0)
    phvwr         p.common.p4_intr_global_drop, 1

    nop.e
    nop

process_feedback:
    seq            c1, CAPRI_FEEDBACK_FEEDBACK_TYPE, RDMA_TIMER_EXPIRY_FEEDBACK
    bcf            [c1], timer_expiry
    seq            c1, CAPRI_FEEDBACK_FEEDBACK_TYPE, RDMA_COMPLETION_FEEDBACK
    bcf            [!c1], drop_feedback

completion_feedback:
    //phvwr          p.cqe.op_type, CAPRI_COMPLETION_FEEDBACK_OPTYPE // Branch Delay Slot
    CAPRI_COMPLETION_FEEDBACK_WRID(r7)
    phvwr          p.cqe.send.wrid, r7
    phvwrpair      p.cqe.status[7:0], CAPRI_COMPLETION_FEEDBACK_STATUS, p.cqe.error, CAPRI_COMPLETION_FEEDBACK_ERROR

    CAPRI_RESET_TABLE_2_ARG()
    CAPRI_SET_TABLE_0_VALID(0)

    phvwrpair      CAPRI_PHV_FIELD(RRQWQE_TO_CQ_P, cq_id), d.cq_id, \
                   CAPRI_PHV_FIELD(RRQWQE_TO_CQ_P, cqe_type), CQE_TYPE_SEND_NPG
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_cqcb_process, r0)

    nop.e
    nop

timer_expiry:
    // It is ok to update max_tx_psn/max_ssn while bktrack is in progress
    // (provided max_tx_psn/max_ssn is lesser than tx_psn/ssn) as it is
    // not updated as part of bktrack process itself
    scwlt24        c1, d.max_tx_psn, CAPRI_TIMER_EXPIRY_FEEDBACK_TX_PSN
    tblwr.c1       d.max_tx_psn, CAPRI_TIMER_EXPIRY_FEEDBACK_TX_PSN
     
    CAPRI_TIMER_EXPIRY_FEEDBACK_SSN(r1)
    scwlt24        c1, d.max_ssn, r1
    bbeq           d.bktrack_in_progress[0], 1, drop_feedback
    tblwr.c1       d.max_ssn, r1 // Branch Delay Slot

    CAPRI_RESET_TABLE_3_ARG()
    phvwr     CAPRI_PHV_FIELD(SQCB1_TO_TIMER_EXPIRY_P, rexmit_psn), CAPRI_TIMER_EXPIRY_FEEDBACK_REXMIT_PSN

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_timer_expiry_process, r0)
    CAPRI_SET_TABLE_0_VALID(0)

    nop.e
    nop

recirc_pkt:
    // If backtrack is already in progress then continue with processing
    // until req_rx_sqcb1_write_back, where bktrack_in_progress flag is checked
    // and recirc packet is dropped if in the middle of bktracking. This allows
    // nxt_to_go_token_id to be incremented in write_back stage for recirc packets

    /****** Logic to handle already recirculated packets ******/

    // clear recirc bit and process the packet based on recirc reason
    phvwr          p.common.p4_intr_recirc, 0

    seq            c2, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE 
    bcf            [c2], process_recirc_work_not_done
    tblsub.c2      d.work_not_done_recirc_cnt, 1 // BD Slot

drop_packet:
    // Drop if not a known recirc reason
    CAPRI_SET_TABLE_0_VALID(0)
    phvwr.e        p.common.p4_intr_global_drop, 1
    nop

recirc_for_turn:
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_recirc_mpu_only_process, r0)
    phvwr          p.common.p4_intr_recirc, 1
    phvwr.e        p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE
    tbladd         d.work_not_done_recirc_cnt, 1 // Exit Slot

