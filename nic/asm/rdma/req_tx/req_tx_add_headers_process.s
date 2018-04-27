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
struct req_tx_s5_t3_k k;
struct sqcb2_t d;

#define IN_P t3_s2s_sqcb_write_back_info_rd
#define IN_RD_P t3_s2s_sqcb_write_back_info_rd
#define IN_SEND_WR_P t3_s2s_sqcb_write_back_info_send_wr
#define IN_TO_S_P to_s5_sq_to_stage

#define K_SPEC_CINDEX CAPRI_KEY_RANGE(IN_TO_S_P, spec_cindex_sbit0_ebit7, spec_cindex_sbit8_ebit15)
#define K_OP_TYPE     CAPRI_KEY_FIELD(IN_P, op_type)
#define K_INV_KEY     CAPRI_KEY_RANGE(IN_SEND_WR_P, op_send_wr_inv_key_or_ah_handle_sbit0_ebit7, op_send_wr_inv_key_or_ah_handle_sbit8_ebit31)
#define K_AH_HANDLE   K_INV_KEY
#define K_AH_SIZE     CAPRI_KEY_RANGE(IN_P, ah_size_sbit0_ebit2, ah_size_sbit3_ebit7)
#define K_IMM_DATA    CAPRI_KEY_FIELD(IN_SEND_WR_P, op_send_wr_imm_data)
#define K_RD_READ_LEN CAPRI_KEY_FIELD(IN_RD_P, op_rd_read_len)
#define K_RD_LOG_PMTU CAPRI_KEY_FIELD(IN_RD_P, op_rd_log_pmtu)

#define ADD_HDR_T t3_s2s_add_hdr_info

#define RDMA_PKT_MIDDLE      0
#define RDMA_PKT_LAST        1
#define RDMA_PKT_FIRST       2
#define RDMA_PKT_ONLY        3
%%
    .param    req_tx_add_headers_2_process

.align
req_tx_add_headers_process:
    // if speculative cindex matches cindex, then this wqe is being
    // processed in the right order and state update is allowed. Otherwise
    // discard and continue with speculation until speculative cindex
    // matches current cindex. Similarly, drop if dcqcn rate enforcement
    // doesn't allow this packet
    seq            c1, K_SPEC_CINDEX, d.sq_cindex
    bcf            [!c1], spec_fail
    // initialize  cf to 0
    crestore       [c7-c1], r0, 0xfe // Branch Delay Slot

    bbeq           CAPRI_KEY_FIELD(IN_P, poll_failed), 1, poll_fail
    seq            c1, CAPRI_KEY_FIELD(IN_P, last_pkt), 1 // Branch Delay Slot

    bbeq           CAPRI_KEY_FIELD(IN_P, rate_enforce_failed), 1, rate_enforce_fail
    seq            c2, CAPRI_KEY_FIELD(IN_P, first), 1 // Branch Delay Slot

    bbeq          CAPRI_KEY_FIELD(IN_TO_S_P, fence), 1, fence
    nop // Branch Delay Slot

    tblwr          d.fence, 0
    // sqcb0 maintains copy of sq_cindex to enable speculation check. Increment
    //the copy on completion of wqe and write it into sqcb0
    tblmincri.c1    d.sq_cindex, d.log_sq_size, 1 

    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_HEADERS)
    // To start with, num_addr is 1 (bth)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, bth, bth, 1) // Branch Delay Slot
    cmov           r2, c2, K_OP_TYPE, d.curr_op_type
    .brbegin
    br             r2[2:0]    
    tblwr.c2       d.curr_op_type, K_OP_TYPE // Branch Delay Slot

    .brcase OP_TYPE_SEND
        .csbegin
        cswitch [c2, c1]
        nop
        .brcase RDMA_PKT_MIDDLE
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot

        .brcase RDMA_PKT_LAST
            phvwr          BTH_ACK_REQ, 1
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_LAST, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot

        .brcase RDMA_PKT_FIRST
            // check_credits = TRUE
            setcf          c5, [c0]

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_FIRST, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot

        .brcase RDMA_PKT_ONLY
            // Update num addrs to 2 if UD service (bth, deth)
            seq            c3, d.service, RDMA_SERV_TYPE_UD

            // check_credits = TRUE  : Keep this instruction here to avoid hazard for c3
            setcf          c5, [c0]

            phvwr.!c3      BTH_ACK_REQ, 1

            DMA_PHV2PKT_SETUP_CMDSIZE_C(r6, 2, c3)
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_ONLY, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        .csend

    .brcase OP_TYPE_SEND_INV
        .csbegin
        cswitch [c2, c1]
        tblwr.c2       d.inv_key, K_INV_KEY // Branch Delay Slot
        .brcase RDMA_PKT_MIDDLE
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        .brcase RDMA_PKT_LAST
            phvwr          BTH_ACK_REQ, 1
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
            phvwr          BTH_ACK_REQ, 1
            // check_credits = TRUE
            setcf          c5, [c0]

            // dma_cmd[2] - IETH hdr; num addrs 2 (bth, ieth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, ieth, ieth, 1)
            phvwr          IETH_R_KEY, K_INV_KEY

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_ONLY_WITH_INV, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        .csend

    .brcase OP_TYPE_SEND_IMM
        .csbegin
        cswitch [c2, c1]
        tblwr.c2       d.imm_data, K_IMM_DATA  // Branch Delay Slot
        
        .brcase RDMA_PKT_MIDDLE
            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
        .brcase RDMA_PKT_LAST
            phvwr          BTH_ACK_REQ, 1
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
            phvwr.!c3          BTH_ACK_REQ, 1

            DMA_PHV2PKT_SETUP_CMDSIZE_C(r6, 2, !c3)
            DMA_PHV2PKT_SETUP_CMDSIZE_C(r6, 3, c3)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N_C(r6, immeth, immeth, 1, !c3)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N_C(r6, immeth, immeth, 2, c3)

            phvwr          IMMDT_DATA, K_IMM_DATA

            b              op_type_end
            add            r2, RDMA_PKT_OPC_SEND_ONLY_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT
        .csend

    .brcase OP_TYPE_READ
        // inc_rrq_pindex = TRUE; adjust_psn = TRUE; inc_lsn = TRUE
        crestore       [c7, c6, c4], 0xd0, 0xd0
        
        // dma_cmd[2] - RETH hdr, num addrs 2 (bth, reth)
        DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, reth, reth, 1)
        
        // rrqwqe_p = rrq_base_addr + rrq_p_index * sizeof(rrqwqe_t)
        sll            r3, d.rrq_base_addr, RRQ_BASE_ADDR_SHIFT
        add            r3, r3, d.rrq_pindex, LOG_RRQ_WQE_SIZE
        
        phvwr          p.{rrqwqe.psn...rrqwqe.msn}, d.{tx_psn...ssn}
        
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
            phvwr          BTH_ACK_REQ, 1
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
            phvwr          BTH_ACK_REQ, 1
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
        tblwr.c2       d.imm_data, K_IMM_DATA  // Branch Delay Slot
        .brcase RDMA_PKT_MIDDLE
            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_MIDDLE, d.service, RDMA_OPC_SERV_TYPE_SHIFT // Branch Delay Slot
        
    .brcase RDMA_PKT_LAST
            phvwr          BTH_ACK_REQ, 1
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
            phvwr          BTH_ACK_REQ, 1
            // check_credits = TRUE
            setcf          c5, [c0]

            // dma_cmd[2] : addr1 - RETH hdr, num addrs 3 (bth, reth, immeth)
            DMA_PHV2PKT_SETUP_CMDSIZE(r6, 3)
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, reth, reth, 1)

            // dma_cmd[2] : addr2 - IMMETH hdr
            DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, immeth, immeth, 2)
            phvwr          IMMDT_DATA, K_IMM_DATA

            b              op_type_end
            add            r2, RDMA_PKT_OPC_RDMA_WRITE_ONLY_WITH_IMM, d.service, RDMA_OPC_SERV_TYPE_SHIFT //branch delay slot
        .csend

    .brcase OP_TYPE_CMP_N_SWAP
        // inc_rrq_pindex = TRUE; inc_lsn = TRUE;
        crestore       [c7, c4], 0x90, 0x90
        
        // dma_cmd[2] - ATOMICETH hdr, num addrs 2 (bth, atomiceth)
        DMA_PHV2PKT_SETUP_CMDSIZE(r6, 2)
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, atomiceth, atomiceth, 1)
        
        // rrqwqe_p = rrq_base_addr + rrq_pindex & sizeof(rrqwqe_t)
        sll            r3, d.rrq_base_addr, RRQ_BASE_ADDR_SHIFT
        add            r3, r3, d.rrq_pindex, LOG_RRQ_WQE_SIZE
        
        phvwr          RRQWQE_ATOMIC_OP_TYPE, K_OP_TYPE
        phvwr          p.{rrqwqe.psn...rrqwqe.msn}, d.{tx_psn...ssn}
        
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
        
        // rrqwqe_p = rrq_base_addr + rrq_pindex & sizeof(rrqwqe_t)
        sll            r3, d.rrq_base_addr, RRQ_BASE_ADDR_SHIFT
        add            r3, r3, d.rrq_pindex, LOG_RRQ_WQE_SIZE
        
        phvwr          RRQWQE_ATOMIC_OP_TYPE, K_OP_TYPE
        phvwr          p.{rrqwqe.psn...rrqwqe.msn}, d.{tx_psn...ssn}
        
        // dma_cmd[3] - rrqwqe
        DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RRQWQE)
        DMA_HBM_PHV2MEM_SETUP(r6, rrqwqe, rrqwqe, r3)
        
        b              op_type_end
        add            r2, RDMA_PKT_OPC_FETCH_ADD, d.service, RDMA_OPC_SERV_TYPE_SHIFT
    .brend

op_type_end:
    // phv_p->bth.pkey = 0xffff
    phvwr          BTH_PKEY, 0xffff  

    // Store cur tx_psn as exp_rsp_psn, if rsp is expected for this psn
    add            r7, d.tx_psn, r0

    // store wqe_start_psn for backtrack from middle of multi-packet msg
    tblwr.c2       d.wqe_start_psn, d.tx_psn

    b.!c6          inc_psn
    // phv_p->bth.psn = sqcb1_p->tx_psn
    phvwrpair      BTH_OPCODE, r2, BTH_PSN, d.tx_psn // Branch Delay Slot

    // if (adjust_psn)
    // tx_psn = read_len >> log_pmtu
    srl            r3, K_RD_READ_LEN, K_RD_LOG_PMTU
    tblmincr       d.tx_psn, 24, r3

    // tx_psn += (read_len & ((1 << log_pmtu) -1)) ? 1 : 0
    add            r3, K_RD_READ_LEN, r0
    mincr          r3, K_RD_LOG_PMTU, r0
    sle            c6, r3, r0

inc_psn:
    // sqcb1_p->tx_psn++
    tblmincri.!c6  d.tx_psn, 24, 1

    // if (rrqwqe_to_hdr_info_p->last)
    //     sqcb1_p->ssn++;
    tblmincri.c1   d.ssn, 24, 1

    // inc lsn for read, atomic, write (without imm)
    tblmincri.c4   d.lsn, 24, 1
   
    SQCB1_ADDR_GET(r1)
    add            r2, FIELD_OFFSET(sqcb1_t, tx_psn), r1
    memwr.d        r2, d.{tx_psn...lsn[23:8]}
    add            r2, r2, 8
    memwr.b        r2, d.lsn[7:0]

    // TODO:For RTL perf tests, disable check_credits
    setcf          c5, [!c0]

    // if (check_credits && (sqcb1_p->ssn > sqcb1_p->lsn))
    //     phv_p->bth.a = 1
    //     write_back_info_p->set_credits = TRUE
    scwlt24.c5     c5, d.lsn, d.ssn
    bcf            [!c5], rrq_p_index_chk
    SQCB0_ADDR_GET(r2) // Branch Delay Slot

    phvwr          BTH_ACK_REQ, 1
    // Disable TX scheduler for this QP until ack is received with credits to
    // send subsequent packets
    DOORBELL_NO_UPDATE_DISABLE_SCHEDULER(K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, SQ_RING_ID, r3, r5)

rrq_p_index_chk:
    // do we need to increment rrq_pindex ?
    bcf             [!c7], cb1_byte_update
    // cb1_byte is by default set to FALSE
    add             r5, r0, r0 // BD Slot

    // incr_rrq_p_index
    tblmincri       d.rrq_pindex, d.log_rrq_size, 1
    add             r3, r1, RRQ_P_INDEX_OFFSET
    phvwr           p.rrq_p_index, d.rrq_pindex

    // dma_cmd[4] - incr rrq_p_index for read/atomic
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RRQ_PINDEX)
    DMA_HBM_PHV2MEM_SETUP(r6, rrq_p_index, rrq_p_index, r3)

cb1_byte_update:
    // on top of it, set need_credits flag is conditionally
    add.c5         r5, r5, SQCB0_NEED_CREDITS_FLAG
    or             r5, r5, CAPRI_KEY_FIELD(IN_P, in_progress), SQCB0_IN_PROGRESS_BIT_OFFSET
    add            r3, r2, FIELD_OFFSET(sqcb0_t, cb1_byte)
    memwr.b        r3, r5

    tblwr          d.in_progress, CAPRI_KEY_FIELD(IN_P, in_progress)

    // Skip timer logic if not last/only or read/atomic requests
    bcf            [!c1 & !c7], load_hdr_template
    tblwr.c5       d.need_credits, 1 // Branch Delay Slot

    // if retransmit timer is disabled (local_ack_timeout = 0), do not track
    // timer expiry.
    // If response is already pending then do not modify lask_ack_or_req_ts.
    // If (rexmit_psn > previous exp_rsp_psn) update last_ack_or_req_ts
    // to current request timestamp and start timer if not runing already. In
    // either case, update exp_rsp_psn to indicate that more responses are
    // expected
    seq            c1, d.local_ack_timeout, 0
    scwle24        c2, d.rexmit_psn, d.exp_rsp_psn
    bcf            [c1 | c2], load_hdr_template
    tblwr          d.exp_rsp_psn, r7 // Branch Delay Slot
    tblwr          d.last_ack_or_req_ts, r4
    bbeq           d.timer_on, 1, load_hdr_template
    CAPRI_START_SLOW_TIMER(r1, r2, K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, TIMER_RING_ID, 10)
    tblwr          d.timer_on, 1

load_hdr_template:

    //For UD, ah_handle comes in send req
    seq            c3, d.service, RDMA_SERV_TYPE_UD
    cmov           r1, c3, K_AH_HANDLE, d.header_template_addr
    cmov           r2, c3, K_AH_SIZE, d.header_template_size

    // phv_p->bth.dst_qp = sqcb1_p->dst_qp if it is not UD service
    phvwr.!c3      BTH_DST_QP, d.dst_qp
    phvwrpair      P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA, \
                   P4PLUS_TO_P4_FLAGS, (P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN)

   // HACK: overloading timestamp field d to store flow_index for GFT.
#ifdef GFT
    phvwrpair          p.p4plus_to_p4.flow_index[15:0], d.timestamp, P4PLUS_TO_P4_FLOW_INDEX_VALID, 1
#else
    //not enough bits on d[] vector to have 64 bit TS value
    phvwr          p.{roce_options.TS_value[15:0]...roce_options.TS_echo[31:16]}, d.{timestamp...timestamp_echo}
#endif
    phvwrpair      p.roce_options.MSS_value, d.mss, p.roce_options.EOL_kind, ROCE_OPT_KIND_EOL

    CAPRI_RESET_TABLE_3_ARG()
    phvwrpair CAPRI_PHV_FIELD(ADD_HDR_T, hdr_template_inline), CAPRI_KEY_FIELD(IN_P, hdr_template_inline), CAPRI_PHV_FIELD(ADD_HDR_T, service), d.service
    phvwrpair CAPRI_PHV_FIELD(ADD_HDR_T, header_template_addr), r1, CAPRI_PHV_FIELD(ADD_HDR_T, header_template_size), r2
    phvwr CAPRI_PHV_RANGE(ADD_HDR_T, roce_opt_ts_enable, roce_opt_mss_enable), d.{roce_opt_ts_enable...roce_opt_mss_enable}

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_add_headers_2_process, r0)

    nop.e
    nop

fence:
    tblwr           d.fence, 1
    tblwr           d.fence_done, 0
    //fall-through
poll_fail:
rate_enforce_fail:
spec_fail:
    phvwr   p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_3_VALID(0)
    nop.e
    nop
