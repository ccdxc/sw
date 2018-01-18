#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct sqcb1_t d;
struct req_rx_sqcb1_process_k_t k;

#define SQCB1_TO_RRQWQE_T struct req_rx_sqcb1_to_rrqwqe_info_t
#define RRQWQE_TO_CQ_T struct req_rx_rrqwqe_to_cq_info_t
#define ECN_INFO_T struct req_rx_ecn_info_t

%%

    .param    req_rx_rrqwqe_process
    .param    req_rx_cqcb_process
    .param    req_rx_dcqcn_ecn_process

.align
req_rx_sqcb1_process:

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

    // invoke program to generate CNP using table 2, if ecn is set
    bbeq      k.args.ecn_set, 0, process_rx_pkt
    // Load dcqcn_cb to store timestamps and trigger Doorbell to generate CNP.
    CAPRI_GET_TABLE_2_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, ECN_INFO_T, p_key, k.args.p_key)

    add     r1, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT //dcqcn_cb addr
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_rx_dcqcn_ecn_process, r1)

process_rx_pkt:
    SQCB1_ADDR_GET(r5)
    bbeq           k.global.flags.req_rx._feedback, 1, process_feedback
    add            r2, r0, k.global.flags //Branch Delay Slot

    // TODO Check valid PSN

    // bth.psn >= sqcb1_p->rexmit_psn, valid response
    scwlt24        c1, k.to_stage.bth_psn, d.rexmit_psn

    ARE_ALL_FLAGS_SET(c2, r2, REQ_RX_FLAG_AETH)
    bcf            [!c2 & c1], duplicate_read_resp_mid
    // rexmit_psn is resp psn + 1
    add            r1, k.to_stage.bth_psn, 1 // Branch Delay Slot

    // skip ack sanity checks if there is no aeth hdr
    bcf            [!c2], post_rexmit_psn
    phvwr          p.rexmit_psn, r1 // Branch Delay Slot
 
check_ack_sanity:
    // if (msn >= sqcb1_p->ssn) invalid_pkt_msn
    scwle24        c3, d.ssn, k.to_stage.msn 
    bcf            [c3], invalid_pkt_msn

    add            r3, k.to_stage.syndrome, r0

    ARE_ALL_FLAGS_SET(c2, r2, REQ_RX_FLAG_ACK) // Branch Delay Slot
    bcf            [!c1], process_aeth

    IS_MASKED_VAL_EQUAL_B(c4, r3, SYNDROME_MASK, ACK_SYNDROME) // Branch Delay Slot

    // bth.psn < sqcb1_p->rexmit_psn, duplicate and not unsolicited p_ack, drop
    bcf            [!c2 | !c4], duplicate_ack

    // unsolicited ack i.e. duplicate of most recent p_ack is allowed
    sub            r4, d.rexmit_psn, -1  // Branch Delay Slot
    mincr          r4, 24, r0
    seq            c3, r4, k.to_stage.bth_psn
    bcf            [!c3], duplicate_ack

process_aeth:
    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_MSN_CREDITS)

    bcf            [!c4], post_rexmit_psn
    phvwr.!c4      p.rexmit_psn, k.to_stage.bth_psn // Branch Delay Slot

    // if (sqcb1_p->lsn != ((1 << (sqcb1_p->credits >> 1)) + sqcb1_p->msn))
    //     doorbell_incr_pindex(fc_ring_id) 
    DECODE_ACK_SYNDROME_CREDITS(r2, k.to_stage.syndrome, c1)
    mincr          r2, 24, k.to_stage.msn
    sne            c1, d.lsn, r2

post_credits:
    // dma_cmd - msn and credits
    add            r4, r5, SQCB1_MSN_OFFSET
    DMA_HBM_PHV2MEM_SETUP(r6, msn, credits, r4)
    bcf            [!c1], post_rexmit_psn
    phvwrpair      p.msn, k.to_stage.msn, p.credits, k.to_stage.syndrome[4:0]

    // dma_cmd - fc_ring db data
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_FC_DB)
    PREPARE_DOORBELL_INC_PINDEX(k.global.lif, k.global.qtype, k.global.qid, FC_RING_ID, r1, r2)
    phvwr          p.db_data1, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, db_data1, db_data1, r1)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)

post_rexmit_psn:
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_REXMIT_PSN)
    add            r4, r5, SQCB1_REXMIT_PSN_OFFSET
    DMA_HBM_PHV2MEM_SETUP(r6, rexmit_psn, rexmit_psn, r4)
    bcf            [c3], unsolicited_ack
    nop            // Branch Delay Slot

set_arg:

    CAPRI_GET_TABLE_0_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, cur_sge_offset, d.rrqwqe_cur_sge_offset)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, cur_sge_id, d.rrqwqe_cur_sge_id)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, in_progress, d.in_progress)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, cq_id, d.cq_id)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, e_rsp_psn, d.e_rsp_psn)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, msn, d.msn)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, rrq_empty, k.args.rrq_empty)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, timer_active, d.timer_active)
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, dma_cmd_start_index, k.args.dma_cmd_start_index)
    add            r2, k.args.rrq_cindex, r0
    mincr          r2, d.log_rrq_size, 1
    CAPRI_SET_FIELD(r7, SQCB1_TO_RRQWQE_T, rrq_cindex, r2)

    sll            r5, d.rrq_base_addr, RRQ_BASE_ADDR_SHIFT
    add            r5, r5, k.args.rrq_cindex, LOG_RRQ_WQE_SIZE
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqwqe_process, r5)

    nop.e
    nop

unsolicited_ack:
    // if its unsolicted ack, just post credits, msn and exit, CQ posting not needed
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)
    CAPRI_SET_TABLE_0_VALID(0)
    // TODO - need to do this as part of write-back for locking
    tblmincri      d.nxt_to_go_token_id, SIZEOF_TOKEN_ID_BITS, 1
    nop.e
    nop

duplicate_read_resp_mid:
duplicate_ack:
recirc_cnt_exceed:
recirc:
invalid_pkt_msn:
    CAPRI_SET_TABLE_0_VALID(0)
    // TODO - need to do this as part of write-back for locking
    tblmincri      d.nxt_to_go_token_id, SIZEOF_TOKEN_ID_BITS, 1
    phvwr         p.common.p4_intr_global_drop, 1

    nop.e
    nop

process_feedback:
    CAPRI_SET_TABLE_0_VALID(0)

    // TODO - need to do this as part of write-back for locking
    tblmincri      d.nxt_to_go_token_id, SIZEOF_TOKEN_ID_BITS, 1

    CAPRI_GET_TABLE_3_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_CQ_T, tbl_id, 3)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_CQ_T, dma_cmd_index, REQ_RX_DMA_CMD_CQ)

    REQ_RX_CQCB_ADDR_GET(r1, d.cq_id)
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, req_rx_cqcb_process, r1)

    nop.e
    nop
