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
    // if (msn >= sqcb1_p->ssn) invalid_pkt_msn
    sle            c3, d.ssn, k.to_stage.msn 
    bcf            [c3], invalid_pkt_msn
    nop

    tblwr          d.msn, k.to_stage.msn
    // TODO credits settings
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

    add            r1, d.rrq_base_addr, k.args.rrq_cindex, LOG_RRQ_WQE_SIZE
    CAPRI_GET_TABLE_0_K(req_rx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_rx_rrqwqe_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

    nop.e
    nop

duplicate_ack:
recirc_cnt_exceed:
recirc:
invalid_pkt_msn:

    nop.e
    nop

