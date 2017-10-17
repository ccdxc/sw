#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct sqcb1_t d;
struct req_rx_sqcb1_process_k_t k;

#define SQCB1_TO_RRQWQE_T struct req_rx_sqcb1_to_rrqwqe_info_t

%%

    .param    req_rx_rrqwqe_process

.align
req_rx_sqcb1_process:
    // TODO Check valid PSN

    // TODO bth.psn < sqcb1_p->rexmit_psn, duplicate ack, drop
    slt            c1, k.to_stage.bth_psn, d.rexmit_psn
    bcf            [c1], duplicate_ack

    // TODO Check pending_recirc_pkts_maxk
    sub            r1, d.token_id, d.nxt_to_go_token_id
    bgti           r1, PENDING_RECIR_PKTS_MAX, recirc_cnt_exceed

    // get token_id for this packet
    phvwr          p.my_token_id, d.token_id // Branch Delay Slot
    
    // check if its this packet's turn, if not recirc
    seq            c1, d.token_id, d.nxt_to_go_token_id
    bcf            [!c1], recirc
    tbladd         d.token_id, 1 // Branch Delay Slot

    add            r2, r0, k.global.flags
    ARE_ALL_FLAGS_SET(c2, r2, REQ_RX_FLAG_AETH)
    bcf            [!c2], set_arg
    nop            // Branch Delay Slot
 
    // if (msn >= sqcb1_p->ssn) invalid_pkt_msn
    scwle24        c3, d.ssn, k.to_stage.msn 
    bcf            [c3], invalid_pkt_msn
    nop            // Branch Delay Slot

    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_START)
    SQCB1_ADDR_GET(r3)

    IS_ANY_FLAG_SET_B(c3, r1, RNR_SYNDROME|RESV_SYNDROME|NAK_SYNDROME)
    bcf            [c3], post_rexmit_psn
    phvwr          p.rexmit_psn, k.to_stage.bth_psn // Branch Delay Slot
   
    // if ACK, rexmit_psn is ack_psn + 1
    add            r1, k.to_stage.bth_psn, 1
    phvwr          p.rexmit_psn, r1

    // if (sqcb1_p->lsn != ((1 << (sqcb1_p->credits >> 1)) + sqcb1_p->msn))
    //     doorbell_incr_pindex(fc_ring_id) 
    DECODE_NAK_SYNDROME_CREDITS(r2, k.to_stage.syndrome, c1)
    mincr          r2, 24, k.to_stage.msn
    sne            c1, d.lsn, r2

post_credits:
    // dma_cmd - msn and credits
    phvwr          p.msn, k.to_stage.msn
    phvwr          p.credits, k.to_stage.syndrome[4:0]

    add            r4, r3, SQCB1_MSN_OFFSET
    DMA_HBM_PHV2MEM_SETUP(r6, msn, credits, r4)
    bcf            [!c1], post_rexmit_psn
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)

    // dma_cmd - fc_ring db data
    PREPARE_DOORBELL_INC_PINDEX(k.global.lif, k.global.qtype, k.global.qid, FC_RING_ID, r1, r2)
    phvwr          p.db_data1, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, db_data1, db_data1, r1)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)

post_rexmit_psn:
    add            r4, r3, SQCB1_REXMIT_PSN_OFFSET
    DMA_HBM_PHV2MEM_SETUP(r6, rexmit_psn, rexmit_psn, r4)

set_arg:

    CAPRI_GET_TABLE_0_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, cur_sge_offset, d.rrqwqe_cur_sge_offset)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, cur_sge_id, d.rrqwqe_cur_sge_id)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, in_progress, d.in_progress)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, cq_id, d.cq_id)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, e_rsp_psn, d.e_rsp_psn)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, rrq_empty, k.args.rrq_empty)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, timer_active, d.timer_active)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, dma_cmd_start_index, k.args.dma_cmd_start_index)
    add            r2, k.args.rrq_cindex, r0
    mincr          r2, d.log_rrq_size, 1
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, rrq_cindex, r2)

    add            r5, d.rrq_base_addr, k.args.rrq_cindex, LOG_RRQ_WQE_SIZE
    CAPRI_GET_TABLE_0_K(req_rx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_rx_rrqwqe_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r5)

    nop.e
    nop

duplicate_ack:
recirc_cnt_exceed:
recirc:
invalid_pkt_msn:

    nop.e
    nop

