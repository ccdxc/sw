#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct rrqwqe_d_t d;
struct req_rx_s1_t0_k k;

#define RRQWQE_TO_SGE_P t0_s2s_rrqwqe_to_sge_info
#define RRQWQE_TO_CQ_P t2_s2s_rrqwqe_to_cq_info
#define RRQSGE_TO_LKEY_P t0_s2s_rrqsge_to_lkey_info
#define SQCB1_WRITE_BACK_P t2_s2s_sqcb1_write_back_info

#define IN_P t0_s2s_sqcb1_to_rrqwqe_info
#define IN_TO_S_P to_s1_rrqwqe_info

#define K_E_RSP_PSN CAPRI_KEY_RANGE(IN_P, e_rsp_psn_sbit0_ebit15, e_rsp_psn_sbit16_ebit23)
#define K_SSN       CAPRI_KEY_RANGE(IN_P, ssn_sbit0_ebit7, ssn_sbit16_ebit23)
#define K_MSN CAPRI_KEY_RANGE(IN_P, msn_sbit0_ebit7, msn_sbit16_ebit23)
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

#define TO_S1_RECIRC_P to_s1_recirc_info
%%
    .param    req_rx_rrqsge_process
    .param    req_rx_rrqlkey_process
    .param    req_rx_sqcb1_write_back_process
    .param    req_rx_cqcb_process

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
    scwle24        c5, d.psn, K_BTH_PSN // Branch Delay Slot
    bcf            [c5 & c6], implicit_nak
ack_or_nak_or_rnr:
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

    CAPRI_RESET_TABLE_2_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, e_rsp_psn), K_E_RSP_PSN, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_offset), K_CUR_SGE_OFFSET, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_id), K_CUR_SGE_ID
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rrq_in_progress), CAPRI_KEY_FIELD(IN_P, rrq_in_progress), \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, tbl_id), 3
    
    phvwr          p.cqe.send.msn, r2
    IS_MASKED_VAL_EQUAL(c3, r1, SYNDROME_MASK, ACK_SYNDROME)
    bcf            [c6 & !c3], nak_or_rnr
  
ack:
    cmov           r6, c6, K_BTH_PSN, K_TX_PSN // Branch Delay Slot
    mincr.c6       r6, 24, 1
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1
    phvwr.e        p.rexmit_psn, r6
    phvwrpair      CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2

nak_or_rnr:
    scwlt24        c1, K_MSN, r2

    IS_MASKED_VAL_EQUAL_B(c3, r1, SYNDROME_MASK, NAK_SYNDROME)
    bcf            [!c3], rnr
    phvwr          p.rexmit_psn, K_BTH_PSN // Branch Delay Slot

nak:
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

    IS_MASKED_VAL_EQUAL_B(c3, r1, NAK_CODE_MASK, NAK_CODE_REM_ACC_ERR)
    phvwrpair.c3   p.cqe.status, CQ_STATUS_REMOTE_ACC_ERR, \
                   p.cqe.error, 1

    IS_MASKED_VAL_EQUAL_B(c3, r1, NAK_CODE_MASK, NAK_CODE_REM_OP_ERR)
    phvwrpair.c3   p.cqe.status, CQ_STATUS_REMOTE_OPER_ERR, \
                   p.cqe.error, 1

    // post err completion for msn one more than the one last completed
    // as specified in NAK
    setcf          c1, [c0]
    add            r3, K_AETH_MSN, 1
    phvwr          p.cqe.send.msn, r3

nak_completion:
    phvwrpair.e    CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), K_BTH_PSN, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2
    phvwr.c1       CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1

rnr:
    IS_MASKED_VAL_EQUAL_B(c3, r1, SYNDROME_MASK, RNR_SYNDROME)
    bcf            [!c3], invalid_syndrome
    nop            // Branch Delay Slot

    // TODO compute timeout from rnr syndrome
    phvwr          p.rnr_timeout, r1[4:0]

    phvwrpair.e    CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), K_BTH_PSN, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2
    phvwr.c1       CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1

read_or_atomic_or_implicit_nak:
    ARE_ALL_FLAGS_SET(c3, r5, REQ_RX_FLAG_FIRST) // Branch Delay Slot

    // atomic or read requires explicit response so msn in atomic or
    // read_resp_last or read_resp_only should be >= the corresponding message's
    // sequence number
    scwlt24.c4     c2, K_AETH_MSN, d.msn
    bcf            [!c3 & c2], invalid_rsp_msn

    // if (in_progress) 
    //     pkt->psn == sqcb1_p->e_rsp_psn
    //  else
    //     pkt->psn == rrqwqe_p->psn 
    seq            c1, CAPRI_KEY_FIELD(IN_P, rrq_in_progress), 1  // Branch Delay Slot
    sne.c1         c2, K_BTH_PSN, K_E_RSP_PSN
    sne.!c1        c2, K_BTH_PSN, d.psn
    bcf            [c2], implicit_nak

read_or_atomic:
    // if (first)
    //     min((rrqwqe_p->msn -1), sqcb1_to_rrqwqe_info_p->msn)
    // else
    //     min(rrqwqe_p->msn, sqcb1_to_rrqwqe_info_p->msn)
    add            r1, d.msn, 0 // Branch Delay Slot
    mincr.c3       r1, 24, -1
    scwle24        c2, r1, K_AETH_MSN
    cmov           r1, c2, r1, K_AETH_MSN

    phvwr          p.cqe.send.msn, r1
    add            r6, K_BTH_PSN, 1
    phvwr          p.rexmit_psn, r6
    // If its mid packet, retain the last ack msn written in sqcb1
    add.!c4        r1, K_MSN, 0
    
    seq            c2, d.read_rsp_or_atomic, RRQ_OP_TYPE_READ
    bcf            [!c2], atomic
    // Zero length read resp should only post completion and no data
    // transfer
    seq            c2, d.num_sges, r0 // Branch Delay Slot
    bcf            [c2], zero_length_read 

read:
    add            r3, d.read.wqe_sge_list_addr, K_CUR_SGE_ID, LOG_SIZEOF_SGE_T // Branch Delay Slot
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqsge_process, r3)

    CAPRI_RESET_TABLE_0_ARG()
    //phvwr CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, is_atomic), 0
    phvwrpair CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, remaining_payload_bytes), K_REMAINING_PAYLOAD_BYTES, \
              CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, cur_sge_id), K_CUR_SGE_ID
    phvwrpair CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, cur_sge_offset), K_CUR_SGE_OFFSET, \
              CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, rrq_in_progress), CAPRI_KEY_FIELD(IN_P, rrq_in_progress)
    sub            r3, d.num_sges, K_CUR_SGE_ID
    phvwrpair CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, num_valid_sges), r3, \
              CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_start_index), K_DMA_CMD_START_INDEX

    // if read_resp contains already ack'ed msn, do not post CQ
    // ideally this should happen only with read_resp_first. For read_resp_last
    // or read_resp_only, msn should always be the un-acked msn
    scwlt24.c4 c4, K_MSN, r1
    phvwr.!c4  CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_eop), 1

    // Just in case if there are more than 2 sges to be processed for this
    // phv, recirc path requires sge_list_addr, so populate it here as only
    // wqe stage is aware of this address
    phvwr     CAPRI_PHV_FIELD(TO_S1_RECIRC_P, rrqwqe_sge_list_addr), d.read.wqe_sge_list_addr

    CAPRI_RESET_TABLE_2_ARG()
    cmov           r3, c1, K_E_RSP_PSN, d.psn
    add            r3, r3, 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, e_rsp_psn), r3, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6
    phvwr.e   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r1
    phvwr.c4  CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1

zero_length_read:

    CAPRI_SET_TABLE_0_VALID(0)
    // to keep it consistent with read process where
    // table 0 and 1 are taken for sge process
    CAPRI_RESET_TABLE_2_ARG()

    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, e_rsp_psn), d.psn, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, last_pkt), 1, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, tbl_id), 3
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r1
    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

atomic:
    #phvwr          p.cqe.op_type, d.atomic.op_type
    KT_BASE_ADDR_GET2(r3, r2)
    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+
    //                     ((sge_p->lkey >> KEY_INDEX_SHIFT) * sizeof(key_entry_t));
    add            r2, d.atomic.sge.l_key, r0
    KEY_ENTRY_ADDR_GET(r3, r3, r2)

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

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqlkey_process, r3)

    // Hardcode table id 2 for write_back process
    // to keep it consistent with read process where
    // table 0 and 1 are taken for sge process
    CAPRI_RESET_TABLE_2_ARG()

    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, e_rsp_psn), d.psn, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, last_pkt), 1, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, tbl_id), 3
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r1
    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

end:
    nop.e
    nop

implicit_nak:
    // PSN of the next expected read/atomic response is start psn 
    // contained in rrq wqe or exp_rsp_psn stored
    // in sqcb1 if already few read response packets were received
    seq            c2, CAPRI_KEY_FIELD(IN_P, rrq_in_progress), 1
    cmov           r6, c2, K_E_RSP_PSN, d.psn
    phvwr          p.rexmit_psn, r6 
    
    // if its implicit nak, completion can be posted till wqe before
    // the first outstanding read request wqe
    add            r2, d.msn, 0
    mincr          r2, 24, -1
    phvwr          p.cqe.send.msn, r2

    sne            c1, K_REMAINING_PAYLOAD_BYTES, 0
    DMA_CMD_STATIC_BASE_GET_C(r3, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_SKIP_TO_EOP, c1)
    DMA_SKIP_CMD_SETUP_C(r3, 0 /* CMD_EOP */, 1 /* SKIP_TO_EOP */, c1)

    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

    CAPRI_RESET_TABLE_2_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, e_rsp_psn), K_E_RSP_PSN, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_offset), K_CUR_SGE_OFFSET, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_id), K_CUR_SGE_ID
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rrq_in_progress), CAPRI_KEY_FIELD(IN_P, rrq_in_progress), \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, tbl_id), 3
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2

    // If ack msn in implicit nak is already acked' do not post CQ
    scwlt24        c2, K_MSN, r2
    phvwr.c2       CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_cq), 1
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_bktrack), 1

    // set cmd_eop if skip_to_eop cmd exists and cq is not posted
    setcf.e        c1, [c1 & !c2]
    DMA_SKIP_CMD_SETUP_C(r3, 1 /* CMD_EOP */, 1 /* SKIP_TO_EOP */, c1)

invalid_syndrome:
invalid_rsp_msn:
invalid_nak_code:
    phvwr         p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)

    nop.e
    nop
