#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_add_headers_process_k_t k;
struct sqcb1_t d;

#define INFO_OUT_T struct req_tx_sqcb_write_back_info_t
%%
    .param    req_tx_write_back_process

.align
req_tx_add_headers_process:
    // check_credits = FALSE
    setcf          c5, [!c0]

    // adjust_psn = FALSE
    setcf          c6, [!c0]

    // get DMA cmd entry based on dma_cmd_index - dma_cmd[0]
    DMA_CMD_I_BASE_GET(r7, r2, REQ_TX_DMA_CMD_START_FLIT_ID, r0)

    DMA_PHV2PKT_SETUP(r7, common.p4_intr_global_tm_iport, common.p4_intr_global_glb_rsv)
    phvwri          p.common.p4_intr_global_tm_iport, 9
    phvwri          p.common.p4_intr_global_tm_oport, 11
    phvwri          p.common.p4_intr_global_tm_oq, 0

    // dma_cmd[1]
    sub            r7, r7, 1, LOG_DMA_CMD_SIZE_BITS
    DMA_PHV2PKT_SETUP(r7, p4plus_to_p4, p4plus_to_p4);
    phvwr          P4PLUS_TO_P4_APP_ID, 2
    phvwr          P4PLUS_TO_P4_FLAGS, d.p4plus_to_p4_flags
    phvwr          P4PLUS_TO_P4_IP_LEN, 0
    phvwr          P4PLUS_TO_P4_UDP_LEN, 0
    phvwr          P4PLUS_TO_P4_VLAN_ID, 2

    // dma_cmd[2]
    sub            r7, r7, 1, LOG_DMA_CMD_SIZE_BITS
    // PHV_Q_DMA_CMD(phv_p, dma_cmd_index, DMA_CMD_TYPE_MEM_TO_PKT, hbm_addr_get(sqcb1_p->header_template_addr),
    //                 sizeof(header_template_t))
    DMA_HBM_MEM2PKT_SETUP(r7, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr)

    // dma_cmd[3]
    sub            r7, r7, 1, LOG_DMA_CMD_SIZE_BITS
    DMA_PHV2PKT_SETUP(r7, bth, bth)

    // phv_p->bth.dst_qp = sqcb1_p->dst_qp
    phvwr          BTH_DST_QP, d.dst_qp

    // r1 = k.args.op_type
    add            r2, k.args.op_type, r0
    beqi           r2, OP_TYPE_SEND, op_type_send
    nop
    beqi           r2, OP_TYPE_SEND_INV, op_type_send_inv
    nop
    beqi           r2, OP_TYPE_SEND_IMM, op_type_send_imm
    nop
    b              end
    nop

op_type_send:
    seq            c1, k.args.first, 1
    seq            c2, k.args.last, 1
    setcf          c3, [c1 & c2]
    setcf          c4, [c1 & !c2]
    add.c3         r2, RDMA_PKT_OPC_SEND_ONLY, d.service, RDMA_OPC_SERV_TYPE_SHIFT
    add.c4         r2, RDMA_PKT_OPC_SEND_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT

    setcf          c3, [!c1 & c2]
    setcf          c4, [!c1 & !c2]
    add.c3         r2, RDMA_PKT_OPC_SEND_LAST, d.service, RDMA_OPC_SERV_TYPE_SHIFT
    add.c4         r2, RDMA_PKT_OPC_SEND_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT
    phvwr          BTH_OPCODE, r2
    b              end
    // check_credits = TRUE
    setcf          c5, [c0] // Branch Delay Slot

op_type_send_inv:
    b              end
    nop

op_type_send_imm:
    b              end
    nop

end:
    b.!c6          inc_psn
    // phv_p->bth.psn = sqcb1_p->tx_psn
    phvwr          BTH_PSN, d.tx_psn  // Branch Delay Slot

    // if (adjust_psn)
    // sqcb1_p->tx_psn += rrqwqe_to_hdr_info_p->op.rd.read_len >> rrqwqe_to_hdr_info_p->log_pmtu
    add.c6         r3, k.args.log_pmtu, r0
    srlv.c6        r3, k.args.op.rd.read_len, r3
    tblmincr.c6    d.tx_psn, 24, r3

    // sqcb1_p->tx_psn += (rrqwqe_to_hdr_info_p->op.rd.read_len & ((1 << rrqwqe_to_hdr_info_p->log_pmtu) -1)) ? 1 : 0
    add.c6         r3, k.args.op.rd.read_len, r0
    mincr.c6       r3, k.args.log_pmtu, r0
    sle.c6         c6, r3, r0

inc_psn:
    // sqcb1_p->tx_psn++
    tblmincri.!c6  d.tx_psn, 24, 1

    // if (rrqwqe_to_hdr_info_p->last)
    //     sqcb1_p->ssn++;
    seq            c1, k.args.last, 1
    tblmincri.c1   d.ssn, 24, 1

    // get tbl_id from s2s data
    add            r1, k.args.tbl_id, r0
    CAPRI_GET_TABLE_I_ARG(req_tx_phv_t, r1, r7)

    // if (check_credits && (sqcb1_p->ssn > sqcb1_p->lsn))
    //     phv_p->bth.a = 1
    //     write_back_info_p->set_credits = TRUE
    slt.c5         c5, d.lsn, d.ssn
    phvwr.c5       BTH_ACK_REQ, 1
    CAPRI_SET_FIELD_C(r7, INFO_OUT_T, set_credits, 1, c5)

    CAPRI_SET_FIELD(r7, INFO_OUT_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, busy, k.args.busy)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, current_sge_id, k.args.op.send_wr.current_sge_id)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, current_sge_offset, k.args.op.send_wr.current_sge_offset)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, num_sges, k.args.op.send_wr.num_sges)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, wqe_addr, k.args.wqe_addr)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, last, k.args.last)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, first, k.args.first)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, op_type, k.args.op_type)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, incr_rrq_pindex, 0) // TODO set this only for READ or atomic
    CAPRI_SET_FIELD(r7, INFO_OUT_T, tbl_id, k.args.tbl_id)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, set_li_fence, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, set_fence, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, set_bktrack, 0)

    SQCB0_ADDR_GET(r2)
    CAPRI_GET_TABLE_I_K(req_tx_phv_t, r1, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_write_back_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r2)

    nop.e
    nop
