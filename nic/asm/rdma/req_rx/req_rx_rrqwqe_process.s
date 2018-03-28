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

#define K_E_RSP_PSN CAPRI_KEY_RANGE(IN_P, e_rsp_psn_sbit0_ebit7, e_rsp_psn_sbit16_ebit23)
#define K_MSN CAPRI_KEY_RANGE(IN_P, msn_sbit0_ebit7, msn_sbit16_ebit23)
#define K_CUR_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, cur_sge_offset_sbit0_ebit15, cur_sge_offset_sbit16_ebit31)
#define K_CUR_SGE_ID CAPRI_KEY_FIELD(IN_P, cur_sge_id)
#define K_RRQ_CINDEX CAPRI_KEY_RANGE(IN_P, rrq_cindex_sbit0_ebit0, rrq_cindex_sbit1_ebit7)
#define K_DMA_CMD_START_INDEX CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index)
#define K_CQ_ID CAPRI_KEY_RANGE(IN_P, cq_id_sbit0_ebit15, cq_id_sbit16_ebit23)
#define K_REXMIT_PSN CAPRI_KEY_RANGE(IN_P, rexmit_psn_sbit0_ebit0, rexmit_psn_sbit17_ebit23)

#define K_AETH_SYNDROME CAPRI_KEY_FIELD(IN_TO_S_P, aeth_syndrome)
#define K_AETH_MSN      CAPRI_KEY_FIELD(IN_TO_S_P, aeth_msn)
#define K_BTH_PSN  CAPRI_KEY_FIELD(IN_TO_S_P, bth_psn)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_TO_S_P, remaining_payload_bytes_sbit0_ebit5, remaining_payload_bytes_sbit14_ebit15)

%%
    .param    req_rx_rrqsge_process
    .param    req_rx_rrqlkey_process
    .param    req_rx_sqcb1_write_back_process
    .param    req_rx_cqcb_process

.align
req_rx_rrqwqe_process:

    // TODO ack processing
    add            r5, r0, K_GLOBAL_FLAGS
    ARE_ALL_FLAGS_SET(c1, r5, REQ_RX_FLAG_ACK)
    add            r6, K_REXMIT_PSN, r0

    bcf            [!c1], read_or_atomic
    ARE_ALL_FLAGS_SET(c4, r5, REQ_RX_FLAG_COMPLETION)  // Branch Delay Slot
ack:
    //SQCB1_ADDR_GET(r5)
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

    CAPRI_RESET_TABLE_3_ARG()
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, e_rsp_psn, K_E_RSP_PSN)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, cur_sge_id, K_CUR_SGE_ID)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, cur_sge_offset, K_CUR_SGE_OFFSET)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, rrq_in_progress, CAPRI_KEY_FIELD(IN_P, rrq_in_progress))
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, tbl_id, 3)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, rexmit_psn, r6)
    
    CAPRI_SET_TABLE_0_VALID(0);

    // r1 = aeth_syndrome
    add            r1, K_AETH_SYNDROME, r0

    //phv_p->cqwqe.id.msn = pkt_msn
    //if (sqcb1_to_rrqwqe_info_p->rrq_empty == FALSE)
    //    phv_p->cqwqe.id.msn = min((rrqwqe_p->msn - 1), phv_p->aeth.msn)
    bbeq           CAPRI_KEY_FIELD(IN_P, rrq_empty), 1, p_ack
    add            r2, K_AETH_MSN, r0 // Branch Delay Slot
    add            r2, d.msn, 0
    mincr          r2, 24, -1
    scwle24        c3, r2, K_AETH_MSN
    cmov           r2, c3, r2, K_AETH_MSN

p_ack:
    IS_MASKED_VAL_EQUAL(c3, r1, SYNDROME_MASK, ACK_SYNDROME)
    bcf            [!c3], n_ack
    nop            // Branch Delay Slot
  
    phvwr          p.cqwqe.id.msn, r2 // Branch Delay Slot

    bbeq           CAPRI_KEY_FIELD(IN_P, rrq_empty), 1, set_cqcb_arg
    // if (pkt_psn >= rrqwqe_p->psn)
    // implicit nak, ring bktrack ring setting rexmit_psn to rrqwqe_p->psn
    scwle24        c1, d.psn, K_BTH_PSN // Branch Delay Slot
    seq            c2, CAPRI_KEY_FIELD(IN_P, rrq_in_progress), 1
    cmov           r6, c2, K_E_RSP_PSN, d.psn
    phvwr          p.rexmit_psn, r6
    //phvwr.!c2      p.rexmit_psn, d.psn 
    //phvwr.c2       p.rexmit_psn, K_E_RSP_PSN
    CAPRI_SET_FIELD2_C(SQCB1_WRITE_BACK_P, post_bktrack, 1, c1)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, dma_cmd_eop, 1)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, rexmit_psn, r6)

    // If ack msn in implicit nak is already acked' do not post CQ
    scwle24        c1, r2, K_MSN
    bcf            [c1], end
    nop            // Branch Delay Slot
 
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, dma_cmd_eop, 0)

    b              set_cqcb_arg
    nop            // Branch Delay Slot

n_ack:
    IS_MASKED_VAL_EQUAL_B(c3, r1, SYNDROME_MASK, NAK_SYNDROME)
    bcf            [!c3], rnr
    nop            // Branch Delay Slot

nak_seq_error:
    // SQ backtrack if NAK is due to SEQ_ERR    
    IS_MASKED_VAL_EQUAL_B(c3, r1, NAK_CODE_MASK, NAK_CODE_SEQ_ERR)
    CAPRI_SET_FIELD2_C(SQCB1_WRITE_BACK_P, post_bktrack, 1, c3)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, dma_cmd_eop, 1)

    // if implicit ack MSN in NAK matches already ack'ed msn, do not post CQ
    sub            r2, r2, 1
    scwle24        c1, r2, K_MSN
    bcf            [c1], end
    phvwr          p.cqwqe.id.msn, r2 // Branch Delay Slot

    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, dma_cmd_eop, 0)

    b              set_cqcb_arg
    nop            // Branch Delay Slot

rnr:
    IS_MASKED_VAL_EQUAL_B(c3, r1, SYNDROME_MASK, RNR_SYNDROME)
    bcf            [!c3], invalid_syndrome
    nop            // Branch Delay Slot

    // TODO Start timer if timer is not active currently 

    b              set_cqcb_arg
    nop            // Branch Delay Slot

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
    
    seq            c2, d.read_rsp_or_atomic, RRQ_OP_TYPE_READ
    bcf            [!c2], atomic
    nop            // Branch Delay Slot

read:
    add            r3, d.read.wqe_sge_list_addr, K_CUR_SGE_ID, LOG_SIZEOF_SGE_T
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqsge_process, r3)

    CAPRI_RESET_TABLE_0_ARG()
    //CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, is_atomic, 0)
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, remaining_payload_bytes, K_REMAINING_PAYLOAD_BYTES)
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, cur_sge_id, K_CUR_SGE_ID)
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, cur_sge_offset, K_CUR_SGE_OFFSET)
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, rrq_in_progress, CAPRI_KEY_FIELD(IN_P, rrq_in_progress))
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, dma_cmd_eop, 1)
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, rrq_cindex, K_RRQ_CINDEX)
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, dma_cmd_start_index, K_DMA_CMD_START_INDEX)
    sub            r3, d.num_sges, K_CUR_SGE_ID
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, num_valid_sges, r3)
    cmov           r3, c1, K_E_RSP_PSN, d.psn
    add            r3, r3, 1
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, e_rsp_psn, r3)
    CAPRI_SET_FIELD2(RRQWQE_TO_SGE_P, rexmit_psn, r6)

    // if read_resp first contains already ack'ed msn, do not post CQ
    scwle24        c1, r1, K_MSN
    bcf            [c1 & c3], end
    nop            // Branch Delay Slot

    // set cmd_eop at the end of data transfer if there is no CQ posting
    CAPRI_SET_FIELD2_C(RRQWQE_TO_SGE_P, dma_cmd_eop, 0, c4)

    b              set_cqcb_arg
    nop            // Branch Delay Slot

atomic:
    phvwr          p.cqwqe.op_type, d.atomic.op_type
    KT_BASE_ADDR_GET2(r1, r2)
    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+
    //                     ((sge_p->lkey & KEY_INDEX_MASK) * sizeof(key_entry_t));
    add            r2, d.atomic.sge.l_key, r0
    KEY_ENTRY_ADDR_GET(r1, r1, r2)

    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD2(RRQSGE_TO_LKEY_P, sge_va, d.atomic.sge.va)
    CAPRI_SET_FIELD2(RRQSGE_TO_LKEY_P, sge_bytes, d.atomic.sge.len)
    CAPRI_SET_FIELD2(RRQSGE_TO_LKEY_P, dma_cmd_eop, 0)
    CAPRI_SET_FIELD2(RRQSGE_TO_LKEY_P, dma_cmd_start_index, K_DMA_CMD_START_INDEX)
    //CAPRI_SET_FIELD2(RRQSGE_TO_LKEY_P, sge_index, 0)
    CAPRI_SET_FIELD2(RRQSGE_TO_LKEY_P, cq_dma_cmd_index, REQ_RX_DMA_CMD_CQ)
    CAPRI_SET_FIELD2(RRQSGE_TO_LKEY_P, cq_id, K_CQ_ID)
    CAPRI_SET_FIELD2(RRQSGE_TO_LKEY_P, is_atomic, 1)

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, req_rx_rrqlkey_process, r1)

    // Hardcode table id 2 for write_back process
    // to keep it consistent with read process where
    // table 0 and 1 are taken for sge process
    CAPRI_RESET_TABLE_3_ARG()

    //CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, in_progress, 0)
    //CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, cur_sge_id, 0)
    //CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, cur_sge_offset, 0)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, e_rsp_psn, d.psn)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, last_pkt, 1)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, tbl_id, 3)
    CAPRI_SET_FIELD2(SQCB1_WRITE_BACK_P, rexmit_psn, r6)

    //SQCB0_ADDR_GET(r1)
    //add            r6, r1, RRQ_C_INDEX_OFFSET
    //memwr.hx       r6, K_RRQ_CINDEX

    //SQCB1_ADDR_GET(r1)
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_sqcb1_write_back_process, r0)

set_cqcb_arg:
    // if(!completion) goto end
    bcf            [!c4], end
    nop            // Branch Delay Slot 

    // Hardcode table id 2 for CQCB process
    CAPRI_RESET_TABLE_2_ARG()

    CAPRI_SET_FIELD2(RRQWQE_TO_CQ_P, cq_id, K_CQ_ID) 
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_cqcb_process, r0)

end:
    nop.e
    nop

out_of_order_rsp:
invalid_syndrome:
invalid_rsp_msn:
    phvwr         p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)

    nop.e
    nop
