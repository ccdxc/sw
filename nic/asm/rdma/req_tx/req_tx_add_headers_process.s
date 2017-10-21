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
struct req_tx_add_headers_process_k_t k;
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
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, 5)

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
            // dma_cmd[5] - deth only if it is UD service
            //DMA_PHV2PKT_SETUP_C(r6, deth, deth, c3)
            //DMA_NEXT_CMD_I_BASE_GET_C(r6, 1, c3)
        
            // check_credits = TRUE
            setcf          c5, [c0]

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
            // dma_cmd[5] - IMMETH hdr
            phvwr          IETH_R_KEY, d.inv_key
            DMA_PHV2PKT_SETUP(r6, ieth, ieth)

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

            // dma_cmd[5] - IMMEth hdr
            DMA_PHV2PKT_SETUP(r6, ieth, ieth)
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
            // dma_cmd[5] - IMMEth hdr
            DMA_PHV2PKT_SETUP(r6, immeth, immeth)
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
        
            // dma_cmd[5] - deth for UD service
            //DMA_PHV2PKT_SETUP_C(r6, deth, deth, c3)
            // leave room for deth hdr - dma_cmd[5]  if UD service 
            seq            c3, d.service, RDMA_SERV_TYPE_UD
            DMA_NEXT_CMD_I_BASE_GET_C(r6, 1, c3)
        
            // dma_cmd[5] - IMMEth hdr / For UD  dma_cmd[6]
            DMA_PHV2PKT_SETUP(r6, immeth, immeth)
            phvwr          IMMDT_DATA, k.args.op.send_wr.imm_data

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_ONLY_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        .csend

    .brcase OP_TYPE_READ
        //inc_rrq_pindex = TRUE
        setcf          c7, [c0]
        // inc_lsn = TRUE
        setcf          c4, [c0]
        // adjust_psn = TRUE
        setcf          c6, [c0]
        
        // dma_cmd[5] - add READ Req reth hdr
        DMA_PHV2PKT_SETUP(r6, reth, reth) 
        DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
        
        // rrqwqe_p = rrq_base_addr + rrq_p_index * sizeof(rrqwqe_t)
        add            r3, d.rrq_base_addr, k.args.rrq_p_index, LOG_RRQ_WQE_SIZE
        
        phvwr          RRQWQE_PSN, d.tx_psn           
        add            r1, d.msn, 1
        phvwr          RRQWQE_MSN, r1 
        
        // dma_cmd[6]
        DMA_NEXT_CMD_I_BASE_GET(r6, 1)
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
            // check_credits = TRUE
            setcf          c5, [c0]
            // inc_lsn = TRUE
            setcf          c4, [c0]

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_LAST, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_FIRST
            // dma_cmd[5] = add reth hdr
            DMA_PHV2PKT_SETUP(r6, reth, reth)

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        
        .brcase RDMA_PKT_ONLY
            // check_credits = TRUE
            setcf          c5, [c0]
            // inc_lsn = TRUE
            setcf          c4, [c0]

            // dma_cmd[5] - add reth hdr
            DMA_PHV2PKT_SETUP(r6, reth, reth) 

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

            // dma_cmd[5] - add IMMETH hdr
            DMA_PHV2PKT_SETUP(r6, immeth, immeth)
            phvwr          IMMDT_DATA, d.imm_data

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_LAST_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_FIRST
            DMA_PHV2PKT_SETUP(r6, reth, reth)
            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_ONLY
            // check_credits = TRUE
            setcf          c5, [c0]

            // dma_cmd[5] = add reth hdr
            DMA_PHV2PKT_SETUP(r6, reth, reth)

            // dma_cmd[6] - IMMEth hdr
            DMA_NEXT_CMD_I_BASE_GET(r6, 1)
            DMA_PHV2PKT_SETUP(r6, immeth, immeth)
            phvwr          IMMDT_DATA, k.args.op.send_wr.imm_data

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_ONLY_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        .csend

    .brcase OP_TYPE_CMP_N_SWAP
        // inc_lsn = TRUE
        setcf          c4, [c0]
        
        //inc_rrq_pindex = TRUE
        setcf          c7, [c0]
        
        // add atomiceth hdr
        DMA_PHV2PKT_SETUP(r6, atomiceth, atomiceth)
        DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
        
        // rrqwqe_p = rrq_base_addr + rrq_p_index & sizeof(rrqwqe_t)
        add            r3, d.rrq_base_addr, k.args.rrq_p_index, LOG_RRQ_WQE_SIZE
        
        phvwr          RRQWQE_PSN, d.tx_psn

        // dma_cmd[6]
        DMA_NEXT_CMD_I_BASE_GET(r6, 1)
        DMA_HBM_PHV2MEM_SETUP(r6, rrqwqe, rrqwqe, r3)
        
        b              op_type_end
        add            r2, RDMA_PKT_OPC_CMP_SWAP, d.service, RDMA_OPC_SERV_TYPE_SHIFT
   
    .brcase OP_TYPE_FETCH_N_ADD
        // inc_lsn = TRUE
        setcf          c4, [c0]

        //inc_rrq_pindex = TRUE
        setcf          c7, [c0]

        // add atomiceth hdr
        DMA_PHV2PKT_SETUP(r6, atomiceth, atomiceth)
        DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
        
        // rrqwqe_p = rrq_base_addr + rrq_p_index & sizeof(rrqwqe_t)
        add            r3, d.rrq_base_addr, k.args.rrq_p_index, LOG_RRQ_WQE_SIZE
        
        phvwr          RRQWQE_PSN, d.tx_psn
        
        // dma_cmd[6]
        DMA_NEXT_CMD_I_BASE_GET(r6, 1)
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
    add            r3, k.args.log_pmtu, r0
    srlv           r3, k.args.op.rd.read_len, r3
    tblmincr       d.tx_psn, 24, r3

    // tx_psn += (read_len & ((1 << log_pmtu) -1)) ? 1 : 0
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
    scwlt24.c5     c5, d.lsn, d.ssn
    phvwr.c5       BTH_ACK_REQ, 1
    
    SQCB0_ADDR_GET(r2)

    // do we need to increment rrq_pindex ?
    bcf             [!c7], cb1_byte_update
    // cb1_busy is by default set to FALSE
    add            r5, r0, r0   //BD Slot

    // incr_rrq_p_index
    add             r3, r2, RRQ_P_INDEX_OFFSET
    add             r7, r0, k.args.rrq_p_index
    mincr           r7, d.log_rrq_size, 1
    phvwr           p.rrq_p_index, r7.hx

    // dma_cmd[7] - incr rrq_p_index for read/atomic
    DMA_NEXT_CMD_I_BASE_GET(r6, 1)
    DMA_HBM_PHV2MEM_SETUP(r6, rrq_p_index, rrq_p_index, r3)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

cb1_byte_update:
    // on top of it, set need_credits flag is conditionally
    add.c5         r5, r5, SQCB0_NEED_CREDITS_FLAG
    add            r3, r2, FIELD_OFFSET(sqcb0_t, cb1_byte)
    memwr.b        r3, r5

    SQCB1_ADDR_GET(r1)
    CAPRI_GET_TABLE_2_K(req_tx_phv_t, r7)
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
