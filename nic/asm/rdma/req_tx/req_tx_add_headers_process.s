#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"

    #c1 - last
    #c2 - first
    #c3 - UD service, Needed only for send & send_imm
    #c4 - incr_lsn
    #c5 - check credits
    #c6 - adjust_psn/incr_psn
    #c7 - incr_rrq_pindex

struct req_tx_phv_t p;
struct req_tx_write_back_process_k_t k;
struct sqcb1_t d;

#define RDMA_PKT_MIDDLE      0
#define RDMA_PKT_LAST        1
#define RDMA_PKT_FIRST       2
#define RDMA_PKT_ONLY        3
%%
    .param    req_tx_add_headers_2_process

.align
req_tx_add_headers_process:
    // initialize  cf to 0
    crestore        [c7-c1], r0, 0xfe

    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_HEADERS)
    // To start with, num_addr is 1 (bth)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, bth, bth, 1)
    seq            c2, k.args.first, 1
    add            r2, k.args.op_type, r0
    .brbegin
    br             r2[2:0]    
    seq            c1, k.args.last, 1 // Branch Delay Slot

    .brcase OP_TYPE_SEND
        .csbegin
        cswitch [c2, c1]
        nop
        .brcase RDMA_PKT_MIDDLE
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot

        .brcase RDMA_PKT_LAST
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_LAST, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot

        .brcase RDMA_PKT_FIRST
            // check_credits = TRUE
            setcf          c5, [c0]

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot

        .brcase RDMA_PKT_ONLY
            // check_credits = TRUE
            setcf          c5, [c0]

            // Update num addrs to 2 if UD service (bth, deth)
            seq            c3, d.service, RDMA_SERV_TYPE_UD
            DMA_PHV2PKT_SETUP_CMDSIZE_C(r6, 2, c3)
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_ONLY, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        .csend

    .brcase OP_TYPE_SEND_INV
        .csbegin
        cswitch [c2, c1]
        tblwr.c2       d.inv_key, k.args.op.send_wr.inv_key // Branch Delay Slot
        .brcase RDMA_PKT_MIDDLE
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        .brcase RDMA_PKT_LAST
            // dma_cmd[2] - IETH hdr
            phvwr          IETH_R_KEY, d.inv_key
            // num addrs 2 (bth, ieth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, ieth, ieth, 1)

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_LAST_WITH_INV, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        .brcase RDMA_PKT_FIRST
            // check_credits = TRUE
            setcf          c5, [c0]

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        .brcase RDMA_PKT_ONLY
            // check_credits = TRUE
            setcf          c5, [c0]

            // dma_cmd[2] - IETH hdr; num addrs 2 (bth, ieth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, ieth, ieth, 1)
            phvwr          IETH_R_KEY, k.args.op.send_wr.inv_key

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_ONLY_WITH_INV, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        .csend

    .brcase OP_TYPE_SEND_IMM
        .csbegin
        cswitch [c2, c1]
        tblwr.c2       d.imm_data, k.args.op.send_wr.imm_data  // Branch Delay Slot
        
        .brcase RDMA_PKT_MIDDLE
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_LAST
            // dma_cmd[2] - IMMETH hdr, num addrs 2 (bth, immeth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, immeth, immeth, 1)
            phvwr          IMMDT_DATA, d.imm_data
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_LAST_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        
        .brcase RDMA_PKT_FIRST
            // check_credits = TRUE
            setcf          c5, [c0]

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        
        .brcase RDMA_PKT_ONLY
            // check_credits = TRUE
            setcf          c5, [c0]
        
            // dma_cmd[2] - IMMETH hdr, num addrs 2 (bth, immeth) or 3 for UD (bth, deth, immeth)
            seq            c3, d.service, RDMA_SERV_TYPE_UD
            DMA_PHV2PKT_SETUP_CMDSIZE_C(r6, 2, !c3)
            DMA_PHV2PKT_SETUP_CMDSIZE_C(r6, 3, c3)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N_C(r6, immeth, immeth, 1, !c3)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N_C(r6, immeth, immeth, 2, c3)

            phvwr          IMMDT_DATA, k.args.op.send_wr.imm_data

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_ONLY_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        .csend

    .brcase OP_TYPE_READ
        // inc_rrq_pindex = TRUE; adjust_psn = TRUE; inc_lsn = TRUE
        crestore       [c7, c6, c4], 0xd0, 0xd0
        
        // dma_cmd[2] - RETH hdr, num addrs 2 (bth, reth)
        DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, reth, reth, 1)
        DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
        
        // rrqwqe_p = rrq_base_addr + rrq_p_index * sizeof(rrqwqe_t)
        add            r3, d.rrq_base_addr, k.args.rrq_p_index, LOG_RRQ_WQE_SIZE
        
        phvwr          RRQWQE_PSN, d.tx_psn           
        phvwr          RRQWQE_MSN, d.ssn
        
        // dma_cmd[3]
        DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RRQWQE)
        DMA_HBM_PHV2MEM_SETUP(r6, rrqwqe, rrqwqe, r3)
        
        b              op_type_end
        add            r2, RDMA_PKT_OPC_RDMA_READ_REQ, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot

    .brcase OP_TYPE_WRITE
        .csbegin
        cswitch [c2, c1]
        nop
        .brcase RDMA_PKT_MIDDLE
            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_LAST
            // check_credits = TRUE; inc_lsn = TRUE
            crestore       [c5, c4], 0x30, 0x30

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_LAST, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_FIRST
            // dma_cmd[2] - RETH hdr, num addrs 2 (bth, reth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, reth, reth, 1)

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        
        .brcase RDMA_PKT_ONLY
            // check_credits = TRUE; inc_lsn = TRUE
            crestore       [c5, c4], 0x30, 0x30

            // dma_cmd[2] - RETH hdr, num addrs 2 (bth, reth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, reth, reth, 1)

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_ONLY, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        .csend

  
    .brcase OP_TYPE_WRITE_IMM
        .csbegin
        cswitch [c2, c1]
        tblwr.c2       d.imm_data, k.args.op.send_wr.imm_data  // Branch Delay Slot
        .brcase RDMA_PKT_MIDDLE
            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_LAST
            // check_credits = TRUE
            setcf          c5, [c0]

            // dma_cmd[2] - IMMETH hdr, num addrs 2 (bth, immeth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, immeth, immeth, 1)
            phvwr          IMMDT_DATA, d.imm_data

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_LAST_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_FIRST
            // dma_cmd[2] - RETH hdr, num addrs 2 (bth, reth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, reth, reth, 1)
            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_ONLY
            // check_credits = TRUE
            setcf          c5, [c0]

            // dma_cmd[2] : addr1 - RETH hdr, num addrs 3 (bth, reth, immeth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 3)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, reth, reth, 1)

            // dma_cmd[2] : addr2 - IMMETH hdr
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, immeth, immeth, 2)
            phvwr          IMMDT_DATA, k.args.op.send_wr.imm_data

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_ONLY_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        .csend

    .brcase OP_TYPE_CMP_N_SWAP
        // inc_rrq_pindex = TRUE; inc_lsn = TRUE;
        crestore       [c7, c4], 0x90, 0x90
        
        // dma_cmd[2] - ATOMICETH hdr, num addrs 2 (bth, atomiceth)
        DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, atomiceth, atomiceth, 1)
        DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
        
        // rrqwqe_p = rrq_base_addr + rrq_p_index & sizeof(rrqwqe_t)
        add            r3, d.rrq_base_addr, k.args.rrq_p_index, LOG_RRQ_WQE_SIZE
        
        phvwr          RRQWQE_PSN, d.tx_psn
        phvwr          RRQWQE_ATOMIC_OP_TYPE, k.args.op_type
        phvwr          RRQWQE_MSN, d.ssn
        
        // dma_cmd[3] 
        DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RRQWQE)
        DMA_HBM_PHV2MEM_SETUP(r6, rrqwqe, rrqwqe, r3)
        
        b              op_type_end
        add            r2, RDMA_PKT_OPC_CMP_SWAP, d.service, RDMA_OPC_SERV_TYPE_SHIFT
   
    .brcase OP_TYPE_FETCH_N_ADD
        // inc_rrq_pindex = TRUE; inc_lsn = TRUE;
        crestore       [c7, c4], 0x90, 0x90

        // dma_cmd[2] - ATOMICETH hdr, num addrs 2 (bth, atomiceth)
        DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, atomiceth, atomiceth, 1)
        DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
        
        // rrqwqe_p = rrq_base_addr + rrq_p_index & sizeof(rrqwqe_t)
        add            r3, d.rrq_base_addr, k.args.rrq_p_index, LOG_RRQ_WQE_SIZE
        
        phvwr          RRQWQE_PSN, d.tx_psn
        phvwr          RRQWQE_ATOMIC_OP_TYPE, k.args.op_type
        phvwr          RRQWQE_MSN, d.ssn
        
        // dma_cmd[3] - rrqwqe
        DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RRQWQE)
        DMA_HBM_PHV2MEM_SETUP(r6, rrqwqe, rrqwqe, r3)
        
        b              op_type_end
        add            r2, RDMA_PKT_OPC_FETCH_ADD, d.service, RDMA_OPC_SERV_TYPE_SHIFT
    .brend

op_type_end:
    phvwr          BTH_OPCODE, r2

    b.!c6          inc_psn
    // phv_p->bth.psn = sqcb1_p->tx_psn
    phvwr          BTH_PSN, d.tx_psn  // Branch Delay Slot

    // if (adjust_psn)
    // tx_psn = read_len >> log_pmtu
    add            r3, k.args.op.rd.log_pmtu, r0
    srlv           r3, k.args.op.rd.read_len, r3
    tblmincr       d.tx_psn, 24, r3

    // tx_psn += (read_len & ((1 << log_pmtu) -1)) ? 1 : 0
    add            r3, k.args.op.rd.read_len, r0
    mincr          r3, k.args.op.rd.log_pmtu, r0
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
    scwlt24.c5     c5, d.lsn, d.ssn
    bcf            [!c5], rrq_p_index_chk
    SQCB0_ADDR_GET(r2)     // Branch Delay Slot

    phvwr          BTH_ACK_REQ, 1
    // Disable TX scheduler for this QP until ack is received with credits to
    // send subsequent packets
    DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(k.global.lif, k.global.qtype, k.global.qid, SQ_RING_ID, r3, r4)

rrq_p_index_chk:
    // do we need to increment rrq_pindex ?
    bcf             [!c7], cb1_byte_update
    // cb1_busy is by default set to FALSE
    add            r5, r0, r0   //BD Slot

    // incr_rrq_p_index
    add             r3, r2, RRQ_P_INDEX_OFFSET
    add             r7, r0, k.args.rrq_p_index
    mincr           r7, d.log_rrq_size, 1
    phvwr           p.rrq_p_index, r7.hx

    // dma_cmd[4] - incr rrq_p_index for read/atomic
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RRQ_PINDEX)
    DMA_HBM_PHV2MEM_SETUP(r6, rrq_p_index, rrq_p_index, r3)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

cb1_byte_update:
    // on top of it, set need_credits flag is conditionally
    add.c5         r5, r5, SQCB0_NEED_CREDITS_FLAG
    add            r3, r2, FIELD_OFFSET(sqcb0_t, cb1_byte)
    memwr.b        r3, r5

    SQCB1_ADDR_GET(r1)
    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_add_headers_2_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

invalid_op_type:
    nop.e
    nop

ud_error:
    //For UD we can silently drop
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop
