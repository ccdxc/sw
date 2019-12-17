#include "req_rx.h"
#include "sqcb.h"
#include "defines.h"

struct req_rx_phv_t p;
struct rrqwqe_t d;
struct req_rx_s1_t0_k k;

#define RRQWQE_TO_SGE_P t0_s2s_rrqwqe_to_sge_info
#define RRQWQE_TO_CQ_P t2_s2s_rrqwqe_to_cq_info
#define RRQSGE_TO_LKEY_P t0_s2s_rrqsge_to_lkey_info
#define SQCB1_WRITE_BACK_P t2_s2s_sqcb1_write_back_info

#define IN_P t0_s2s_sqcb1_to_rrqwqe_info
#define IN_TO_S_P to_s1_rrqwqe_info

#define K_SSN       CAPRI_KEY_RANGE(IN_P, ssn_sbit0_ebit7, ssn_sbit16_ebit23)
#define K_CUR_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, cur_sge_offset_sbit0_ebit7, cur_sge_offset_sbit16_ebit31)
#define K_CUR_SGE_ID CAPRI_KEY_FIELD(IN_P, cur_sge_id)
#define K_RRQ_CINDEX CAPRI_KEY_RANGE(IN_P, rrq_cindex_sbit0_ebit0, rrq_cindex_sbit1_ebit7)
#define K_DMA_CMD_START_INDEX CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index)
//#define K_CQ_ID CAPRI_KEY_RANGE(IN_P, cq_id_sbit0_ebit15, cq_id_sbit16_ebit23)
#define K_TX_PSN CAPRI_KEY_RANGE(IN_P, tx_psn_sbit0_ebit0, tx_psn_sbit17_ebit23)

#define K_AETH_SYNDROME CAPRI_KEY_FIELD(IN_TO_S_P, aeth_syndrome)
#define K_AETH_MSN      CAPRI_KEY_FIELD(IN_TO_S_P, aeth_msn)
#define K_BTH_PSN  CAPRI_KEY_FIELD(IN_TO_S_P, bth_psn)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_TO_S_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit13)
#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, priv_oper_enable)
#define K_LOG_PAGE_SIZE  CAPRI_KEY_RANGE(IN_TO_S_P, log_page_size_sbit0_ebit2, log_page_size_sbit3_ebit4)

#define TO_S1_RECIRC_P to_s1_recirc_info
#define TO_S2_P to_s2_rrqsge_info
#define TO_S4_P to_s4_sqcb1_wb_info
#define TO_S6_P to_s6_cq_info
#define TO_S7_P to_s7_stats_info

%%
    .param    req_rx_rrqsge_process
    .param    req_rx_rrqsge_opt_process
    .param    req_rx_rrqlkey_process
    .param    req_rx_sqcb1_write_back_process
    .param    req_rx_cqcb_process
    .param    req_rx_rrqwqe_base_sge_process
    .param    req_rx_rrqwqe_base_sge_opt_process
    .param    req_rx_rrqlkey_rsvd_lkey_process

.align
req_rx_rrqwqe_process:
    add            r5, r0, K_GLOBAL_FLAGS
    ARE_ALL_FLAGS_SET(c1, r5, REQ_RX_FLAG_ACK)

    bcf            [!c1], read_or_atomic_or_implicit_nak
    ARE_ALL_FLAGS_SET(c4, r5, REQ_RX_FLAG_COMPLETION)  // Branch Delay Slot

ack_or_nak_or_rnr_or_implicit_nak:
    CAPRI_SET_TABLE_0_VALID(0)

    // r1 = aeth_syndrome
    add            r1, K_AETH_SYNDROME, r0

    // MSN posted in CQ should be smaller than SSN. In case
    // of ack drops, response to retried request can have PSN/MSN 
    // logically greater than PSN/MSN of the retried request but
    // that should not result in completion of WQEs more than those
    // that have been retransmitted. If c6 is FALSE, requester is in
    // retransmitting packets and so bound to see acks/read/atomic
    // responses with MSN/PSN that are not sent out
    scwlt24        c6, K_AETH_MSN, K_SSN
    cmov           r2, c6, K_AETH_MSN, K_SSN

    // This is an ack/nak/rnr packet. If rrq_empty (no outstanding
    // read or atomic response) then msn in this ack/nak/rnr 
    // should be completed. if rrq is not empty, then
    // receiving ack means read/atomic responses are missing, so
    // completion should be posted for lowest of msn in the ack or 
    // msn of the message before oldest read/atomic request in rrq
    bbeq           CAPRI_KEY_FIELD(IN_P, rrq_empty), 1, ack_or_nak_or_rnr
    mincr.!c6      r2, 24, -1 // Branch Delay Slot
    add            r3, d.msn, 0
    mincr          r3, 24, -1
    scwle24        c5, r3, r2
    cmov           r2, c5, r3, r2

    // if (pkt_psn >= rrqwqe_p->psn)
    // implicit nak, ring bktrack ring setting rexmit_psn to rrqwqe_p->psn
    scwlt24        c5, d.psn, K_BTH_PSN // Branch Delay Slot
    bcf            [c5 & c6], implicit_nak
ack_or_nak_or_rnr:
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

    CAPRI_RESET_TABLE_2_ARG()
    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_offset), K_CUR_SGE_OFFSET, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_id), K_CUR_SGE_ID
    
    phvwr          p.cqe.send.msn, r2
    IS_MASKED_VAL_EQUAL(c3, r1, SYNDROME_MASK, ACK_SYNDROME)
    bcf            [c6 & !c3], nak_or_rnr
  
ack:
    cmov           r6, c6, K_BTH_PSN, K_TX_PSN // Branch Delay Slot
    mincr.c6       r6, 24, 1
    seq            c2, CAPRI_KEY_FIELD(IN_P, rrq_empty), 1
    scwlt24.!c2    c3, r6, d.e_psn
    cmov.!c2       r6, c3, r6, d.e_psn
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1
    phvwr.e        p.rexmit_psn, r6
    phvwrpair      CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2

nak_or_rnr:
    IS_MASKED_VAL_EQUAL_B(c3, r1, SYNDROME_MASK, NAK_SYNDROME)
    bcf            [!c3], rnr
    phvwr          p.rexmit_psn, K_BTH_PSN // Branch Delay Slot

nak:
    add            r6, K_BTH_PSN, r0
    // SQ backtrack if NAK is due to SEQ_ERR    
    IS_MASKED_VAL_EQUAL_B(c3, r1, NAK_CODE_MASK, NAK_CODE_SEQ_ERR)
    bcf            [c3], nak_completion
    phvwr.c3       CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_bktrack), 1

    // If its invalid nak code (reserved or Invalid_rd_request
    // 0110 0100 - 0110 1111), just drop the ack. This results in
    // retransmission at a later time and eventual error disabling of the QP
    // on exceeding num err retry count. Invalid_rd_Request is also not a valid
    // nak code for RC service 
    bgti           r1, 0x63, invalid_nak_code
    IS_MASKED_VAL_EQUAL_B(c3, r1, NAK_CODE_MASK, NAK_CODE_INV_REQ)
    phvwrpair.c3   p.cqe.status, CQ_STATUS_REMOTE_INV_REQ_ERR, \
                   p.cqe.error, 1
    phvwr.c3       CAPRI_PHV_RANGE(TO_S7_P, lif_cqe_error_id_vld, lif_error_id), \
                    ((1 << 5) | (1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_REMOTE_INV_REQ_ERR_OFFSET))
    phvwrpair.c3   CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rrqwqe_remote_inv_req_err_rcvd), 1

    IS_MASKED_VAL_EQUAL_B(c3, r1, NAK_CODE_MASK, NAK_CODE_REM_ACC_ERR)
    phvwrpair.c3   p.cqe.status, CQ_STATUS_REMOTE_ACC_ERR, \
                   p.cqe.error, 1
    phvwr.c3       CAPRI_PHV_RANGE(TO_S7_P, lif_cqe_error_id_vld, lif_error_id), \
                    ((1 << 5) | (1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_REMOTE_ACC_ERR_OFFSET))
    phvwrpair.c3   CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rrqwqe_remote_acc_err_rcvd), 1

    IS_MASKED_VAL_EQUAL_B(c3, r1, NAK_CODE_MASK, NAK_CODE_REM_OP_ERR)
    phvwrpair.c3   p.cqe.status, CQ_STATUS_REMOTE_OPER_ERR, \
                   p.cqe.error, 1
    phvwr.c3       CAPRI_PHV_RANGE(TO_S7_P, lif_cqe_error_id_vld, lif_error_id), \
                    ((1 << 5) | (1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_REMOTE_OPER_ERR_OFFSET))
    phvwrpair.c3   CAPRI_PHV_FIELD(TO_S7_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_P, qp_err_dis_rrqwqe_remote_oper_err_rcvd), 1

    phvwr          CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp), 1
    // post err completion for msn one more than the one last completed
    // as specified in NAK
    add            r2, K_AETH_MSN, 1
    mincr          r2, 24, r0
    phvwr          p.cqe.send.msn, r2
    mincr          r6, 24, 1
    phvwr          p.rexmit_psn, r6

nak_completion:
    phvwrpair.e    CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1

rnr:
    IS_MASKED_VAL_EQUAL_B(c3, r1, SYNDROME_MASK, RNR_SYNDROME)
    bcf            [!c3], invalid_syndrome

    // RNR timeout is stored as 0x20 if rnr syndrome value is 0. This is because
    // TxDMA rnr_timeout value should be non-zero for expiry logic to use
    // rnr timeout rather than local ack timeout.
    // RNR timeout is decoded using the below formula which will provide
    // right value if 0 is stored as 0x20

    // if rnr syndrome value is represented as TTTTT, first 4bits
    // is say "n" and last bit is "m" then  timeout value is
    // (2^n + m*2^(n-1))/100 msec. As per the spec, 0 is 655.36 msec
    seq            c6, r1[4:0], 0 // BD-Slot
    cmov           r3, c6, 0x20, r1[4:0]
    phvwr          p.rnr_timeout, r3

    // DMA rnr_timeout to sqcb2 and set end-of-cmds. If post_cq happens end-of-cmds will be unset in sqcb1-wb.
    SQCB2_ADDR_GET(r1)
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_RNR_TIMEOUT)
    add            r3, r1, SQCB2_RNR_TIMEOUT_OFFSET
    DMA_HBM_PHV2MEM_SETUP(r6, rnr_timeout, rnr_timeout, r3)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

    phvwr          CAPRI_PHV_RANGE(TO_S7_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_RNR_RETRY_ERR_OFFSET))

    phvwrpair.e    CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), K_BTH_PSN, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1

read_or_atomic_or_implicit_nak:
    ARE_ALL_FLAGS_SET(c3, r5, REQ_RX_FLAG_FIRST) // Branch Delay Slot

    // atomic or read requires explicit response so msn in atomic or
    // read_resp_last or read_resp_only should be >= the corresponding message's
    // sequence number
    scwlt24.c4     c2, K_AETH_MSN, d.msn
    bcf            [c4 & !c3 & c2], invalid_read_or_atomic_rsp_msn

    // pkt->psn == rrqwqe_p->e_psn
    sne            c2, K_BTH_PSN, d.e_psn
    bcf            [c2], invalid_read_or_atomic_rsp_msn

read_or_atomic:
    sub            r1, d.e_psn, d.psn
    mincr          r1, 24, 0
    tblmincri      d.e_psn, 24, 1

    phvwr          p.cqe.send.msn, d.msn
    phvwr          p.rexmit_psn, d.e_psn
    
    seq            c2, d.read_rsp_or_atomic, RRQ_OP_TYPE_READ
    bcf            [!c2], atomic
    // Zero length read resp should only post completion and no data
    // transfer
    seq            c2, d.num_sges, r0 // Branch Delay Slot
    bcf            [c2], zero_length_read 

read:
    IS_ANY_FLAG_SET(c5, r5, (REQ_RX_FLAG_ONLY|REQ_RX_FLAG_LAST)) // Branch Delay Slot

    // Just in case if there are more than 2 sges to be processed for this
    // phv, recirc path requires sge_list_addr, so populate it here as only
    // wqe stage is aware of this address
    phvwr     CAPRI_PHV_FIELD(TO_S1_RECIRC_P, rrqwqe_sge_list_addr), d.read.wqe_sge_list_addr

    CAPRI_RESET_TABLE_2_ARG()
    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), d.e_psn
    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), d.msn

    seq            c2, CAPRI_KEY_FIELD(IN_TO_S_P, sge_opt), 1
    j.c2           req_rx_rrqwqe_base_sge_opt_process
    phvwr.c5  CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1 // Branch Delay Slot

    seq            c2, K_CUR_SGE_ID, 0
    j.c2           req_rx_rrqwqe_base_sge_process

    add            r3, d.read.wqe_sge_list_addr, K_CUR_SGE_ID, LOG_SIZEOF_SGE_T // Branch Delay Slot

    // if log_rq_page_size = 0, rq is in hbm and page boundary check is not needed
    phvwr           CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, end_of_page), 0

    seq             c3, K_LOG_PAGE_SIZE, 0
    sub.c3          r3, r3, 2, LOG_SIZEOF_SGE_T
    bcf             [c3], page_boundary_check_done

    srl            r5, r3, K_LOG_PAGE_SIZE // Branch Delay Slot
    add            r6, r3, (RRQWQE_SGE_TABLE_READ_SIZE - 1)
    srl            r6, r6, K_LOG_PAGE_SIZE
    sne            c3, r5, r6
    // move addr_to_load back by sizeof 2 SGE's
    sub.!c3        r3, r3, 2, LOG_SIZEOF_SGE_T
    // start addr and end addr are not in the same page, move addr_to_load back by sizeof 3 SGE's
    phvwr.c3       CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, end_of_page), 1
    sub.c3         r3, r3, 3, LOG_SIZEOF_SGE_T

page_boundary_check_done:
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, \
                                req_rx_rrqsge_process, r3)

    CAPRI_RESET_TABLE_0_ARG()
    //phvwr CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, is_atomic), 0
    phvwrpair CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, remaining_payload_bytes), K_REMAINING_PAYLOAD_BYTES, \
              CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, cur_sge_id), K_CUR_SGE_ID
    phvwr     CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, cur_sge_offset), K_CUR_SGE_OFFSET
    sub            r3, d.num_sges, K_CUR_SGE_ID
    phvwrpair.e CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, num_valid_sges), r3, \
                CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_start_index), K_DMA_CMD_START_INDEX

    // if read_resp contains already ack'ed msn, do not post CQ
    // ideally this should happen only with read_resp_first. For read_resp_last
    // or read_resp_only, msn should always be the un-acked msn
    phvwr.!c5  CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_eop), 1

zero_length_read:

    CAPRI_SET_TABLE_0_VALID(0)
    // to keep it consistent with read process where
    // table 0 and 1 are taken for sge process
    CAPRI_RESET_TABLE_2_ARG()

    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, last_pkt), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), d.e_psn, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), d.msn
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

atomic:
    #phvwr          p.cqe.op_type, d.atomic.op_type
    KT_BASE_ADDR_GET2(r3, r2)
    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+
    //                     ((sge_p->lkey >> KEY_INDEX_SHIFT) * sizeof(key_entry_t));
    add            r2, d.atomic.sge.l_key, r0
    KEY_ENTRY_ADDR_GET(r3, r3, r2)

    seq            c6, r2, RDMA_RESERVED_LKEY_ID
    phvwr.c6       CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, rsvd_key_err), 1
    crestore.c6    [c6], K_PRIV_OPER_ENABLE, 0x1

    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, sge_va), d.atomic.sge.va, \
              CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, sge_bytes), d.atomic.sge.len[15:0]
    phvwrpair CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, dma_cmd_eop), 0, \
              CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, dma_cmd_start_index), K_DMA_CMD_START_INDEX
    //phvwr CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, sge_index), 0
    //phvwrpair CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, cq_dma_cmd_index), REQ_RX_DMA_CMD_CQ, \
    //          CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, cq_id), K_CQ_ID
    phvwrpair CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, is_atomic), 1, \
              CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, bubble_one_stage), 1

    CAPRI_NEXT_TABLE0_READ_PC_C(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_rrqlkey_rsvd_lkey_process, req_rx_rrqlkey_process, r3, c6)

    // Hardcode table id 2 for write_back process
    // to keep it consistent with read process where
    // table 0 and 1 are taken for sge process
    CAPRI_RESET_TABLE_2_ARG()

    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, last_pkt), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), d.e_psn, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), d.msn

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

end:
    nop.e
    nop

implicit_nak:
    // PSN of the next expected read/atomic response to rexmit from
    phvwr          p.rexmit_psn, d.e_psn
    
    // if its implicit nak, completion can be posted till wqe before
    // the first outstanding read request wqe
    add            r2, d.msn, 0
    mincr          r2, 24, -1
    phvwr          p.cqe.send.msn, r2

    phvwr          CAPRI_PHV_RANGE(TO_S7_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_IMPLIED_NAK_SEQ_ERR_OFFSET))

    sne            c1, K_REMAINING_PAYLOAD_BYTES, 0
    DMA_CMD_STATIC_BASE_GET_C(r3, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_SKIP_TO_EOP, c1)
    DMA_SKIP_CMD_SETUP_C(r3, 0 /* CMD_EOP */, 1 /* SKIP_TO_EOP */, c1)

    // If its implicit nak, bktrack dma cmd will have end_of_cmd so unset end_of_cmd for
    // lsn_or_rexmit_psn cmd
    DMA_CMD_STATIC_BASE_GET(r3, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_LSN_OR_REXMIT_PSN)
    DMA_UNSET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r3)

    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

    CAPRI_RESET_TABLE_2_ARG()
    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_offset), K_CUR_SGE_OFFSET, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_id), K_CUR_SGE_ID
    phvwrpair.e CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), d.e_psn, \
                CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_bktrack), 1, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1

invalid_syndrome:
invalid_nak_code:
    phvwr.e       CAPRI_PHV_FIELD(TO_S4_P, error_drop_phv), 1
    CAPRI_SET_TABLE_0_VALID(0)

invalid_read_or_atomic_rsp_msn:
    CAPRI_SET_TABLE_0_VALID(0)
    phvwr          CAPRI_PHV_FIELD(TO_S4_P, error_drop_phv), 1
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)
