#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct rrqwqe_d_t d;
struct req_rx_rrqwqe_process_k_t k;

#define RRQWQE_TO_SGE_T struct req_rx_rrqwqe_to_sge_info_t
#define RRQWQE_TO_CQ_T struct req_rx_rrqwqe_to_cq_info_t
#define RRQSGE_TO_LKEY_T struct req_rx_rrqsge_to_lkey_info_t
#define SQCB1_WRITE_BACK_T struct req_rx_sqcb1_write_back_info_t

%%
    .param    req_rx_rrqsge_process
    .param    req_rx_rrqlkey_process
    .param    req_rx_sqcb1_write_back_process
    .param    req_rx_cqcb_process

.align
req_rx_rrqwqe_process:

    // TODO ack processing
    add            r5, r0, k.global.flags
    ARE_ALL_FLAGS_SET(c1, r5, REQ_RX_FLAG_ACK)

    bcf            [!c1], read_or_atomic
    ARE_ALL_FLAGS_SET(c4, r5, REQ_RX_FLAG_COMPLETION)  // Branch Delay Slot
ack:
    // Hardcode table 2 for write_back process
    SQCB1_ADDR_GET(r5)
    CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_rx_sqcb1_write_back_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r5)

    CAPRI_GET_TABLE_2_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, e_rsp_psn, k.args.e_rsp_psn)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, cur_sge_id, k.args.cur_sge_id)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, cur_sge_offset, k.args.cur_sge_offset)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, tbl_id, 2)

    CAPRI_SET_TABLE_0_VALID(0);

    // r1 = aeth_syndrome
    add            r1, k.to_stage.syndrome, r0

    // c2 = TRUE if rrq_empty
    seq            c2, k.args.rrq_empty, 1
    //phv_p->cqwqe.id.msn = pkt_msn
    //if (sqcb1_to_rrqwqe_info_p->rrq_empty == FALSE)
    //    phv_p->cqwqe.id.msn = min((rrqwqe_p->msn - 1), phv_p->aeth.msn)
    bcf            [c2], p_ack
    add            r2, k.to_stage.msn, r0 // Branch Delay Slot
    add            r2, d.msn, 0
    mincr          r2, 24, -1
    scwle24        c3, r2, k.to_stage.msn
    cmov           r2, c3, r2, k.to_stage.msn

p_ack:
    phvwr          p.cqwqe.id.msn, r2 // Branch Delay Slot

    IS_ANY_FLAG_SET_B(c3, r1, RNR_SYNDROME|RESV_SYNDROME|NAK_SYNDROME)
    bcf            [c3], n_ack
    nop            // Branch Delay Slot
  
    // if aeth contains already ack'ed msn, do not post CQ
    //scwle24        c1, r2, k.args.msn // Branch Delay Slot
    //bcf            [c1], end
    //nop            // Branch Delay Slot

    bcf            [c2], set_cqcb_arg
    // if (pkt_psn >= rrqwqe_p->psn)
    // implicit nak, ring bktrack ring setting rexmit_psn to rrqwqe_p->psn
    scwle24        c1, d.psn, k.to_stage.bth_psn // Branch Delay Slot
    phvwr.c1       p.rexmit_psn, d.psn
    CAPRI_SET_FIELD_C(r7, SQCB1_WRITE_BACK_T, post_bktrack, 1, c1)

    b              set_cqcb_arg
    nop            // Branch Delay Slot

n_ack:
    ARE_ALL_FLAGS_SET_B(c3, r1, NAK_SYNDROME)
    bcf            [!c3], rnr
    nop            // Branch Delay Slot

nak_seq_error:
    // SQ backtrack if NAK is due to SEQ_ERR    
    seq            c3, r1, NAK_SEQ_ERR_SYNDROME
    CAPRI_SET_FIELD_C(r7, SQCB1_WRITE_BACK_T, post_bktrack, 1, c3)

    b              set_cqcb_arg
    nop            // Branch Delay Slot

rnr:
    ARE_ALL_FLAGS_SET_B(c3, r1, RNR_SYNDROME)
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
    seq            c1, k.args.in_progress, 1 
    sne.c1         c2, k.to_stage.bth_psn, k.args.e_rsp_psn
    sne.!c1        c2, k.to_stage.bth_psn, d.psn
    bcf            [c2], out_of_order_rsp
    nop            // Branch Delay Slot
    
    ARE_ALL_FLAGS_SET(c3, r5, REQ_RX_FLAG_FIRST)

    // atomic or read requires explicit response so msn in atomic or
    // read_resp_last or read_resp_only should be >= the corresponding message's
    // sequence number
    scwlt24.c4     c2, k.to_stage.msn, d.msn
    bcf            [!c3 & c2], invalid_rsp_msn

    // if (first)
    //     min((rrqwqe_p->msn -1), sqcb1_to_rrqwqe_info_p->msn)
    // else
    //     min(rrqwqe_p->msn, sqcb1_to_rrqwqe_info_p->msn)
    add            r1, d.msn, 0 // Branch Delay Slot
    mincr.c3       r1, 24, -1
    scwle24        c2, r1, k.to_stage.msn
    cmov           r1, c2, r1, k.to_stage.msn
    phvwr          p.cqwqe.id.msn, r1
    
    seq            c2, d.read_rsp_or_atomic, RRQ_OP_TYPE_READ
    bcf            [!c2], atomic
    nop            // Branch Delay Slot

read:
    CAPRI_GET_TABLE_0_ARG(req_rx_phv_t, r7)
    //CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, is_atomic, 0)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, cur_sge_id, k.args.cur_sge_id)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, cur_sge_offset, k.args.cur_sge_offset)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD_C(r7, RRQWQE_TO_SGE_T, dma_cmd_eop, 1, !c4)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, rrq_cindex, k.args.rrq_cindex)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, dma_cmd_start_index, k.args.dma_cmd_start_index)
    sub            r3, d.num_sges, k.args.cur_sge_id
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, num_valid_sges, r3)
    cmov           r3, c1, k.args.e_rsp_psn, d.psn
    add            r3, r3, 1
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, e_rsp_psn, r3)

    CAPRI_GET_TABLE_0_K(req_rx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_rx_rrqsge_process)
    add            r3, d.read.wqe_sge_list_addr, k.args.cur_sge_id, LOG_SIZEOF_SGE_T
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r3)

    // if read_resp first contains already ack'ed msn, do not post CQ
    scwle24        c1, r1, k.args.msn
    bcf            [c1 & c3], end
    nop            // Branch Delay Slot

    b              set_cqcb_arg
    nop            // Branch Delay Slot

atomic:
    phvwr          p.cqwqe.op_type, d.atomic.op_type
    KT_BASE_ADDR_GET(r1, r2)
    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET())+
    //                     ((sge_p->lkey & KEY_INDEX_MASK) * sizeof(key_entry_t));
    add            r2, d.atomic.sge.l_key, r0
    KEY_ENTRY_ADDR_GET(r1, r1, r2)

    CAPRI_GET_TABLE_0_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, sge_va, d.atomic.sge.va)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, sge_bytes, d.atomic.sge.len)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, dma_cmd_eop, 0)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, dma_cmd_start_index, k.args.dma_cmd_start_index)
    //CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, sge_index, 0)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, cq_dma_cmd_index, REQ_RX_DMA_CMD_CQ)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, cq_id, k.args.cq_id)
    CAPRI_SET_FIELD(r7, RRQSGE_TO_LKEY_T, is_atomic, 1)

    CAPRI_GET_TABLE_0_K(req_rx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_rx_rrqlkey_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, r6, r1)

    // Hardcode table id 2 for write_back process
    // to keep it consistent with read process where
    // table 0 and 1 are taken for sge process
    CAPRI_GET_TABLE_2_ARG(req_rx_phv_t, r7)

    //CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, in_progress, 0)
    //CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, cur_sge_id, 0)
    //CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, cur_sge_offset, 0)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, e_rsp_psn, d.psn)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, tbl_id, 2)

    SQCB0_ADDR_GET(r1)
    add            r6, r1, RRQ_C_INDEX_OFFSET
    memwr.hx       r6, k.args.rrq_cindex

    SQCB1_ADDR_GET(r1)
    CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_rx_sqcb1_write_back_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

set_cqcb_arg:
    // if(!completion) goto end
    bcf            [!c4], end
    nop            // Branch Delay Slot 

    // Hardcode table id 3 for CQCB process
    CAPRI_GET_TABLE_3_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_CQ_T, tbl_id, 3)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_CQ_T, dma_cmd_index, REQ_RX_DMA_CMD_CQ)

    CAPRI_GET_TABLE_3_K(req_rx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_rx_cqcb_process)
    CQCB_ADDR_GET(r1, k.args.cq_id)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

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
