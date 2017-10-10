#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"

struct req_tx_phv_t p;
struct req_tx_add_headers_process_k_t k;
struct sqcb1_t d;

%%

.align
req_tx_add_headers_process:
    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_I_BASE_GET(r6, r2, REQ_TX_DMA_CMD_START_FLIT_ID, r0)

    // dma_cmd[0] - p4_intr
    DMA_PHV2PKT_SETUP(r6, common.p4_intr_global_tm_iport, common.p4_intr_global_glb_rsv)
    phvwri          p.common.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri          p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS
    phvwri          p.common.p4_intr_global_tm_oq, 0

    // dma_cmd[1] - p4_txdma_intr
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)
    DMA_PHV2PKT_SETUP(r6, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv)
    phvwr           p.common.p4_txdma_intr_qid, k.global.qid
    SQCB0_ADDR_GET(r1)
    phvwr           p.common.p4_txdma_intr_qstate_addr, r1
    phvwr           p.common.p4_txdma_intr_qtype, k.global.qtype

    // dma_cmd[2] - p4plus_to_p4_header
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)
    DMA_PHV2PKT_SETUP(r6, p4plus_to_p4, p4plus_to_p4);
    phvwr          P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA
    phvwr          P4PLUS_TO_P4_FLAGS, d.p4plus_to_p4_flags
    phvwr          P4PLUS_TO_P4_VLAN_TAG, 0

    // dma_cmd[3] - header_template
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)
    // PHV_Q_DMA_CMD(phv_p, dma_cmd_index, DMA_CMD_TYPE_MEM_TO_PKT, hbm_addr_get(sqcb1_p->header_template_addr),
    //                 sizeof(header_template_t))
    DMA_HBM_MEM2PKT_SETUP(r6, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr)

    // dma_cmd[4] - BTH
    addi           r6, r6, DMA_SWITCH_TO_NEXT_FLIT_BITS
    DMA_PHV2PKT_SETUP(r6, bth, bth)

    // setup for dma_cmd[5]
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)

    // phv_p->bth.dst_qp = sqcb1_p->dst_qp
    phvwr          BTH_DST_QP, d.dst_qp

    // get tbl_id from s2s data
    add            r1, k.args.tbl_id, r0
    CAPRI_SET_TABLE_I_VALID(r1, 0)
    //CAPRI_GET_TABLE_I_ARG(req_tx_phv_t, r1, r7)

    #c1 - last
    #c2 - first
    #c3 - immediate
    #c4 - incr_lsn
    #c5 - check credits
    #c6 - adjust_psn/incr_psn
    #c7 - incr_rrq_pindex

    //CAPRI_SET_FIELD(r7, INFO_OUT_T, incr_rrq_pindex, 0) // set for READ or atomic
    setcf          c7, [!c0]

    seq            c2, k.args.first, 1
    seq            c1, k.args.last, 1

    // r2 = k.args.op_type
    add            r2, k.args.op_type, r0
    beqi           r2, OP_TYPE_SEND, op_type_send
    nop

    beqi           r2, OP_TYPE_SEND_INV, op_type_send_inv
    nop

    beqi           r2, OP_TYPE_SEND_IMM, op_type_send_imm
    nop

    beqi           r2, OP_TYPE_WRITE, op_type_write
    nop

    beqi           r2, OP_TYPE_WRITE_IMM, op_type_write_imm
    nop

    beqi           r2, OP_TYPE_READ, op_type_read
    nop

    beqi           r2, OP_TYPE_CMP_N_SWAP, op_type_atomic_cmp_swap
    nop

    beqi           r2, OP_TYPE_FETCH_N_ADD, op_type_atomic_fetch_add
    nop

    b              invalid_op_type
    nop

op_type_atomic_cmp_swap:
    // opc = CMP_N_SWAP or FETCH_N_ADD
    b              op_type_atomic
    add            r2, RDMA_PKT_OPC_CMP_SWAP, d.service, RDMA_OPC_SERV_TYPE_SHIFT
    
op_type_atomic_fetch_add:
    add            r2, RDMA_PKT_OPC_FETCH_ADD, d.service, RDMA_OPC_SERV_TYPE_SHIFT

op_type_atomic:
    // add atomiceth hdr
    DMA_PHV2PKT_SETUP(r6, atomiceth, atomiceth)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)

    // rrqwqe_p = (rrqwqe_t *)(sqcb1_p->rrq_base_addr) + (rrqwqe_to_hdr_info_p->rrq_p_index * sizeof(rrqwqe_t))
    add            r3, d.rrq_base_addr, k.args.rrq_p_index, LOG_RRQ_WQE_SIZE

    phvwr          RRQWQE_READ_RSP_OR_ATOMIC, RRQ_OP_TYPE_ATOMIC
    phvwr          RRQWQE_NUM_SGES, 1
    phvwr          RRQWQE_PSN, d.tx_psn
    phvwr          RRQWQE_ATOMIC_SGE_VA, k.args.op.atomic.sge.va
    phvwr          RRQWQE_ATOMIC_SGE_LEN, k.args.op.atomic.sge.len
    phvwr          RRQWQE_ATOMIC_SGE_LKEY, k.args.op.atomic.sge.l_key

    // dma_cmd[6]
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)
    DMA_HBM_PHV2MEM_SETUP(r6, rrqwqe, rrqwqe, r3)

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

    //CAPRI_SET_FIELD(r7, INFO_OUT_T, incr_rrq_pindex, 1) // set for READ or atomic
    setcf          c7, [c0]

    b              set_bth_opc
    // inc_lsn = TRUE
    setcf          c4, [c0] // Branch Delay Slot

op_type_read:
    //opc = read_req
    add            r2, RDMA_PKT_OPC_RDMA_READ_REQ, d.service, RDMA_OPC_SERV_TYPE_SHIFT

    // add READ Req reth hdr
    DMA_PHV2PKT_SETUP(r6, reth, reth) 
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)

    // rrqwqe_p = (rrqwqe_t *)(sqcb1_p->rrq_base_addr) + (rrqwqe_to_hdr_info_p->rrq_p_index * sizeof(rrqwqe_t))
    add            r3, d.rrq_base_addr, k.args.rrq_p_index, LOG_RRQ_WQE_SIZE

    phvwr          RRQWQE_READ_RSP_OR_ATOMIC, RRQ_OP_TYPE_READ
    phvwr          RRQWQE_NUM_SGES, k.args.op.rd.num_sges
    phvwr          RRQWQE_PSN, d.tx_psn           
    add            r1, d.msn, 1
    phvwr          RRQWQE_MSN, r1 
    phvwr          RRQWQE_READ_LEN, k.args.op.rd.read_len
    add            r1, k.to_stage.wqe_addr, TXWQE_SGE_OFFSET
    phvwr          RRQWQE_READ_WQE_SGE_LIST_ADDR, r1

    // dma_cmd[6]
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)
    DMA_HBM_PHV2MEM_SETUP(r6, rrqwqe, rrqwqe, r3) 

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

    //CAPRI_SET_FIELD(r7, INFO_OUT_T, incr_rrq_pindex, 1) // set for READ or atomic
    setcf          c7, [c0]
    
    setcf          c4, [c0]
    b              set_bth_opc
    // adjust_psn = TRUE
    setcf          c6, [c0] // Branch Delay Slot

op_type_send_inv:
    tblwr.c2       d.inv_key, k.args.op.send_wr.inv_key

    //figure out the opcode
    .csbegin

    cswitch [c2, c1]
    nop

    .brcase 0 //not-first, not-last
        add            r2, RDMA_PKT_OPC_SEND_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        b              set_bth_opc
        nop

    .brcase 1 //not-first, last
        // add IMMETH hdr
        // dma_cmd[5]
        DMA_PHV2PKT_SETUP(r6, ieth, ieth)
        add            r2, RDMA_PKT_OPC_SEND_LAST_WITH_INV, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        b              set_bth_opc
        phvwr          IETH_R_KEY, d.inv_key //branch delay slot

    .brcase 2 //first, not-last
        add            r2, RDMA_PKT_OPC_SEND_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        b              set_bth_opc
        nop

    .brcase 3 //first, last (only)
        // add IMMETH hdr
        // dma_cmd[5]
        DMA_PHV2PKT_SETUP(r6, ieth, ieth)
        add            r2, RDMA_PKT_OPC_SEND_ONLY_WITH_INV, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        b              set_bth_opc
        phvwr          IETH_R_KEY, k.args.op.send_wr.inv_key //branch delay slot
    .csend

op_type_send_imm:
    setcf          c3, [c1]
    tblwr.c2       d.imm_data, k.args.op.send_wr.imm_data 

op_type_send:

    //figure out the opcode

    .csbegin

    cswitch [c2, c1]
    nop

    .brcase 0 //not-first, not-last
        add            r2, RDMA_PKT_OPC_SEND_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        b              set_bth_opc
        nop

    .brcase 1 //not-first, last
        bcf            [!c3], set_bth_opc
        add            r2, RDMA_PKT_OPC_SEND_LAST, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        // add IMMETH hdr
        // dma_cmd[4]
        DMA_PHV2PKT_SETUP(r6, immeth, immeth)
        add            r2, RDMA_PKT_OPC_SEND_LAST_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        b              set_bth_opc
        phvwr          IMMDT_DATA, d.imm_data //branch delay slot

    .brcase 2 //first, not-last
        add            r2, RDMA_PKT_OPC_SEND_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        b              set_bth_opc
        nop

    .brcase 3 //first, last (only)
        bcf            [!c3], set_bth_opc
        add            r2, RDMA_PKT_OPC_SEND_ONLY, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        // add IMMETH hdr
        // dma_cmd[4]
        DMA_PHV2PKT_SETUP(r6, immeth, immeth)
        add            r2, RDMA_PKT_OPC_SEND_ONLY_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        b              set_bth_opc
        phvwr          IMMDT_DATA, k.args.op.send_wr.imm_data //branch delay slot
    .csend

op_type_write_imm:
    setcf          c3, [c1]
    tblwr.c2       d.imm_data, k.args.op.send_wr.imm_data 

op_type_write:
    setcf          c4, [c1 & !c3]
    // if first, add RETH hdr
    // dma_cmd[5]
    //jump to next flit
    DMA_PHV2PKT_SETUP_C(r6, reth, reth, c2)

    //figure out the opcode
    .csbegin

    cswitch [c2, c1]
    nop

    .brcase 0 //not-first, not-last
        add            r2, RDMA_PKT_OPC_RDMA_WRITE_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        b              set_bth_opc
        nop

    .brcase 1 //not-first, last
        bcf            [!c3], set_bth_opc
        add            r2, RDMA_PKT_OPC_RDMA_WRITE_LAST, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        // dma_cmd[5] - add IMMETH hdr
        //jump to next flit
        DMA_PHV2PKT_SETUP(r6, immeth, immeth)
        add            r2, RDMA_PKT_OPC_RDMA_WRITE_LAST_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        b              set_bth_opc
        phvwr          IMMDT_DATA, d.imm_data //branch delay slot

    .brcase 2 //first, not-last
        add            r2, RDMA_PKT_OPC_RDMA_WRITE_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        b              set_bth_opc
        nop

    .brcase 3 //first, last (only)

        bcf            [!c3], set_bth_opc
        add            r2, RDMA_PKT_OPC_RDMA_WRITE_ONLY, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        // add IMMETH hdr
        // dma_cmd[6]
        DMA_NEXT_CMD_I_BASE_GET(r6, 1)
        DMA_PHV2PKT_SETUP(r6, immeth, immeth)
        add            r2, RDMA_PKT_OPC_RDMA_WRITE_ONLY_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        b              set_bth_opc
        phvwr          IMMDT_DATA, k.args.op.send_wr.imm_data //branch delay slot
 
    .csend

set_bth_opc:
    phvwr          BTH_OPCODE, r2
    b              end
    // check_credits = TRUE
    setcf          c5, [c0] // Branch Delay Slot

end:
    b.!c6          inc_psn
    // phv_p->bth.psn = sqcb1_p->tx_psn
    phvwr          BTH_PSN, d.tx_psn  // Branch Delay Slot

    // if (adjust_psn)
    // sqcb1_p->tx_psn += rrqwqe_to_hdr_info_p->op.rd.read_len >> rrqwqe_to_hdr_info_p->log_pmtu
    add            r3, k.args.log_pmtu, r0
    srlv           r3, k.args.op.rd.read_len, r3
    tblmincr       d.tx_psn, 24, r3

    // sqcb1_p->tx_psn += (rrqwqe_to_hdr_info_p->op.rd.read_len & ((1 << rrqwqe_to_hdr_info_p->log_pmtu) -1)) ? 1 : 0
    add            r3, k.args.op.rd.read_len, r0
    mincr          r3, k.args.log_pmtu, r0
    sle            c6, r3, r0

inc_psn:
    // sqcb1_p->tx_psn++
    tblmincri.!c6  d.tx_psn, 24, 1

    // if (rrqwqe_to_hdr_info_p->last)
    //     sqcb1_p->ssn++;
    tblmincri.c1   d.ssn, 24, 1

    // inc lsn for read, atomic, write (without imm)
    tblmincri.c4   d.lsn, 24, 1

    // if (check_credits && (sqcb1_p->ssn > sqcb1_p->lsn))
    //     phv_p->bth.a = 1
    //     write_back_info_p->set_credits = TRUE
    slt.c5         c5, d.lsn, d.ssn
    phvwr.c5       BTH_ACK_REQ, 1
    
    SQCB0_ADDR_GET(r2)
    // incr_rrq_p_index if reqd
    add.c7          r3, r2, RRQ_P_INDEX_OFFSET
    add.c7          r7, r0, k.args.rrq_p_index
    mincr.c7        r7, d.log_rrq_size, 1
    memwr.hx.c7      r3, r7

    // cb1_busy is by default set to FALSE
    add            r7, r0, r0

    // on top of it, set need_credits flag is conditionally
    add.c5         r7, r7, SQCB0_NEED_CREDITS_FLAG

    add            r3, r2, FIELD_OFFSET(sqcb0_t, cb1_byte)
    memwr.b        r3, r7

invalid_op_type:
    nop.e
    nop
