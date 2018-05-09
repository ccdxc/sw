#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct rrqwqe_d_t d;
struct req_rx_s1_t0_k k;

#define RRQWQE_TO_SGE_P t0_s2s_rrqwqe_to_sge_info
#define RRQWQE_TO_CQ_P t2_s2s_rrqwqe_to_cq_info
#define RRQSGE_TO_LKEY_P t0_s2s_rrqsge_to_lkey_info
#define SQCB1_WRITE_BACK_P t3_s2s_sqcb1_write_back_info

#define IN_P t0_s2s_sqcb1_to_rrqwqe_info
#define IN_TO_S_P to_s1_to_stage

#define K_E_RSP_PSN CAPRI_KEY_RANGE(IN_P, e_rsp_psn_or_ssn_sbit0_ebit7, e_rsp_psn_or_ssn_sbit16_ebit23)
#define K_SSN       CAPRI_KEY_RANGE(IN_P, e_rsp_psn_or_ssn_sbit0_ebit7, e_rsp_psn_or_ssn_sbit16_ebit23)
#define K_MSN CAPRI_KEY_RANGE(IN_P, msn_sbit0_ebit7, msn_sbit16_ebit23)
#define K_CUR_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, cur_sge_offset_sbit0_ebit7, cur_sge_offset_sbit16_ebit31)
#define K_CUR_SGE_ID CAPRI_KEY_FIELD(IN_P, cur_sge_id)
#define K_RRQ_CINDEX CAPRI_KEY_RANGE(IN_P, rrq_cindex_sbit0_ebit0, rrq_cindex_sbit1_ebit7)
#define K_DMA_CMD_START_INDEX CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index)
#define K_CQ_ID CAPRI_KEY_RANGE(IN_P, cq_id_sbit0_ebit15, cq_id_sbit16_ebit23)
#define K_REXMIT_PSN CAPRI_KEY_RANGE(IN_P, rexmit_psn_sbit0_ebit0, rexmit_psn_sbit17_ebit23)

#define K_AETH_SYNDROME CAPRI_KEY_FIELD(IN_TO_S_P, aeth_syndrome)
#define K_AETH_MSN      CAPRI_KEY_FIELD(IN_TO_S_P, aeth_msn)
#define K_BTH_PSN  CAPRI_KEY_FIELD(IN_TO_S_P, bth_psn)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_TO_S_P, remaining_payload_bytes_sbit0_ebit3, remaining_payload_bytes_sbit12_ebit13)

%%
    .param    req_rx_rrqsge_process
    .param    req_rx_rrqlkey_process
    .param    req_rx_sqcb1_write_back_process
    .param    req_rx_cqcb_process

.align
req_rx_rrqwqe_process:

    add            r5, r0, K_GLOBAL_FLAGS
    ARE_ALL_FLAGS_SET(c1, r5, REQ_RX_FLAG_ACK)
    add            r6, K_REXMIT_PSN, r0

    bcf            [!c1], read_or_atomic
    ARE_ALL_FLAGS_SET(c4, r5, REQ_RX_FLAG_COMPLETION)  // Branch Delay Slot

ack_or_nak_or_rnr_or_implicit_nak:
    //SQCB1_ADDR_GET(r5)
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

    CAPRI_RESET_TABLE_3_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, e_rsp_psn), K_E_RSP_PSN, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_offset), K_CUR_SGE_OFFSET, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, cur_sge_id), K_CUR_SGE_ID
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rrq_in_progress), CAPRI_KEY_FIELD(IN_P, rrq_in_progress), \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, tbl_id), 3
    phvwr     CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6
    
    CAPRI_SET_TABLE_0_VALID(0)

    // r1 = aeth_syndrome
    add            r1, K_AETH_SYNDROME, r0

    // MSN posted in CQ should be smaller than SSN. In case
    // of ack drops, response to retried request can have PSN/MSN 
    // logically greater than PSN/MSN of the retried request but
    // that should not result in completion of WQEs more than those
    // that have been retransmitted
    scwlt24        c3, K_AETH_MSN, K_SSN
    cmov           r2, c3, K_AETH_MSN, K_SSN

    // This is an ack/nak/rnr packet. If rrq_empty (no outstanding
    // read or atomic response) then msn in this ack/nak/rnr 
    // should be completed. if rrq is not empty, then
    // receiving ack means read/atomic responses are missing, so
    // completion should be posted for lowest of msn in the ack or 
    // msn of the message before oldest read/atomic request in rrq
    bbeq           CAPRI_KEY_FIELD(IN_P, rrq_empty), 1, ack_or_nak_or_rnr
    mincr.!c3      r2, 24, -1 // Branch Delay Slot
    add            r2, d.msn, 0
    mincr          r2, 24, -1
    scwle24        c5, r2, K_AETH_MSN
    cmov           r2, c5, r2, K_AETH_MSN

    // if (pkt_psn >= rrqwqe_p->psn)
    // implicit nak, ring bktrack ring setting rexmit_psn to rrqwqe_p->psn
    scwle24        c5, d.psn, K_BTH_PSN // Branch Delay Slot
    bcf            [!c5], ack_or_nak_or_rnr

implicit_nak:
    // PSN of the next expected read/atomic response is start psn 
    // contained in rrq wqe or exp_rsp_psn stored
    // in sqcb1 if already few read response packets were received
    seq            c2, CAPRI_KEY_FIELD(IN_P, rrq_in_progress), 1
    cmov           r6, c2, K_E_RSP_PSN, d.psn

    phvwr          p.rexmit_psn, r6 
    phvwr          p.msn, r2
    phvwr          p.cqwqe.id.msn, r2

    // If ack msn in implicit nak is already acked' do not post CQ
    scwle24        c1, r2, K_MSN
             
    phvwrpair      CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2

    bcf            [!c1],  set_cqcb_arg
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, post_bktrack), 1

    b              end
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, dma_cmd_eop), 1

ack_or_nak_or_rnr:
    phvwr          p.msn, r2
    phvwr          p.cqwqe.id.msn, r2
    IS_MASKED_VAL_EQUAL(c3, r1, SYNDROME_MASK, ACK_SYNDROME)
    bcf            [!c3], nak_or_rnr
  
ack:
    add            r6, K_BTH_PSN, 1 // Branch Delay Slot
    phvwr          p.rexmit_psn, r6

    b              set_cqcb_arg
    phvwrpair      CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2

nak_or_rnr:
    scwle24        c1, r2, K_MSN

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
    phvwr.c3       p.cqwqe.status, CQ_STATUS_REMOTE_INV_REQ_ERR

    IS_MASKED_VAL_EQUAL_B(c3, r1, NAK_CODE_MASK, NAK_CODE_REM_ACC_ERR)
    phvwr.c3       p.cqwqe.status, CQ_STATUS_REMOTE_ACC_ERR

    IS_MASKED_VAL_EQUAL_B(c3, r1, NAK_CODE_MASK, NAK_CODE_REM_OP_ERR)
    phvwr.c3       p.cqwqe.status, CQ_STATUS_REMOTE_OPER_ERR

    // post err completion for msn one more than the one last completed
    // as specified in NAK
    setcf          c1, [!c0]
    add            r3, K_AETH_MSN, 1
    phvwr          p.cqwqe.id.msn, r3

nak_completion:
    bcf            [!c1], set_cqcb_arg
    phvwrpair      CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), K_BTH_PSN, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2

    b              end
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, dma_cmd_eop), 1

rnr:
    IS_MASKED_VAL_EQUAL_B(c3, r1, SYNDROME_MASK, RNR_SYNDROME)
    bcf            [!c3], invalid_syndrome
    nop            // Branch Delay Slot

    // TODO compute timeout from rnr syndrome

    bcf            [!c1], set_cqcb_arg
    phvwrpair      CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), K_BTH_PSN, \
                   CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r2

    b              end
    phvwr          CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, dma_cmd_eop), 1

read_or_atomic:
    // if (in_progress) 
    //     pkt->psn == sqcb1_p->e_rsp_psn
    //  else
    //     pkt->psn == rrqwqe_p->psn 
    seq            c1, CAPRI_KEY_FIELD(IN_P, rrq_in_progress), 1 
    sne.c1         c2, K_BTH_PSN, K_E_RSP_PSN
    sne.!c1        c2, K_BTH_PSN, d.psn
    bcf            [c2], out_of_order_rsp
    add            r5, r0, K_GLOBAL_FLAGS //Branch Delay Slot
    
    ARE_ALL_FLAGS_SET(c3, r5, REQ_RX_FLAG_FIRST)

    // atomic or read requires explicit response so msn in atomic or
    // read_resp_last or read_resp_only should be >= the corresponding message's
    // sequence number
    scwlt24.c4     c2, K_AETH_MSN, d.msn
    bcf            [!c3 & c2], invalid_rsp_msn

    // if (first)
    //     min((rrqwqe_p->msn -1), sqcb1_to_rrqwqe_info_p->msn)
    // else
    //     min(rrqwqe_p->msn, sqcb1_to_rrqwqe_info_p->msn)
    add            r1, d.msn, 0 // Branch Delay Slot
    mincr.c3       r1, 24, -1
    scwle24        c2, r1, K_AETH_MSN
    cmov           r1, c2, r1, K_AETH_MSN

    phvwr          p.cqwqe.id.msn, r1
    phvwr          p.msn, r1
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
    phvwrpair CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_eop), 1, \
              CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, rrq_cindex), K_RRQ_CINDEX
    sub            r3, d.num_sges, K_CUR_SGE_ID
    phvwrpair CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, num_valid_sges), r3, \
              CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_start_index), K_DMA_CMD_START_INDEX
    cmov           r3, c1, K_E_RSP_PSN, d.psn
    add            r3, r3, 1
    phvwrpair CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, e_rsp_psn), r3, \
              CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, rexmit_psn), r6
    phvwr     CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, msn), r1

    // if read_resp first contains already ack'ed msn, do not post CQ
    scwle24        c1, r1, K_MSN
    bcf            [c1 & c3], end
    nop            // Branch Delay Slot

    // set cmd_eop at the end of data transfer if there is no CQ posting
    phvwr.c4  CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_eop), 0

    b              set_cqcb_arg
    nop            // Branch Delay Slot

zero_length_read:

    CAPRI_SET_TABLE_0_VALID(0)
    // to keep it consistent with read process where
    // table 0 and 1 are taken for sge process
    CAPRI_RESET_TABLE_3_ARG()

    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, e_rsp_psn), d.psn, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, last_pkt), 1, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, tbl_id), 3
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r1
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

    b              set_cqcb_arg
    nop            // Branch Delay Slot

atomic:
    phvwr          p.cqwqe.op_type, d.atomic.op_type
    KT_BASE_ADDR_GET2(r3, r2)
    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+
    //                     ((sge_p->lkey & KEY_INDEX_MASK) * sizeof(key_entry_t));
    add            r2, d.atomic.sge.l_key, r0
    KEY_ENTRY_ADDR_GET(r3, r3, r2)

    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, sge_va), d.atomic.sge.va, \
              CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, sge_bytes), d.atomic.sge.len[15:0]
    phvwrpair CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, dma_cmd_eop), 0, \
              CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, dma_cmd_start_index), K_DMA_CMD_START_INDEX
    //phvwr CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, sge_index), 0
    phvwrpair CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, cq_dma_cmd_index), REQ_RX_DMA_CMD_CQ, \
              CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, cq_id), K_CQ_ID
    phvwr     CAPRI_PHV_FIELD(RRQSGE_TO_LKEY_P, is_atomic), 1

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, req_rx_rrqlkey_process, r3)

    // Hardcode table id 2 for write_back process
    // to keep it consistent with read process where
    // table 0 and 1 are taken for sge process
    CAPRI_RESET_TABLE_3_ARG()

    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, e_rsp_psn), d.psn, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, last_pkt), 1, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, tbl_id), 3
    phvwrpair CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, rexmit_psn), r6, \
              CAPRI_PHV_FIELD(SQCB1_WRITE_BACK_P, msn), r1

    //SQCB0_ADDR_GET(r1)
    //add            r6, r1, RRQ_C_INDEX_OFFSET
    //memwr.hx       r6, K_RRQ_CINDEX

    //SQCB1_ADDR_GET(r1)
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

set_cqcb_arg:
    // if(!completion) goto end
    bcf            [!c4], end
    nop            // Branch Delay Slot 

    // Re-load err_retry, rnr_retry counter and set rnr_timeout to 0 if
    // outstanding request is cleared. Otherwise, keep decrementing the
    // err_retry_count upon retrans timer expiry or NAK (seq_err) or
    // implicit NAK, or rnr_retry_count upon rnr timer expiry
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_REXMIT_PSN)
    DMA_HBM_PHV2MEM_PHV_END_SETUP(r6, rnr_retry_ctr)

    SQCB2_ADDR_GET(r3) 
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_RNR_TIMEOUT)
    add            r2, r3, SQCB2_RNR_TIMEOUT_OFFSET
    DMA_HBM_PHV2MEM_SETUP(r6, rnr_timeout, rnr_timeout, r2) 

    // Hardcode table id 2 for CQCB process
    CAPRI_RESET_TABLE_2_ARG()

    phvwr CAPRI_PHV_FIELD(RRQWQE_TO_CQ_P, cq_id), K_CQ_ID
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_cqcb_process, r0)

end:
    nop.e
    nop

out_of_order_rsp:
invalid_syndrome:
invalid_rsp_msn:
invalid_nak_code:
    phvwr         p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)

    nop.e
    nop
