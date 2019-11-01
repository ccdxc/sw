#include "req_rx.h"
#include "sqcb.h"
#include "defines.h"

struct req_rx_phv_t p;
struct sqcb1_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define SQCB1_TO_RRQWQE_P t0_s2s_sqcb1_to_rrqwqe_info
#define ECN_INFO_P t3_s2s_ecn_info
#define RRQWQE_TO_CQ_P t2_s2s_rrqwqe_to_cq_info
#define SQCB1_TO_TIMER_EXPIRY_P t2_s2s_sqcb1_to_timer_expiry_info
#define SQCB1_TO_COMPL_FEEDBACK_P t2_s2s_sqcb1_to_compl_feedback_info
#define SQCB1_TO_SGE_RECIRC_P t0_s2s_sqcb1_to_sge_recirc_info
#define SQCB1_TO_SQ_DRAIN_P t2_s2s_sqcb1_to_sq_drain_feedback_info

#define TO_S7_P to_s7_stats_info
#define TO_S4_P to_s4_sqcb1_wb_info
#define TO_S3_P to_s3_rrqlkey_info
#define TO_S2_P to_s2_rrqsge_info
#define TO_S1_P to_s1_rrqwqe_info
#define TO_S1_RECIRC_P to_s1_recirc_info

#define TOKEN_ID r6
#define WORK_NOT_DONE_RECIRC_CNT_MAX    8
#define WQE_SIZE_2_SGES                 6
#define P4_INTR_RECIRC_COUNT_MAX        6
#define RDMA_RECIRC_ITER_COUNT_MAX      15 

%%

    .param    req_rx_rrqwqe_process
    .param    req_rx_cqcb_process
    .param    req_rx_dcqcn_ecn_process
    .param    req_rx_rome_pkt_process
    .param    req_rx_recirc_mpu_only_process
    .param    req_rx_timer_expiry_process
    .param    req_rx_dummy_sqcb1_write_back_process
    .param    req_rx_completion_feedback_process
    .param    req_rx_sqcb1_recirc_sge_process
    .param    req_rx_sq_drain_feedback_process
    .param    req_rx_dummy_drop_phv_process
    .param    req_rx_sqcb1_write_back_err_process

.align
req_rx_sqcb1_process:
    bcf            [c2 | c3 | c7], table_error
    add            r1, r0, CAPRI_APP_DATA_RAW_FLAGS  // Branch Delay Slot

    // Do not check and increment token_id for feedback phv unlike
    // response packets. Feedback phvs can be processed out of order from
    // response packets and can be completed in 1 pass. Hence there is no
    // need to allocate token id and recirc these phvs 
    beqi           r1, REQ_RX_FLAG_RDMA_FEEDBACK, process_feedback
    // Initialize cqe to success initially
    phvwrpair      p.cqe.status, CQ_STATUS_SUCCESS, p.cqe.qid, CAPRI_RXDMA_INTRINSIC_QID // Branch Delay Slot

    // If QP is not in RTS state, do not process any received packet. Branch to
    // check for drain state and process packets until number of acknowledged
    // messages (msn) matches total number of messages sent out (max_ssn -1)
    seq            c1, d.state, QP_STATE_RTS // Branch Delay Slot
    bcf            [!c1], check_state

    // is this a new packet or recirc packet
    seq            c1, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0 // Branch Delay Slot
    nop
process_req_rx:
    bcf            [!c1], recirc_pkt
    cmov           TOKEN_ID, c1, d.token_id, CAPRI_APP_DATA_RECIRC_TOKEN_ID // BD Slot

    // If bktrack is in progress do not process any response packets to
    // avoid updating CB state while bktrack logic is updating the same
    bbeq            d.bktrack_in_progress[0], 1, drop_packet

    // If number of work_not_done recirc packets are more than what can be handled,
    //  drop and let requester retry the requests
    slt             c2, d.work_not_done_recirc_cnt, WORK_NOT_DONE_RECIRC_CNT_MAX // Branch Delay Slot

    // see if received P_KEY is DEFAULT_PKEY
    seq             c3, CAPRI_APP_DATA_BTH_P_KEY, DEFAULT_PKEY

    bcf             [!c2 | !c3], drop_packet

    // copy cur_timestamp loaded in r4 into phv to DMA ack_timestamp
    // into sqcb2 for valid aeth packet
    phvwr          p.ack_timestamp, r4 // Branch Delay slot

    // get token_id for this packet
    phvwr          p.common.rdma_recirc_token_id, TOKEN_ID
    phvwr          CAPRI_PHV_FIELD(TO_S4_P, my_token_id), TOKEN_ID

    // recirc if work_not_done_recirc_cnt != 0
    seq            c2, d.work_not_done_recirc_cnt, 0
    bcf            [!c2], recirc_for_turn

    tbladd         d.token_id, 1 // Branch Delay Slot

process_recirc_work_not_done:
    crestore    [c3, c2], r1, (REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_ONLY) 
    seq         c7, d.pkt_spec_enable, 1
    setcf       c4, [!c3 | c2 | c7]

    // skip_token_id_check if not read response or read response and
    // its read_resp_only or read_response with rrqwqe size of <= 2 SGES
    // (optimized 2 sge speculation processing) or recirc packet with reason
    // other than work_not_done
    bcf         [c1 & c4], skip_token_id_check  // c1 is set initially if (recirc_cnt = 0)

token_id_check:
    // Slow path: token id check is mandatory if fresh packet with Read-resp FML, OR
    // recirc packet with reason work_not_done
    seq         c2, TOKEN_ID, d.nxt_to_go_token_id // BD slot
    bcf         [!c2], recirc_for_turn

skip_token_id_check:
    //Check if congestion management type is Rome
    seq            c3, d.congestion_mgmt_type, 2
    bcf            [!c3], dcqcn_ecn_process

    add            r2, AH_ENTRY_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT //dcqcn_cb addr //BD Slot
    add            r2, r2, DCQCN_CB_T_SIZE_BYTES // rome_receiver_cb addr
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_rome_pkt_process, r2)
    b              process_rx_pkt
    nop //BD Slot

dcqcn_ecn_process:
    //Check if ECN bits are set in Packet and congestion management is enabled.                      
    sne            c5, k.rdma_bth_ecn, 3  // BD-Slot
    sne            c6, d.congestion_mgmt_type, 1
    bcf            [c5 | c6], process_rx_pkt

    // Load dcqcn_cb to store timestamps and trigger Doorbell to generate CNP.
    CAPRI_RESET_TABLE_3_ARG() //BD Slot
    phvwr   CAPRI_PHV_FIELD(ECN_INFO_P, p_key), CAPRI_APP_DATA_BTH_P_KEY

    // r2 has been calculated in BD Slot
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

    CAPRI_RESET_TABLE_0_ARG()

    ARE_ALL_FLAGS_SET(c3, r1, REQ_RX_FLAG_ACK) // Branch Delay Slot
    bcf            [!c3], atomic 
    seq            c4, RRQ_P_INDEX, d.rrq_spec_cindex // Branch Delay Slot

ack:
    // remaining_payload_bytes != 0
    bne            r2, r0, invalid_pyld_len
    nop            // Branch Delay Slot
    b              check_psn

atomic:
    ARE_ALL_FLAGS_SET(c1, r1, REQ_RX_FLAG_ATOMIC_AETH)
    bcf            [!c1], read
    phvwr          CAPRI_PHV_FIELD(TO_S3_P, pd), d.pd //BD Slot

    // remaining_payload_bytes != 0 
    bne            r2, r0, invalid_pyld_len
    nop

    bcf            [c4], rrq_empty
    nop            // Branch Delay Slot

    b              check_psn
    phvwr          CAPRI_PHV_FIELD(TO_S1_P, priv_oper_enable), d.sqcb1_priv_oper_enable

read:
    ARE_ALL_FLAGS_SET(c1, r1, REQ_RX_FLAG_READ_RESP)
    bcf            [!c1], invalid_opcode

    // remaining_payload_bytes != pmtu
    sll            r3, 1, d.log_pmtu
    sne            c1, r3, r2
    IS_ANY_FLAG_SET(c3, r1, REQ_RX_FLAG_MIDDLE|REQ_RX_FLAG_FIRST)
    bcf            [c1 & c3], invalid_pyld_len

    // remaining_payload_bytes > pmtu
    slt            c1, r3, r2 // Branch Delay Slot
    IS_ANY_FLAG_SET(c2, r1, REQ_RX_FLAG_ONLY|REQ_RX_FLAG_LAST)
    bcf            [c1 & c2], invalid_pyld_len

    // remaining_payload_bytes < 1
    slt            c1, r2, 1 // Branch Delay Slot
    ARE_ALL_FLAGS_SET(c5, r1, REQ_RX_FLAG_LAST)
    bcf            [c1 & c5], invalid_pyld_len
    nop            // Branch Delay Slot

    bcf            [c4], rrq_empty
    phvwrpair      CAPRI_PHV_FIELD(TO_S2_P, log_pmtu), d.log_pmtu, \
                   CAPRI_PHV_FIELD(TO_S2_P, priv_oper_enable), d.sqcb1_priv_oper_enable //BD Slot

    bcf            [!c7], check_psn
    phvwrpair      CAPRI_PHV_FIELD(TO_S1_P, priv_oper_enable), d.sqcb1_priv_oper_enable, \
                   CAPRI_PHV_FIELD(TO_S1_P, log_pmtu), d.log_pmtu  //BD Slot

    phvwr          CAPRI_PHV_FIELD(TO_S1_P, sge_opt), 1
    phvwr          CAPRI_PHV_FIELD(TO_S4_P, sge_opt), 1

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

    // PSN in response should be less than the last request
    // PSN sent out
    scwle24        c3, d.max_tx_psn, CAPRI_APP_DATA_BTH_PSN
    bcf            [c3], invalid_pkt_psn

check_duplicate_read_resp_mid:
    // bth.psn >= sqcb1_p->rexmit_psn, valid response
    scwlt24        c1, CAPRI_APP_DATA_BTH_PSN, d.rexmit_psn

    ARE_ALL_FLAGS_SET(c6, r1, REQ_RX_FLAG_AETH)
    bcf            [!c6 & c1], duplicate_read_resp_mid
    ARE_ALL_FLAGS_SET(c2, r1, REQ_RX_FLAG_ACK) // Branch Delay Slot

    // skip ack sanity checks if there is no aeth hdr
    bcf            [!c6], post_rexmit_psn_to_ack_timestamp
 
check_msn:
    // if (msn >= sqcb1_p->ssn) invalid_pkt_msn
    scwle24        c3, d.max_ssn, CAPRI_APP_DATA_AETH_MSN
    bcf            [c3], invalid_pkt_msn
    add            r3, CAPRI_APP_DATA_AETH_SYNDROME, r0 // Branch Delay Slot

    bcf            [!c1], process_aeth
    IS_MASKED_VAL_EQUAL_B(c6, r3, SYNDROME_MASK, ACK_SYNDROME) // Branch Delay Slot

check_duplicate_resp:
    // bth.psn < sqcb1_p->rexmit_psn, duplicate and not unsolicited p_ack, drop
    bcf            [!c2 | !c6], duplicate_resp

    // unsolicited ack i.e. duplicate of most recent p_ack is allowed
    sub            r4, d.rexmit_psn, 1  // Branch Delay Slot
    mincr          r4, 24, r0
    seq            c3, r4, CAPRI_APP_DATA_BTH_PSN
    bcf            [!c3], duplicate_resp

process_aeth:
    sne.c6         c6, CAPRI_APP_DATA_AETH_SYNDROME[4:0], 0x1F // Branch Delay Slot
    // Skip LSN update if not ACK syndrome or ACK syndrome but invalid credits
    bcf            [!c6], post_rexmit_psn_to_ack_timestamp

    DECODE_ACK_SYNDROME_CREDITS(r2, r5, CAPRI_APP_DATA_AETH_SYNDROME, c1)
    mincr          r2, 24, CAPRI_APP_DATA_AETH_MSN

post_lsn_to_ack_timestamp:
    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_LSN_OR_REXMIT_PSN)
    // dma_cmd - msn and credits
    add            r4, r7, SQCB2_LSN_RX_OFFSET
    DMA_HBM_PHV2MEM_SETUP(r6, lsn, lsn, r4)
    phvwr          p.lsn, r2

    // if its unsolicited ack with valid credits, just post credits and exit
    // unset end_of_cmd if later in write_back completion is posted
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

    b              set_arg
    // If its valid ack with credits, post credits, ack_timestamp
    DMA_HBM_PHV2MEM_PHV_END_SETUP_C(r6, ack_timestamp, !c3) // Branch Delay Slot

post_rexmit_psn_to_ack_timestamp:
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_LSN_OR_REXMIT_PSN)
    add            r4, r7, SQCB2_REXMIT_PSN_OFFSET
    // if its rnr/nak/read_resp mid, update rexmit_psn and ack_timestamp
    DMA_HBM_PHV2MEM_SETUP(r6, rexmit_psn, ack_timestamp, r4)

set_arg:
    phvwr          p.err_retry_ctr, d.err_retry_count
    phvwr          p.rnr_retry_ctr, d.rnr_retry_count

    phvwrpair      CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, cur_sge_offset), d.rrqwqe_cur_sge_offset, \
                   CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, tx_psn), d.tx_psn
    phvwrpair      CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, cur_sge_id), d.rrqwqe_cur_sge_id, \
                   CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, ssn), d.ssn
    phvwrpair      CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, msn), d.msn, \
                   CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, dma_cmd_start_index), REQ_RX_RDMA_PAYLOAD_DMA_CMDS_START
    phvwr.c4       CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, rrq_empty), 1
    add            r2, RRQ_C_INDEX, r0
    mincr          r2, d.log_rrq_size, 1
    phvwr          CAPRI_PHV_FIELD(SQCB1_TO_RRQWQE_P, rrq_cindex), r2

set_rrqwqe_pc:
    sll            r5, d.rrq_base_addr, RRQ_BASE_ADDR_SHIFT
    add            r5, r5, d.rrq_spec_cindex, LOG_RRQ_WQE_SIZE

    phvwr          CAPRI_PHV_FIELD(TO_S4_P, rrq_spec_cindex), d.rrq_spec_cindex
    // Increment spec-cindex for Read-Resp-Only/Read-Resp-Last/Atomic packets.
    IS_ANY_FLAG_SET(c2, r1, REQ_RX_FLAG_ONLY| REQ_RX_FLAG_LAST| REQ_RX_FLAG_ATOMIC_AETH)
    tblmincri.c2   d.rrq_spec_cindex, d.log_rrq_size, 1

    // If rrq is empty(c4 is true), do not load rrqwqe entry but invoke mpu only
    // req_rx_rrqwqe_process. If rrq is non-empty, load rrqwqe and lock
    // the table to update msg_psn in the rrqwqe
    CAPRI_NEXT_TABLE0_C_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, r0, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r5, req_rx_rrqwqe_process, c4)

recirc_work_done:
    // Load dummy-write-back in stage1 which eventually loads sqcb1-write-back.
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_dummy_sqcb1_write_back_process, r0)
    
duplicate_read_resp_mid:
duplicate_resp:
    b           exit
    phvwr       CAPRI_PHV_RANGE(TO_S7_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_DUPLICATE_RESPONSES_OFFSET)) //BD Slot

invalid_pkt_psn:
invalid_pkt_msn:
    b           exit
    phvwr       CAPRI_PHV_RANGE(TO_S7_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_PACKET_SEQ_ERR_OFFSET)) //BD Slot

invalid_serv_type:
invalid_pyld_len:
invalid_opcode:
rrq_empty:
    b           exit
    phvwr       CAPRI_PHV_RANGE(TO_S7_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_INVALID_PACKETS_OFFSET)) //BD Slot

exit:
    phvwr          CAPRI_PHV_FIELD(TO_S4_P, error_drop_phv), 1
    // Load dummy-write-back in stage1 which eventually loads sqcb1-write-back in stage3 to increment nxt-to-go-token-id and drop pvh.
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_dummy_sqcb1_write_back_process, r0)

process_feedback:
    seq            c1, CAPRI_FEEDBACK_FEEDBACK_TYPE, RDMA_COMPLETION_FEEDBACK
    bcf            [c1], completion_feedback
    seq            c1, CAPRI_FEEDBACK_FEEDBACK_TYPE, RDMA_TIMER_EXPIRY_FEEDBACK
    bcf            [c1], timer_expiry_feedback
    seq            c1, CAPRI_FEEDBACK_FEEDBACK_TYPE, RDMA_SQ_DRAIN_FEEDBACK
    bcf            [!c1], drop_packet

sq_drain_feedback:
    CAPRI_SQ_DRAIN_FEEDBACK_SSN(r1)
    scwlt24        c1, d.max_ssn, r1
    tblwr.c1       d.max_ssn, r1

    scwlt24        c1, d.max_tx_psn, CAPRI_SQ_DRAIN_FEEDBACK_TX_PSN
    tblwr.c1       d.max_tx_psn, CAPRI_SQ_DRAIN_FEEDBACK_TX_PSN

    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_RESET_TABLE_2_ARG()

    phvwr     CAPRI_PHV_FIELD(SQCB1_TO_SQ_DRAIN_P, ssn), d.max_ssn
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sq_drain_feedback_process, r0)

completion_feedback:
    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_RESET_TABLE_2_ARG()

    CAPRI_COMPLETION_FEEDBACK_WRID(r7) // Branch Delay Slot
    phvwr          p.cqe.send.wrid, r7
    seq            c1, CAPRI_COMPLETION_FEEDBACK_STATUS, CQ_STATUS_SUCCESS
    phvwr.!c1      CAPRI_PHV_RANGE(TO_S7_P, lif_cqe_error_id_vld, lif_error_id), \
                       CAPRI_COMPLETION_FEEDBACK_LIF_STATS_INFO
                       
    bcf            [c1], set_cqcb_arg
    phvwrpair      p.cqe.status[7:0], CAPRI_COMPLETION_FEEDBACK_STATUS, p.cqe.error, CAPRI_COMPLETION_FEEDBACK_ERROR //BD Slot

process_err_feedback:
    // if QP has to be err disabled instanly, then its likely that the problem
    // happened in bktrack. SSN and TX_PSN in feedback msg are not valid, so do
    // not update sqcb1's max_snn and tx_psn with those values.
    bbeq           CAPRI_COMPLETION_FEEDBACK_ERR_QP_INSTANTLY, 1, trigger_completion_feedback
    scwlt24        c1, d.max_ssn, CAPRI_COMPLETION_FEEDBACK_SSN // Branch Delay Slot
    tblwr.c1       d.max_ssn, CAPRI_COMPLETION_FEEDBACK_SSN

    scwlt24        c1, d.max_tx_psn, CAPRI_COMPLETION_FEEDBACK_TX_PSN
    tblwr.c1       d.max_tx_psn, CAPRI_COMPLETION_FEEDBACK_TX_PSN

trigger_completion_feedback:
    phvwrpair      CAPRI_PHV_FIELD(SQCB1_TO_COMPL_FEEDBACK_P, status), CAPRI_COMPLETION_FEEDBACK_STATUS, \
                   CAPRI_PHV_FIELD(SQCB1_TO_COMPL_FEEDBACK_P, err_qp_instantly), CAPRI_COMPLETION_FEEDBACK_ERR_QP_INSTANTLY
    phvwr      CAPRI_PHV_FIELD(SQCB1_TO_COMPL_FEEDBACK_P, ssn), d.max_ssn
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_completion_feedback_process, r0)

set_cqcb_arg:
    phvwrpair      CAPRI_PHV_FIELD(RRQWQE_TO_CQ_P, cq_id), d.cq_id, \
                   CAPRI_PHV_FIELD(RRQWQE_TO_CQ_P, cqe_type), CQE_TYPE_SEND_NPG
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_cqcb_process, r0)

timer_expiry_feedback:
    // It is ok to update max_tx_psn/max_ssn while bktrack is in progress
    // (provided max_tx_psn/max_ssn is lesser than tx_psn/ssn) as it is
    // not updated as part of bktrack process itself
    scwlt24        c1, d.max_tx_psn, CAPRI_TIMER_EXPIRY_FEEDBACK_TX_PSN
    tblwr.c1       d.max_tx_psn, CAPRI_TIMER_EXPIRY_FEEDBACK_TX_PSN
     
    CAPRI_TIMER_EXPIRY_FEEDBACK_SSN(r1)
    scwlt24        c1, d.max_ssn, r1
    bbeq           d.bktrack_in_progress[0], 1, drop_packet
    tblwr.c1       d.max_ssn, r1 // Branch Delay Slot

    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_RESET_TABLE_2_ARG()

    phvwr     CAPRI_PHV_FIELD(SQCB1_TO_TIMER_EXPIRY_P, rexmit_psn), CAPRI_TIMER_EXPIRY_FEEDBACK_REXMIT_PSN
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_timer_expiry_process, r0)

recirc_pkt:

    // clear recirc bit and process the packet based on recirc reason
    phvwr           p.common.p4_intr_recirc, 0

    // if the p4 intrinsic recirc_count has not hit the limit, proceed with pkt handling
    seq             c6, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, P4_INTR_RECIRC_COUNT_MAX
    bcf             [!c6], skip_recirc_cnt_max_check

    // check if phv's recirc iter count has hit the limit
    add             r7, 1, CAPRI_APP_DATA_RECIRC_ITER_COUNT //BD Slot

    // did we reach recirc_iter_count to 15 ? 
    seq             c6, r7, RDMA_RECIRC_ITER_COUNT_MAX
    bcf             [c6], max_recirc_cnt_err
    phvwr.!c6       p.common.p4_intr_recirc_count, 1  //BD Slot
    phvwr           p.common.rdma_recirc_recirc_iter_count, r7


skip_recirc_cnt_max_check:
    // If backtrack is already in progress then continue with processing
    // until req_rx_sqcb1_write_back, where bktrack_in_progress flag is checked
    // and recirc packet is dropped if in the middle of bktracking. This allows
    // nxt_to_go_token_id to be incremented in write_back stage for recirc packets

    /****** Logic to handle already recirculated packets ******/

    seq            c2, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE 
    bcf            [c2], process_recirc_work_not_done
    tblsub.c2      d.work_not_done_recirc_cnt, 1 //BD Slot
    seq            c2, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_ERROR_DISABLE_QP
    bcf            [c2], process_recirc_error_disable_qp
    seq            c2, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_INORDER_WORK_DONE // BD Slot
    bcf            [c2], recirc_work_done
    seq            c2, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_SGE_WORK_PENDING // BD Slot
    bcf            [c2], process_recirc_sge_work_pending
    nop
    // fall-through.Drop if not a known recirc reason

drop_packet:
    phvwr          p.common.p4_intr_global_drop, 1
    // Load dummy-drop-phv to avoid loading eth programs in stage 1 and 5. Today if no table is loaded, common-rxdma eth program
    // will be loaded in stage1
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_dummy_drop_phv_process, r0) //Exit Slot

recirc_for_turn:
    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_recirc_mpu_only_process, r0)
    phvwr          p.common.p4_intr_recirc, 1
    phvwr.e        p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE
    tbladd         d.work_not_done_recirc_cnt, 1 // Exit Slot

process_recirc_error_disable_qp:
    phvwr       CAPRI_PHV_FIELD(TO_S4_P, error_disable_qp), 1

    // Load dummy-write-back in stage1 which eventually loads sqcb1-write-back in stage3 to increment nxt-to-go-token-id and drop pvh.
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_dummy_sqcb1_write_back_process, r0) //Exit Slot

check_state:
    slt             c2, d.state, QP_STATE_SQD
    bcf             [!c2], process_req_rx
    nop       // Branch Delay Slot

    b               drop_packet
    nop

process_recirc_sge_work_pending:
    CAPRI_RESET_TABLE_0_ARG()
    phvwr          CAPRI_PHV_FIELD(TO_S1_RECIRC_P, sge_opt), d.pkt_spec_enable
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_recirc_sge_process, r0)

table_error:
    // TODO add LIF stats
    b               drop_packet
    nop

max_recirc_cnt_err:
    //a packet which went thru too many recirculations had to be terminated and qp had to 
    //be put into error disabled state. The recirc reason, opcode, the psn of the packet etc. 
    //are remembered for further debugging.
    phvwrpair   CAPRI_PHV_FIELD(TO_S7_P, max_recirc_cnt_err), 1, \
                CAPRI_PHV_FIELD(TO_S7_P, recirc_reason), CAPRI_APP_DATA_RECIRC_REASON

    phvwrpair   CAPRI_PHV_FIELD(TO_S7_P, recirc_bth_opcode), CAPRI_APP_DATA_BTH_OPCODE, \
                CAPRI_PHV_FIELD(TO_S7_P, recirc_bth_psn), CAPRI_APP_DATA_BTH_PSN

    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_err_process, r0)     
