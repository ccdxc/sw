#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "sqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "nic/p4/common/defines.h"

struct resp_rx_phv_t p;
struct rqcb1_t d;
//struct resp_rx_rqcb_process_k_t k;
struct common_p4plus_stage0_app_header_table_k k;

#define RQCB_TO_WRITE_P t1_s2s_rqcb_to_write_rkey_info
#define INFO_OUT1_P t0_s2s_rqcb_to_pt_info
//#define RSQ_BT_S2S_INFO_T struct resp_rx_rsq_backtrack_info_t 
//#define RSQ_BT_TO_S_INFO_T struct resp_rx_to_stage_backtrack_info_t
#define RQCB_TO_RQCB1_P t0_s2s_rqcb_to_rqcb1_info
#define TO_S_RQPT_P to_s1_rqpt_info
#define TO_S_WB1_P to_s5_wb1_info
#define TO_S_WQE_P to_s2_wqe_info
#define TO_S_STATS_INFO_P to_s7_stats_info
#define RQCB_TO_RD_ATOMIC_P t1_s2s_rqcb_to_read_atomic_rkey_info
#define TO_S_ATOMIC_INFO_P to_s1_atomic_info
#define TO_S1_DCQCN_P to_s1_dcqcn_info
#define WQE_INFO_P t0_s2s_rqcb_to_wqe_info
#define TO_S_RKEY_P to_s2_ext_hdr_info
#define TO_S_LKEY_P to_s4_lkey_info
#define TO_S_CQCB_P to_s6_cqcb_info

#define REM_PYLD_BYTES  r6
#define RSQWQE_P r2
#define DMA_CMD_BASE r1
#define TOKEN_ID r1
#define IMM_DATA r2
#define DB_ADDR r4
#define DB_DATA r5
#define NEW_RSQ_P_INDEX r6

#define WORK_NOT_DONE_RECIRC_CNT_MAX    8
#define WQE_SIZE_2_SGES                 6

#define P4_INTR_RECIRC_COUNT_MAX        6
#define RDMA_RECIRC_ITER_COUNT_MAX      15

%%
    .param    resp_rx_rqcb3_in_progress_process
    .param    resp_rx_write_dummy_process
    .param    resp_rx_rqcb1_recirc_sge_process
    .param    resp_rx_rome_pkt_process
    .param    resp_rx_rome_cnp_process
    .param    resp_rx_dcqcn_cnp_process
    .param    resp_rx_dcqcn_config_load_process
    .param    rdma_atomic_resource_addr
    .param    resp_rx_read_mpu_only_process
    .param    resp_rx_atomic_resource_process
    .param    resp_rx_recirc_mpu_only_process
    .param    resp_rx_rqcb1_write_back_mpu_only_process
    .param    resp_rx_rqpt_process
    .param    resp_rx_dummy_rqpt_process
    .param    resp_rx_rqcb1_write_back_err_process
    .param    resp_rx_phv_drop_mpu_only_process

.align
resp_rx_rqcb_process:
    bcf             [c2 | c3 | c7], table_error
    // table 0 valid bit would be set by the time we get into S0 because
    // to get +64 logic, p4 program would have set table 0 valid bit to TRUE.
    // we need to this bit back to 0 right way otherwise table 0 gets fired
    // unnecessarily in further stages and cause wrong behavior (mainly for write/read/atomic)
    CAPRI_SET_TABLE_0_VALID(0) // BD Slot

    // feedback phv comes with RESP_RX_FLAG_ERR_DIS_QP. Take a detour.
    // feedback phv is processed right away without going thru any token_id checks.
    bbeq    CAPRI_APP_DATA_RAW_FLAG_ERR_DIS_QP, 1, process_feedback
    add     r7, r0, CAPRI_APP_DATA_RAW_FLAGS  //BD Slot

    // are we in a state to process received packets ?
    slt     c1, d.state, QP_STATE_RTR
    bcf     [c1], phv_drop

    // is this a fresh packet ?
    seq     c1, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0   //BD Slot
    bcf     [!c1], recirc_pkt
    cmov    TOKEN_ID, c1, d.token_id, CAPRI_APP_DATA_RECIRC_TOKEN_ID // BD Slot

    //fresh packet
    // populate global fields

    // see if number of WORK_NOT_DONE recirc packets in the pipeline has exceeded threshold.
    // if so, drop the packet for now
    slt     c2, d.work_not_done_recirc_cnt, WORK_NOT_DONE_RECIRC_CNT_MAX

    // see if received P_KEY is DEFAULT_PKEY
    seq     c3, CAPRI_APP_DATA_BTH_P_KEY, DEFAULT_PKEY

    bcf     [!c2 | !c3], phv_drop

skip_roce_opt_parsing:

    // get a tokenid for the fresh packet
    phvwr  p.common.rdma_recirc_token_id, TOKEN_ID  //BD Slot in fast path

    CAPRI_SET_FIELD2(TO_S_WB1_P, my_token_id, TOKEN_ID)
    // every fresh packet gets a unique token id 
    tbladd    d.token_id, 1 

//#if 0
    // recirc if busy==1 or work_not_done_recirc_cnt !=0
    bbeq    d.busy, 1, recirc_wait_for_turn
//#endif

    seq     c1, d.work_not_done_recirc_cnt, 0

    bcf     [!c1], recirc_wait_for_turn

start_recirc_packet:
    # subtract the pad bytes from the payload length.
    sub     REM_PYLD_BYTES, CAPRI_APP_DATA_PAYLOAD_LEN, CAPRI_APP_DATA_BTH_PAD // BD Slot in straight path

    bbeq     CAPRI_APP_DATA_RAW_FLAG_UD, 1, process_ud
    //set DMA cmd ptr
    RXDMA_DMA_CMD_PTR_SET(RESP_RX_DMA_CMD_START_FLIT_ID, 0) //BD Slot

    // c1 - congestion_mgmt_type = 0, congestion_mgmt not enable
    // c2 - ECN bits are not set
    // c3 - congestion_mgmt_type = 2, Rome(Y) Dcqcn(N)
    // c4 - OPCODE is 129, receive CNP pkt
    seq      c1, d.congestion_mgmt_type, 0
    bcf      [c1], skip_cnp_receive

    // check congestion management type is 1: Dcqcn or 2: Rome
    // if Rome, trigger rome_pkt_process for everytime except for CNP packet
    seq      c3, d.congestion_mgmt_type, 2
    bcf      [!c3], dcqcn_ecn_process
    seq      c4, CAPRI_APP_DATA_BTH_OPCODE, RDMA_PKT_OPC_CNP // BD Slot

    bcf      [!c4], rome_pkt_process
    add      r5, AH_ENTRY_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT //dcqcn_cb addr // BD Slot
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rome_cnp_process, r5)

rome_pkt_process:
    // do not reset table3, rome needs two piece of data from both rqcb_process and rqcb_process_ext
    addi     r5, r5, DCQCN_CB_T_SIZE_BYTES // rome_receiver_cb addr
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rome_pkt_process, r5)
    b        skip_cnp_receive
    nop // BD Slot

dcqcn_ecn_process:
    //Check if ECN bits are set in Packet
    sne      c2, k.rdma_bth_ecn, 3  
    bcf      [c2], skip_cnp_send
    nop     // BD Slot

    phvwr CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, np_ecn_marked_packets), 1
    // Trigger local doorbell to TXDMA CNP ring.
    DOORBELL_INC_PINDEX(CAPRI_RXDMA_INTRINSIC_LIF, Q_TYPE_RDMA_SQ, CAPRI_RXDMA_INTRINSIC_QID, CNP_RING_ID, DB_ADDR, DB_DATA)

skip_cnp_send:
    // Check if its CNP packet.
    bcf      [!c4], skip_cnp_receive

    add     r5, AH_ENTRY_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT //dcqcn_cb addr // BD Slot
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_dcqcn_cnp_process, r5)

    phvwr   CAPRI_PHV_FIELD(TO_S1_DCQCN_P, dcqcn_cfg_id), d.dcqcn_cfg_id
    // mpu-only process
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_dcqcn_config_load_process, r0)

skip_cnp_receive:
    // TODO: Migrate ACK_REQ flag to P4 table
    seq     c7, CAPRI_APP_DATA_BTH_ACK_REQ, 1
    or.c7   r7, r7, RESP_RX_FLAG_ACK_REQ

    crestore [c6, c5, c4, c3, c2, c1], r7, (RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_READ_REQ | RESP_RX_FLAG_SEND | RESP_RX_FLAG_ONLY)
    // c6: cswap, c5: fna, c4: write, c3: read, c2: send, c1: only

    CAPRI_SET_FIELD2(phv_global_common, spec_cindex, SPEC_RQ_C_INDEX)
    // do sanity checks
    // is pkt psn same as e_psn ?
    seq         c7, d.e_psn, CAPRI_APP_DATA_BTH_PSN
    tblwr.c7    d.nak_prune, 0
    bcf         [!c7], seq_err_or_duplicate

    seq         c7, CAPRI_APP_DATA_BTH_OPCODE[7:5], d.serv_type //BD Slot
    bcf         [!c7], inv_req_nak_serv_type

    // check if payload_len is <= pmtu
    sll         r3, 1, d.log_pmtu  //BD Slot
    blt         r3, REM_PYLD_BYTES, inv_req_nak_pyld_len_err
    // c7 is used in process_write_only down the path. Pls do not touch.
    crestore    [c7], r7, (RESP_RX_FLAG_IMMDT)  //BD Slot

    bcf     [c1 | c6 | c5 | c3], process_only_rd_atomic
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7) //BD Slot

/****** Multi-packet: SEND/WRITE FIRST/MIDDLE/LAST ******/
process_send_write_fml:
    crestore    [c6, c5, c4, c3, c2, c1], r7, (RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_SEND | RESP_RX_FLAG_LAST | RESP_RX_FLAG_MIDDLE | RESP_RX_FLAG_FIRST) 
    //  c6: immdt, c5: write, c4: send, c3: last, c2: middle, c1: first
    //  using c5 in below code, make sure its not used
    
    // last packet should be at least 1 byte
    beq.c3      REM_PYLD_BYTES, r0, inv_req_nak_last_pkt_len_err
    phvwr       CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, last_bth_opcode), CAPRI_APP_DATA_BTH_OPCODE // BD Slot

    // first/middle packets should be of pmtu size
    sne         c7, r3, REM_PYLD_BYTES
    bcf.c7      [c1|c2], inv_req_nak_pmtu_err

    phvwr       p.s1.ack_info.psn, d.e_psn // BD Slot

    bcf         [c4], process_send
    phvwrpair   p.cqe.qid, CAPRI_RXDMA_INTRINSIC_QID, p.cqe.type, CQE_TYPE_RECV //BD Slot
     
/****** Fast path: WRITE FIRST/MIDDLE/LAST ******/
process_write:
    // check QP level access permissions
    and         r5, d.access_flags, QP_ACCESS_REMOTE_WRITE
    beqi        r5, 0, inv_req_nak_access_err

    // check expected op type and pkt type for write packets 
    seq.c1      c7, d.{next_op_type...next_pkt_type}, (NEXT_OP_TYPE_ANY << 1)|NEXT_PKT_TYPE_FIRST_ONLY // BD Slot
    seq.!c1     c7, d.{next_op_type...next_pkt_type}, (NEXT_OP_TYPE_WRITE << 1)|NEXT_PKT_TYPE_MID_LAST
    bcf         [!c7], inv_req_nak_opcode_err

    // populate PD in rkey's to_stage
    CAPRI_SET_FIELD2(TO_S_RKEY_P, pd, d.pd) // BD Slot

    // increment e_psn
    tblmincri   d.e_psn, 24, 1

    // check if rnr case for Write Last with Immdt
    seq        c7, SPEC_RQ_C_INDEX, PROXY_RQ_P_INDEX
    bcf.c7      [c6 & c3], process_rnr

    // load rqcb3
    add     r5, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 3) //BD Slot
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_write_dummy_process, r5)

    // update the next op_type and pkt_type expected
    tblwr.c3    d.{next_op_type...next_pkt_type}, (NEXT_OP_TYPE_ANY << 1)|NEXT_PKT_TYPE_FIRST_ONLY
    tblwr.!c3   d.{next_op_type...next_pkt_type}, (NEXT_OP_TYPE_WRITE << 1)|NEXT_PKT_TYPE_MID_LAST

    // only first packet has reth header
    CAPRI_RESET_TABLE_1_ARG()

    phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_WRITE_P, pad), CAPRI_APP_DATA_BTH_PAD, \
                CAPRI_PHV_FIELD(RQCB_TO_WRITE_P, priv_oper_enable), d.priv_oper_enable

    bcf [!c1], write_non_first_pkt
    CAPRI_SET_FIELD2_C(RQCB_TO_WRITE_P, load_reth, 1, !c1)  //BD Slot
    CAPRI_SET_FIELD_RANGE2(RQCB_TO_WRITE_P, va, len, CAPRI_RXDMA_RETH_VA_R_KEY_LEN)

write_non_first_pkt:
    bcf             [!c6], exit
    CAPRI_SET_FIELD2(RQCB_TO_WRITE_P, remaining_payload_bytes, REM_PYLD_BYTES) //BD Slot

    seq         c7, d.immdt_as_dbell, 1
    bcf         [!c7], wr_skip_immdt_as_dbell
    add         IMM_DATA, r0, CAPRI_RXDMA_BTH_IMMETH_IMMDATA

    //handle immdt_as_dbell
    and         r7, r7, ~(RESP_RX_FLAG_IMMDT)
    and         r7, r7, ~(RESP_RX_FLAG_COMPLETION)
    or          r7, r7, RESP_RX_FLAG_RING_DBELL
    
    //IMM_DATA has imm_data
    //format: <lif(11), qtype(3), qid(18)>
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_IMMDT_AS_DBELL)
    RESP_RX_POST_IMMDT_AS_DOORBELL_INCR_PINDEX(DMA_CMD_BASE, \
                                   IMM_DATA[31:21], \
                                   IMM_DATA[20:18], \
                                   IMM_DATA[17:0], \
                                   DB_ADDR, DB_DATA)
    DMA_SET_END_OF_CMDS(struct capri_dma_cmd_pkt2mem_t, DMA_CMD_BASE)

    b           exit
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7) //BD Slot

wr_skip_immdt_as_dbell:
    CAPRI_SET_FIELD2(RQCB_TO_WRITE_P, incr_c_index, 1)
    phvwr       p.cqe.recv.op_type, OP_TYPE_CQE_RECV_RDMA_IMM
    b           rc_checkout
    phvwr       p.cqe.recv.imm_data, IMM_DATA //BD Slot

/****** Slow path: SEND FIRST/MIDDLE/LAST ******/
process_send:
    //  c6: immdt, c5: write, c4: send, c3: last, c2: middle, c1: first

    seq         c7, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    // c7: fresh packet
    seq         c4, d.spec_en, 1
    // c4: send_sge_opt

    // if recirc pkt (token_id check already done) or
    // spec_en, skip token_id_check
    bcf         [!c7 | c4], skip_token_id_check
    seq         c7, TOKEN_ID, d.nxt_to_go_token_id // BD Slot
    bcf         [!c7], recirc_wait_for_turn
    phvwr       p.cqe.recv.op_type, OP_TYPE_CQE_RECV_SEND // BD Slot

skip_token_id_check:
    // check expected op type and pkt type for send packets 
    seq.c1      c7, d.{next_op_type...next_pkt_type}, (NEXT_OP_TYPE_ANY << 1)|NEXT_PKT_TYPE_FIRST_ONLY
    seq.!c1     c7, d.{next_op_type...next_pkt_type}, (NEXT_OP_TYPE_SEND << 1)|NEXT_PKT_TYPE_MID_LAST
    bcf         [!c7], inv_req_nak_opcode_err

    // check if rnr case for Send First
    seq         c7, SPEC_RQ_C_INDEX, PROXY_RQ_P_INDEX // BD Slot
    bcf.c7      [c1], process_rnr
    // increment e_psn
    tblmincri    d.e_psn, 24, 1 // BD Slot

    // update the next op_type and pkt_type expected
    tblwr.c3    d.{next_op_type...next_pkt_type}, (NEXT_OP_TYPE_ANY << 1)|NEXT_PKT_TYPE_FIRST_ONLY
    tblwr.!c3   d.{next_op_type...next_pkt_type}, (NEXT_OP_TYPE_SEND << 1)|NEXT_PKT_TYPE_MID_LAST

    bcf         [!c4], send_sge_non_opt
    tblmincri.c4    d.send_info.spec_psn, 24, 1 // BD Slot

    CAPRI_SET_FIELD2(TO_S_RQPT_P, send_sge_opt, 1)
    CAPRI_SET_FIELD2(TO_S_WB1_P, send_sge_opt, 1)

    b           send_sge_opt
    // if 2 SGE's and FIRST packet, set spec_psn to 0
    tblwr.c1    d.send_info.spec_psn, 0 // BD Slot

send_sge_non_opt:
    // populate PD in lkey's to_stage
    CAPRI_SET_FIELD2(TO_S_LKEY_P, pd, d.pd)

    crestore    [c7, c6, c5], r7, (RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_IMMDT)
    // c7: completion, c6: inv_rkey, c5: immdt

    // if SEND_FIRST, we simply need to checkout a descriptor
    bcf         [c1], rc_checkout 
    phvwr       CAPRI_PHV_FIELD(TO_S_WQE_P, priv_oper_enable), d.priv_oper_enable //BD Slot

    setcf       c4, [c6 | c5]

    // if SEND_MIDDLE OR immediate/inv_rkey is not present, 
    // we simply need to go to in_progress path
    bcf         [c2 | !c4], send_in_progress
    seq         c7, CAPRI_RXDMA_BTH_IETH_R_KEY, RDMA_RESERVED_LKEY_ID //BD Slot

    bcf         [!c6], send_check_immdt
    phvwr.c7    CAPRI_PHV_FIELD(TO_S_LKEY_P, rsvd_key_err), 1   //BD Slot

    phvwrpair   p.cqe.recv.op_type, OP_TYPE_CQE_RECV_SEND_INV, p.cqe.recv.r_key, CAPRI_RXDMA_BTH_IETH_R_KEY 
    CAPRI_SET_FIELD2(TO_S_WQE_P, inv_r_key, CAPRI_RXDMA_BTH_IETH_R_KEY)

send_check_immdt:
    bcf         [!c5], send_in_progress
    seq         c7, d.immdt_as_dbell, 1     //BD Slot
    bcf         [!c7], send_skip_immdt_as_dbell
    phvwr.!c7   p.cqe.recv.op_type, OP_TYPE_CQE_RECV_SEND_IMM // BD Slot

    //handle immdt_as_dbell
    and         r7, r7, ~(RESP_RX_FLAG_IMMDT)
    or          r7, r7, RESP_RX_FLAG_RING_DBELL
    
    //IMM_DATA has imm_data
    //format: <lif(11), qtype(3), qid(18)>
    add         IMM_DATA, r0, CAPRI_RXDMA_BTH_IMMETH_IMMDATA
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_IMMDT_AS_DBELL)
    RESP_RX_POST_IMMDT_AS_DOORBELL_SET_PINDEX(DMA_CMD_BASE, \
                                   IMM_DATA[31:21], \
                                   IMM_DATA[20:18], \
                                   IMM_DATA[17:0], \
                                   DB_ADDR, DB_DATA)

    b           send_in_progress
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7) //BD Slot

send_skip_immdt_as_dbell:
    phvwr       p.cqe.recv.imm_data, CAPRI_RXDMA_BTH_IMMETH_IMMDATA //BD Slot

send_in_progress:
    // load rqcb3 to get wrid
    add     r5, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 3)
    // if send FML, and last pkt, increment spec_c_index
    tblmincri.c3.f SPEC_RQ_C_INDEX, d.log_num_wqes, 1
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqcb3_in_progress_process, r5)

    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_RQCB1_P, log_pmtu), d.log_pmtu, \
                CAPRI_PHV_FIELD(RQCB_TO_RQCB1_P, log_rq_page_size), d.log_rq_page_size
    phvwr       CAPRI_PHV_RANGE(RQCB_TO_RQCB1_P, curr_wqe_ptr, num_sges), d.{curr_wqe_ptr...num_sges}

    b           exit
    phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_RQCB1_P, in_progress), d.in_progress, \
                CAPRI_PHV_FIELD(RQCB_TO_RQCB1_P, remaining_payload_bytes), REM_PYLD_BYTES //BD Slot


/******  Common logic for ONLY packets (send/write) ******/
process_only_rd_atomic:
    // check expected op type and pkt type for only packets 
    seq         c1, d.{next_op_type...next_pkt_type}, (NEXT_OP_TYPE_ANY << 1)|NEXT_PKT_TYPE_FIRST_ONLY
    bcf         [!c1], inv_req_nak_opcode_err
    phvwr       CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, last_bth_opcode), CAPRI_APP_DATA_BTH_OPCODE // BD Slot

    bcf         [c6 | c5 | c3], process_read_atomic
    phvwr       p.s1.ack_info.psn, d.e_psn // BD Slot

    // populate completion entry
    bcf         [c4], process_write_only
    phvwrpair   p.cqe.qid, CAPRI_RXDMA_INTRINSIC_QID, p.cqe.type, CQE_TYPE_RECV //BD Slot

/******  Logic for SEND_ONLY packets ******/
// send SGE opt is same as send only since
// all packets in the multi packet msg will 
// branch to rc_checkout and load rqpt_process
// or dummy_rqpt
process_send_only:
    // increment e_psn
    tblmincri  d.e_psn, 24, 1

    // check if rnr case for Send Only
    seq        c7, SPEC_RQ_C_INDEX, PROXY_RQ_P_INDEX
    bcf        [c7], process_rnr

    tblwr      d.send_info.spec_psn, 0 // BD Slot
    seq        c4, d.spec_en, 1

    CAPRI_SET_FIELD2_C(TO_S_RQPT_P, send_sge_opt, 1, c4)
    CAPRI_SET_FIELD2_C(TO_S_WB1_P, send_sge_opt, 1, c4)

send_sge_opt:

    // populate PD in lkey's to_stage
    CAPRI_SET_FIELD2(TO_S_LKEY_P, pd, d.pd) // BD Slot in straight path
    phvwrpair   CAPRI_PHV_FIELD(TO_S_WQE_P, spec_psn), d.send_info.spec_psn, \
                CAPRI_PHV_FIELD(TO_S_WQE_P, priv_oper_enable), d.priv_oper_enable

    // handle immdiate data and inv_r_key
    crestore    [c7, c6], r7, (RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_IMMDT) // BD Slot
    // c7: inv_rkey, c6: immdt
    bcf         [!c7 & !c6], rc_checkout
    phvwr       p.cqe.recv.op_type, OP_TYPE_CQE_RECV_SEND //BD Slot

    bcf         [!c7], send_only_check_immdt
    seq         c5, CAPRI_RXDMA_BTH_IETH_R_KEY, RDMA_RESERVED_LKEY_ID //BD Slot
    phvwr.c5    CAPRI_PHV_FIELD(TO_S_LKEY_P, rsvd_key_err), 1

    phvwrpair   p.cqe.recv.op_type, OP_TYPE_CQE_RECV_SEND_INV, p.cqe.recv.r_key, CAPRI_RXDMA_BTH_IETH_R_KEY 
    phvwr       p.cqe.recv.r_key, CAPRI_RXDMA_BTH_IETH_R_KEY

    CAPRI_SET_FIELD2(TO_S_WQE_P, inv_r_key, CAPRI_RXDMA_BTH_IETH_R_KEY)

send_only_check_immdt:
    bcf         [!c6], rc_checkout
    seq         c7, d.immdt_as_dbell, 1     //BD Slot
    bcf         [!c7], send_only_skip_immdt_as_dbell
    phvwr.!c7   p.cqe.recv.op_type, OP_TYPE_CQE_RECV_SEND_IMM // BD Slot

    //handle immdt_as_dbell
    and         r7, r7, ~(RESP_RX_FLAG_IMMDT)
    or          r7, r7, RESP_RX_FLAG_RING_DBELL
    
    //IMM_DATA has imm_data
    //format: <lif(11), qtype(3), qid(18)>
    add         IMM_DATA, r0, CAPRI_RXDMA_BTH_IMMETH_IMMDATA
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_IMMDT_AS_DBELL)
    RESP_RX_POST_IMMDT_AS_DOORBELL_SET_PINDEX(DMA_CMD_BASE, \
                                   IMM_DATA[31:21], \
                                   IMM_DATA[20:18], \
                                   IMM_DATA[17:0], \
                                   DB_ADDR, DB_DATA)

    b           rc_checkout
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7) //BD Slot

send_only_skip_immdt_as_dbell:
    b           rc_checkout
    phvwr       p.cqe.recv.imm_data, CAPRI_RXDMA_BTH_IMMETH_IMMDATA //BD Slot


/******  Logic for WRITE_ONLY packets ******/
process_write_only:
    // check QP level access permissions
    and         r5, d.access_flags, QP_ACCESS_REMOTE_WRITE
    beqi        r5, 0, inv_req_nak_access_err

    // is it zero length write request ?
    seq         c5, CAPRI_RXDMA_RETH_DMA_LEN, 0 // BD Slot

    // increment e_psn
    tblmincri   d.e_psn, 24, 1

    // c7 is initialized just before process_rd_only_atomic.
    // pls do not touch
    tblwr.!c7.f d.busy, d.busy

    bcf         [c5], wr_only_zero_len
    CAPRI_RESET_TABLE_1_ARG()   //BD Slot

    // populate PD in rkey's to_stage
    CAPRI_SET_FIELD2(TO_S_RKEY_P, pd, d.pd)
    CAPRI_SET_FIELD_RANGE2(RQCB_TO_WRITE_P, va, len, CAPRI_RXDMA_RETH_VA_R_KEY_LEN)
    phvwrpair    CAPRI_PHV_FIELD(RQCB_TO_WRITE_P, remaining_payload_bytes), REM_PYLD_BYTES, \
                 CAPRI_PHV_FIELD(RQCB_TO_WRITE_P, pad), CAPRI_APP_DATA_BTH_PAD
    CAPRI_SET_FIELD2(RQCB_TO_WRITE_P, priv_oper_enable, d.priv_oper_enable)

    bcf             [c7], write_only_immdt
    // load rqcb3
    add     r5, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 3) // BD Slot

    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_write_dummy_process, r5)    //Exit Slot

write_only_immdt:
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_write_dummy_process, r5)
    bbeq        d.immdt_as_dbell, 0, wr_only_skip_immdt_as_dbell
    CAPRI_RXDMA_BTH_RETH_IMMETH_IMMDATA_C(IMM_DATA, c0) //BD Slot

    //handle immdt_as_dbell
    and         r7, r7, ~(RESP_RX_FLAG_IMMDT)
    and         r7, r7, ~(RESP_RX_FLAG_COMPLETION)
    or          r7, r7, RESP_RX_FLAG_RING_DBELL
    
    //IMM_DATA has imm_data
    //format: <lif(11), qtype(3), qid(18)>
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_IMMDT_AS_DBELL)
    RESP_RX_POST_IMMDT_AS_DOORBELL_INCR_PINDEX(DMA_CMD_BASE, \
                                   IMM_DATA[31:21], \
                                   IMM_DATA[20:18], \
                                   IMM_DATA[17:0], \
                                   DB_ADDR, DB_DATA)
    DMA_SET_END_OF_CMDS_E(struct capri_dma_cmd_pkt2mem_t, DMA_CMD_BASE)
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7) //Exit Slot
    
wr_only_skip_immdt_as_dbell:
    // check if rnr case for Write Only with Immdt
    seq        c5, SPEC_RQ_C_INDEX, PROXY_RQ_P_INDEX
    bcf        [c5], process_rnr
    CAPRI_SET_TABLE_1_VALID_C(c5, 0)

    phvwr       p.cqe.recv.op_type, OP_TYPE_CQE_RECV_RDMA_IMM
    CAPRI_SET_FIELD2(RQCB_TO_WRITE_P, incr_c_index, 1)

    b           rc_checkout
    phvwr       p.cqe.recv.imm_data, IMM_DATA //BD Slot

wr_only_zero_len:
    // zero length write requets are identified by reth.dma_len == 0.
    // if reth.dma_len is 0, but payload length is not 0, standard is not clear on
    // the behavior. currently we generate nak with invalid request.
    // for zero length write requests, r_key validations should not be done and hence
    // we could directly invoke write back stage.
    seq         c6, REM_PYLD_BYTES, 0
    bcf         [!c6], wr_only_zero_len_inv_req_nak

    CAPRI_RESET_TABLE_2_ARG()   // BD Slot

    // no need to check rkey
    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

    // if there is no immediate data, we can right away generate ack
    bcf         [!c7], wr_only_zero_len_no_imm_data
    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1

wr_only_zero_len_with_imm_data:
    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, incr_c_index), 1

    CAPRI_RXDMA_BTH_RETH_IMMETH_IMMDATA_C(IMM_DATA, c0)
    phvwr       p.cqe.recv.op_type, OP_TYPE_CQE_RECV_RDMA_IMM
    b           rc_checkout
    phvwr       p.cqe.recv.imm_data, IMM_DATA //BD Slot

wr_only_zero_len_no_imm_data:
    // in case of wr_zero_len_no_imm_data, we need to have a dummy command to trigger CMD_EOP.
    DMA_CMD_STATIC_BASE_GET_E(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)
    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 1 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/) //Exit Slot

/****** Logic for READ/ATOMIC packets ******/
process_read_atomic:
    // check QP level access permissions
    and.c3      r5, d.access_flags, QP_ACCESS_REMOTE_READ
    and.!c3     r5, d.access_flags, QP_ACCESS_REMOTE_ATOMIC
    beqi        r5, 0, inv_req_nak_access_err

    // for read and atomic, start DMA commands from flit 9 instead of 8
    RXDMA_DMA_CMD_PTR_SET(RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, 0) // BD Slot

    // turn off ACK req bit for read/atomic
    // so that ACK doorbell is not rung
    and         r7, r7, ~(RESP_RX_FLAG_ACK_REQ)
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7)
    // populate PD in rkey's to_stage
    CAPRI_SET_FIELD2(TO_S_RKEY_P, pd, d.pd)
    // wqe_p = (void *)(hbm_addr_get(rqcb_p->rsq_base_addr) +    
    //                      (sizeof(rsqwqe_t) * p_index));
    add         NEW_RSQ_P_INDEX, r0, d.rsq_pindex
    sll         RSQWQE_P, d.rsq_base_addr, RSQ_BASE_ADDR_SHIFT
    add         RSQWQE_P, RSQWQE_P, NEW_RSQ_P_INDEX, LOG_SIZEOF_RSQWQE_T
    // p_index/c_index are in little endian
    mincr       NEW_RSQ_P_INDEX, d.log_rsq_size, 1
   
    // DMA for RSQWQE
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RSQWQE)
    
    // common params for both read/atomic
    CAPRI_RESET_TABLE_1_ARG()
    phvwrpair   p.rsqwqe.read.r_key, CAPRI_RXDMA_RETH_R_KEY, p.rsqwqe.read.va, CAPRI_RXDMA_RETH_VA
    CAPRI_SET_FIELD_RANGE2(RQCB_TO_RD_ATOMIC_P, va, r_key, CAPRI_RXDMA_RETH_VA_R_KEY)
    phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_RD_ATOMIC_P, rsq_p_index), NEW_RSQ_P_INDEX, \
                CAPRI_PHV_FIELD(RQCB_TO_RD_ATOMIC_P, priv_oper_enable), d.priv_oper_enable

    bcf         [c6 | c5], process_atomic
    phvwr       p.rsqwqe.psn, d.e_psn   //BD Slot

process_read:
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, rsqwqe, rsqwqe, RSQWQE_P)

    phvwr       p.rsqwqe.read.len, CAPRI_RXDMA_RETH_DMA_LEN
    CAPRI_SET_FIELD2(RQCB_TO_RD_ATOMIC_P, len, CAPRI_RXDMA_RETH_DMA_LEN)
    
    //increment e_psn by 'n'
    // e_psn += read_len >> log_pmtu
    srl            r3, CAPRI_RXDMA_RETH_DMA_LEN, d.log_pmtu
    tblmincr       d.e_psn, 24, r3

    // if read_len is < pmtu, then increment e_psn by 1
    sne            c6, r3, r0

    // e_psn += (read_len & ((1 << log_pmtu) -1)) ? 1 : 0
    add.c6         r3, CAPRI_RXDMA_RETH_DMA_LEN, r0
    mincr.c6       r3, d.log_pmtu, r0
    sle.c6         c6, r3, r0
    tblmincri.!c6  d.e_psn, 24, 1
    
    // flush the last tblwr in this path
    tblwr.f        d.rsq_pindex, NEW_RSQ_P_INDEX

    // do a MPU-only lookup
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_read_mpu_only_process, r0)

process_atomic:
    // if VA in atomic req is not aligned to an 8-byte boundary,
    // generate invalid req NAK
    // Lower order 3 bits should be 0 for 8-byte alignment
    // va & 0x7 != 0, error
    smneb          c1, CAPRI_RXDMA_RETH_VA, 0x7, 0
    bcf            [c1], inv_req_nak_atomic_va_err
    CAPRI_SET_FIELD2(RQCB_TO_RD_ATOMIC_P, read_or_atomic, RSQ_OP_TYPE_ATOMIC) // BD Slot

//#if 0
    /* Only atomic requests update the busy bit
       Set busy bit so that other requests
       don't end up modifying e_psn, RSQ pindex etc
       before atomic errors out in stage 1
     */ 
    tblwr           d.busy, 1
//#endif

    tblwr           d.rsq_pindex, NEW_RSQ_P_INDEX
    // increment e_psn
    // flush the last tblwr in this path
    tblmincri.f     d.e_psn, 24, 1

    CAPRI_SET_FIELD2(TO_S_ATOMIC_INFO_P, rsqwqe_ptr, RSQWQE_P)

    // do a MPU-only lookup
    addui           r3, r0, hiword(rdma_atomic_resource_addr)
    addi            r3, r3, loword(rdma_atomic_resource_addr)            
    // load 32 Bytes (256-bits) of atomic resource
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_256_BITS, resp_rx_atomic_resource_process, r3)
    
    bcf             [c6], process_cswap
    phvwr           p.rsqwqe.read_or_atomic, RSQ_OP_TYPE_ATOMIC // BD Slot

process_fna:
    //c5:fna
    CAPRI_RXDMA_BTH_ATOMICETH_SWAP_OR_ADD_DATA(r5)
    phvwrpair.e     p.pcie_atomic.atomic_type, PCIE_ATOMIC_TYPE_FNA, p.pcie_atomic.tlp_len, PCIE_TLP_LEN_FNA
    phvwr           p.pcie_atomic.compare_data_or_add_data, r5.dx //Exit Slot

process_cswap:
    //c6:cswap
    
    //CAPRI_RXDMA_BTH_ATOMICETH_CMP_DATA1(r5)
    //phvwr           p.pcie_atomic.compare_data_or_add_data, r5.wx
    CAPRI_RXDMA_BTH_ATOMICETH_SWAP_OR_ADD_DATA(r5)
    phvwrpair.e     p.pcie_atomic.atomic_type, PCIE_ATOMIC_TYPE_CSWAP, p.pcie_atomic.tlp_len, PCIE_TLP_LEN_CSWAP
    phvwr           p.pcie_atomic.swap_data, r5.dx  //Exit Slot

recirc_atomic_rnr:
    // decrement rsq_pindex
    add         r1, r0, -1 
    tblmincr    d.rsq_pindex, d.log_rsq_size, r1

    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_OUT_OF_ATOMIC_RESOURCE_OFFSET))

    b           process_rnr_atomic
    // enable speculation
    tblwr       d.busy, 0 // BD Slot

recirc_work_done:
    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

recirc_error_disable_qp:
    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, error_disable_qp), 1
    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

/****** Logic for handling out-of-order packets ******/
seq_err_or_duplicate:
    scwlt24     c7, d.e_psn, CAPRI_APP_DATA_BTH_PSN
    bcf         [c7], seq_err_nak
    // restore some flags in BD slot so that this piece of code 
    // can be shared between fast path and slow path
    crestore    [c6, c5, c4, c3, c2], r7, (RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_READ_REQ | RESP_RX_FLAG_SEND) //BD Slot

duplicate:
    // if cswap/fna/read, branch out
    bcf         [c6 | c5 | c3], duplicate_rd_atomic

    // for duplicate packets, set this flag so that writeback
    // does not overwrite in_progress field for FML packets
    // this is also needed for duplicate read and atomic packets
    phvwrpair       CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1, \
                    CAPRI_PHV_FIELD(TO_S_WB1_P, soft_nak_or_dup), 1 // BD Slot

duplicate_wr_send:
    /* 
        ACK generation for duplicate wr/send was being done by
        ringing the ack_nak ring and reusing the aeth header in rqcb2_t.
        This would not be the right thing to do if there was a NAK generated 
        before responding to the duplicate request, because its AETH would
        get reused. Modifying the code to ensure ACK is generated by 
        populating the AETH header in rqcb2_t here.
    */
    sub         r2, d.e_psn, 1 // since d.e_psn is a 24-bit value, sub can be used to decrement
    phvwr       p.s1.ack_info.psn, r2
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, dup_wr_send), 1, \
                CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_DUP_REQUEST_OFFSET))

generate_ack:
    // forcefully turn on ACK req bit
    or          r7, r7, RESP_RX_FLAG_ACK_REQ
    // clear inv_rkey flag if set
    and         r7, r7, ~(RESP_RX_FLAG_INV_RKEY)
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7)

    //Generate DMA command to skip to payload end
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)

    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 1 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/)
    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0) //Exit Slot

duplicate_rd_atomic:
    // drop further duplicate processing if threre is already another duplicate req in progress
    // it is most likely that further duplicates are with later psn values than the current 
    // duplicate whose backtrack is in progress (assuming requester is doing go-back-n). in this
    // case, droping further duplicate processing is ok because, upon backtracking to the psn of
    // current duplicate under process, all the further packets are anyway re-generated due 
    // to the way RSQ backtrack is implemented in new logic.
    // in case the duplicate that is being dropped is having earlier psn value, requester will
    // retry anyway. If we want to optimize this case, instead of blindly dropping the duplicate,
    // we could load rqcb2 to see if the new duplicate psn is earlier than what is under process 
    // and recirc it instead of dropping it.

    RXDMA_DMA_CMD_PTR_SET(RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, 0)
    // turn off ACK req bit for read/atomic
    // so that ACK doorbell is not rung
    and         r7, r7, ~(RESP_RX_FLAG_ACK_REQ)
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7)

    // load writeback MPU only. in stage 5, writeback will increment nxt_to_go_token_id
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

    seq             c1, d.bt_in_progress, 1
    bcf             [c1], drop_duplicate_rd_atomic
    tblwr.!c1.f     d.bt_in_progress, 1 //BD Slot

    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, dup_rd_atomic_bt), 1, \
                CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_DUP_REQUEST_OFFSET))
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_START)

    // copy bt_info to rqcb2
    add             r6, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, \
                    ((CB_UNIT_SIZE_BYTES * 2) + FIELD_OFFSET(rqcb2_t, bt_info))
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, s2.bt_info, s2.bt_info, r6)

    // ring backtrack doorbell to wakeup txdma
    DMA_NEXT_CMD_I_BASE_GET(DMA_CMD_BASE, 1)
    PREPARE_DOORBELL_INC_PINDEX(CAPRI_RXDMA_INTRINSIC_LIF, \
                                CAPRI_RXDMA_INTRINSIC_QTYPE, \
                                CAPRI_RXDMA_INTRINSIC_QID, \
                                BT_RING_ID, DB_ADDR, DB_DATA)
    phvwr           p.db_data1, DB_DATA.dx
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data1, db_data1, DB_ADDR)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
    
    // copy psn/va/r_key/len and op_type
    phvwr.c3        p.s2.bt_info.len, CAPRI_RXDMA_RETH_DMA_LEN
    phvwr.!c3       p.s2.bt_info.read_or_atomic, RSQ_OP_TYPE_ATOMIC
    phvwr.e         p.s2.bt_info.psn, CAPRI_APP_DATA_BTH_PSN
    phvwr           p.{s2.bt_info.va...s2.bt_info.r_key}, CAPRI_RXDMA_RETH_VA_R_KEY //Exit Slot
    
drop_duplicate_rd_atomic:
    phvwr.e     p.common.p4_intr_global_drop, 1
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, dup_rd_atomic_drop), 1, \
                CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_DUP_REQUEST_OFFSET)) // Exit Slot

/****** Logic for NAKs ******/
inv_req_nak_serv_type:
    phvwr       CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, last_bth_opcode), CAPRI_APP_DATA_BTH_OPCODE
    b           inv_req_nak
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_svc_type_err), 1 // BD Slot

inv_req_nak_pyld_len_err:
    phvwr       CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, last_bth_opcode), CAPRI_APP_DATA_BTH_OPCODE
    b           inv_req_nak
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_pyld_len_err), 1 // BD Slot

inv_req_nak_last_pkt_len_err:
    b           inv_req_nak
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_last_pkt_len_err), 1 // BD Slot

inv_req_nak_pmtu_err:
    b           inv_req_nak
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_pmtu_err), 1 // BD Slot

inv_req_nak_opcode_err:
    b           inv_req_nak
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_opcode_err), 1 // BD Slot

inv_req_nak_atomic_va_err:
    b           inv_req_nak
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_unaligned_atomic_va_err), 1 // BD Slot

inv_req_nak_access_err:
    b           inv_req_nak
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_access_err), 1 // BD Slot

wr_only_zero_len_inv_req_nak:
    //revert the e_psn
    sub         r1, 0, 1
    // flush the last tblwr in this path
    tblmincr.f  d.e_psn, 24, r1 // BD Slot
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_wr_only_zero_len_err), 1
    //fall thru to inv_req_nak

inv_req_nak:

    phvwr       CAPRI_PHV_FIELD(TO_S_CQCB_P, async_error_event), 1
    phvwrpair   p.s1.eqwqe.code, EQE_CODE_QP_ERR_REQEST, p.s1.eqwqe.type, EQE_TYPE_QP
    phvwr       p.s1.eqwqe.qid, CAPRI_RXDMA_INTRINSIC_QID
    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_INV_REQUEST_OFFSET))

    phvwrpair   CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1, \
                CAPRI_PHV_FIELD(TO_S_WB1_P, async_or_async_error_event), 1

    bbeq        d.nak_prune, 1, skip_nak
    // turn off ACK req bit here. if nak needs to be sent, we will turn it on below
    and         r7, r7, ~(RESP_RX_FLAG_ACK_REQ) // BD Slot

    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_INV_REQ)
    
    //Set c5 to TRUE to signify ASYNC path
    setcf       c5, [c0]
    b           nak
    or          r7, r7, RESP_RX_FLAG_ERR_DIS_QP // BD Slot

seq_err_nak:
    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_OUT_OF_SEQ_OFFSET))
    b           nak_prune
    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_SEQ_ERR) // BD Slot

process_rnr:
    // memwr timestamp into rqcb3 if spec_cindex = proxy_pindex (RNR)
    add         r1, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (3*CB_UNIT_SIZE_BYTES)
    add         r1, r1, FIELD_OFFSET(rqcb3_t, resp_rx_timestamp)
    memwr.h     r1, r4
    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_OUT_OF_BUFFER_OFFSET))
process_rnr_atomic:
    //revert the e_psn
    sub         r1, 0, 1 
    tblmincr    d.e_psn, 24, r1
    phvwr       p.s1.ack_info.syndrome, AETH_RNR_SYNDROME_INLINE_GET(RNR_NAK_TIMEOUT)    

nak_prune:
    /* only seq_err_nak and RNR
       should update nak_prune
     */
    //Set c5 to FALSE to signify recoverable errors like seq_err and RNR
    setcf       c5, [!c0]
    phvwrpair   CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1, \
                CAPRI_PHV_FIELD(TO_S_WB1_P, soft_nak_or_dup), 1
    bbne        d.nak_prune, 1, nak
    // no further tblwr's in either path. so add .f
    tblwr.f     d.nak_prune, 1 // BD Slot

    b           skip_nak
    // turn off ACK req bit when skipping nak
    and         r7, r7, ~(RESP_RX_FLAG_ACK_REQ) // BD Slot

nak: 
    phvwr       p.s1.ack_info.psn, d.e_psn
    // forcefully turn on ACK req bit and invoke writeback with incr_nxt_to_go_token_id
    or          r7, r7, RESP_RX_FLAG_ACK_REQ
 
skip_nak:
    // common to both nak and skip_nak
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7)

    //Generate DMA command to skip to payload end
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)

    //For ASYNC cases, only set SKIP_TO_EOP and not CMD_EOP. CMD_EOP is set by eqcb_process
    DMA_SKIP_CMD_SETUP_C(DMA_CMD_BASE, 1 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/, !c5)
    DMA_SKIP_CMD_SETUP_C(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/, c5)

    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

/****** Logic for UD ******/

process_ud:
    crestore [c2, c1], r7, (RESP_RX_FLAG_SEND | RESP_RX_FLAG_ONLY)
    // c2: send, c1: only

    phvwr       CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, last_bth_opcode), CAPRI_APP_DATA_BTH_OPCODE
    phvwr       CAPRI_PHV_FIELD(TO_S_WQE_P, priv_oper_enable), d.priv_oper_enable
    seq         c7, CAPRI_APP_DATA_BTH_OPCODE[7:5], d.serv_type
    seq         c3, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    seq         c4, SPEC_RQ_C_INDEX, PROXY_RQ_P_INDEX
    // if it doesn't match serv_type OR not send OR not only OR no posted buffers, drop
    bcf         [!c7 | !c2 | !c1 | c4], ud_phv_drop
    // check if payload_len is <= pmtu
    sll         r1, 1, d.log_pmtu // BD Slot
    // REM_PYLD_BYTES for UD is set to the actual payload size and the GRH header size to enable one shot dma for UD.
    // Compare only real payload size to PMTU.
    sub         r2, REM_PYLD_BYTES, GRH_HDR_T_SIZE_BYTES
    blt         r1, r2, ud_phv_drop

    // check if q_key matches
    sne    c1, CAPRI_RXDMA_DETH_Q_KEY, d.q_key //BD Slot
    addi   r1, r0, 0x01234567
    sne    c2, CAPRI_RXDMA_DETH_Q_KEY, r1
    bcf    [c1 & c2] , ud_phv_drop
    IS_ANY_FLAG_SET(c6, r7, RESP_RX_FLAG_IMMDT) //BD Slot

    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7)

    // populate completion entry
    phvwrpair   p.cqe.qid, CAPRI_RXDMA_INTRINSIC_QID, p.cqe.type, CQE_TYPE_RECV
    phvwrpair   p.cqe.recv.op_type, OP_TYPE_CQE_RECV_SEND, p.cqe.recv.src_qp[23:0], CAPRI_RXDMA_DETH_SRC_QP
    CAPRI_RXDMA_DETH_SMAC(r5)
    CAPRI_RXDMA_DETH_IMMETH_SMAC1(r1)
    sll.c6      r1, r1, 32
    cmov        r1, c6, r1, r5
    phvwr       p.cqe.recv.smac, r1
    phvwrpair.c6    p.cqe.recv.op_type, OP_TYPE_CQE_RECV_SEND_IMM, p.cqe.recv.imm_data, CAPRI_RXDMA_DETH_IMMETH_DATA  
    

    // populate PD in lkey's to_stage
    CAPRI_SET_FIELD2(TO_S_LKEY_P, pd, d.pd)

    // in case of UD speculation is always enabled as all the 
    // packets are of SEND_ONLY type

/****** Common Logic for UD/RC RQ descriptor checkout ******/
ud_checkout:
rc_checkout:

    IS_ANY_FLAG_SET(c7, r7, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY)
    // c7: last or only
 
    // checkout a descriptor
    add         r1, r0, SPEC_RQ_C_INDEX

    seq         c1, d.rq_in_hbm, 1
    seq         c2, d.skip_pt, 1
    seq         c3, d.prefetch_en, 1
    bcf         [!c1 & !c2 & !c3], pt_process
    
    // flush the last tblwr in this path
    tblmincri.c7.f SPEC_RQ_C_INDEX, d.log_num_wqes, 1 // BD Slot

    sll         r2, r1, d.log_wqe_size
    add.!c2     r2, r2, d.hbm_rq_base_addr, HBM_SQ_BASE_ADDR_SHIFT
    add.c2      r2, r2, d.phy_base_addr, PHY_BASE_ADDR_SHIFT
    or.c2       r2, r2, 1, 63
    or.c2       r2, r2, CAPRI_RXDMA_INTRINSIC_LIF, 52
    
    CAPRI_RESET_TABLE_0_ARG()

    CAPRI_SET_FIELD2(WQE_INFO_P, log_rq_page_size, d.log_rq_page_size)
    phvwrpair   CAPRI_PHV_FIELD(WQE_INFO_P, remaining_payload_bytes), REM_PYLD_BYTES, \
                CAPRI_PHV_FIELD(WQE_INFO_P, curr_wqe_ptr), r2
    phvwrpair   CAPRI_PHV_FIELD(WQE_INFO_P, dma_cmd_index), RESP_RX_DMA_CMD_PYLD_BASE, \
                CAPRI_PHV_FIELD(WQE_INFO_P, log_pmtu), d.log_pmtu

    //MPU only
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_dummy_rqpt_process, r0)

pt_process:
    sub         r2, d.log_rq_page_size, d.log_wqe_size 
    // page_index = c_index >> (log_rq_page_size - log_wqe_size)
    srlv        r3, r1, r2

    // page_offset = c_index & ((1 << (log_rq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr       r1, r2, r0
    sll         r1, r1, d.log_wqe_size

    // r3 has page_index, r1 has page_offset by now

    // page_seg_offset = page_index & 0x7
    and         r5, r3, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    sub         r3, r3, r5
    // page_index = page_index * sizeof(u64)
    // page_index += rqcb_p->pt_base_addr
    sll         r4, d.pt_base_addr, PT_BASE_ADDR_SHIFT
    add         r3, r4, r3, CAPRI_LOG_SIZEOF_U64
    // now r3 has page_p to load

    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD2(INFO_OUT1_P, log_rq_page_size, d.log_rq_page_size)
    phvwrpair   CAPRI_PHV_FIELD(INFO_OUT1_P, page_seg_offset), r5, \
                CAPRI_PHV_FIELD(INFO_OUT1_P, page_offset), r1
    phvwrpair   CAPRI_PHV_FIELD(INFO_OUT1_P, remaining_payload_bytes), REM_PYLD_BYTES, \
                CAPRI_PHV_FIELD(INFO_OUT1_P, log_pmtu), d.log_pmtu

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqpt_process, r3)

ud_phv_drop:
    // we can still set the intrinsic drop bit, though we are going to writeback
    phvwr       p.common.p4_intr_global_drop, 1
    // turn off completion
    and         r7, r7, ~(RESP_RX_FLAG_COMPLETION)
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7)
    CAPRI_RESET_TABLE_2_ARG()
    // we need to set soft_nak_or_dup so that msn is not updated for these cases in writeback.
    phvwrpair   CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1, \
                CAPRI_PHV_FIELD(TO_S_WB1_P, soft_nak_or_dup), 1
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

phv_drop:
    phvwr       p.common.p4_intr_global_drop, 1
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_phv_drop_mpu_only_process, r0)
    
/****** Logic to recirc packets ******/
recirc_work_not_done:
    seq     c6, TOKEN_ID, d.nxt_to_go_token_id
    // clear the recirc iter count and recirc_cnt to 1 
    phvwr.c6    p.common.rdma_recirc_recirc_iter_count, 0
    bcf     [c6], start_recirc_packet
    phvwr.c6    p.common.p4_intr_recirc_count, 1 //BD Slot

    // fall thru to recirc

recirc_wait_for_turn:
    // fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_recirc_mpu_only_process, r0)

    phvwr       p.common.p4_intr_recirc, 1
    phvwr.e     p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE
    tbladd      d.work_not_done_recirc_cnt, 1 // Exit Slot

/****** Logic to handle already recirculated packets ******/
recirc_pkt:
    // turn off recirc, if this thread needs recirc again, respective
    // code would enable recirc flag there.
    phvwr       p.common.p4_intr_recirc, 0

    // if the p4 intrinsic recirc_count has not hit the limit, proceed with pkt handling
    seq             c6, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, P4_INTR_RECIRC_COUNT_MAX
    bcf             [!c6], skip_recirc_cnt_max_check

    // check if phv's recirc iter count has hit the limit
    add             r6, 1, CAPRI_APP_DATA_RECIRC_ITER_COUNT //BD Slot
    
    // did we reach recirc_iter_count to 15 ? 
    seq             c6, r6, RDMA_RECIRC_ITER_COUNT_MAX
    bcf             [c6], max_recirc_cnt_err
    phvwr.!c6       p.common.p4_intr_recirc_count, 1  //BD Slot
    phvwr           p.common.rdma_recirc_recirc_iter_count, r6
                
    
skip_recirc_cnt_max_check:
    seq     c2, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_SGE_WORK_PENDING
    bcf     [c2], recirc_sge_work_pending
    seq     c3, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE // BD Slot
    bcf     [c3], recirc_work_not_done
    tblsub.c3 d.work_not_done_recirc_cnt, 1 // BD Slot
    seq     c4, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_ATOMIC_RNR
    bcf     [c4], recirc_atomic_rnr
    seq     c5, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_INORDER_WORK_DONE // BD Slot
    bcf     [c5], recirc_work_done
    seq     c5, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_ERROR_DISABLE_QP // BD Slot
    bcf     [c5], recirc_error_disable_qp
    nop     //BD Slot

    // For any any known or non-handled recirc reasons, drop the packet
    phvwr.e     p.common.p4_intr_global_drop, 1
    nop

recirc_sge_work_pending:
    // when sge work is pending, there is no need to wait for 
    // token_id to be current, keep proceeding with further 
    // processing. In this case, stage1 is already setup with
    // required information to process further sges. If middle
    // packet is recircing, it may need some info from rqcb1 as
    // well (such as curr_wqe_ptr, num_sges etc.)
    // Hence pass these RQCB1 variables to recirc process code. 
    
    //invoke an MPU only program to continue the activity
    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD_RANGE2(RQCB_TO_RQCB1_P, curr_wqe_ptr, num_sges, d.{curr_wqe_ptr...num_sges})
    CAPRI_SET_FIELD2(RQCB_TO_RQCB1_P, log_rq_page_size, d.log_rq_page_size)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_recirc_sge_process, r0)

    nop.e
    nop

/****** Exit path ******/
exit:
    nop.e
    nop

process_feedback:
    // in future, if different kind of feedback phvs are handled, we may have to 
    // set error_disable_qp only for those feedback phvs which are of type error.
    // can't combine both the phvwr's as they span beyond 512b.
    phvwr       CAPRI_PHV_RANGE(phv_global_common, _ud, _error_disable_qp), \
                (RESP_RX_FLAG_ERR_DIS_QP | RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_ACK_REQ)
    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, feedback), 1
    phvwr       CAPRI_PHV_FIELD(TO_S_CQCB_P, feedback), 1

    // when resp_rx generates the flush wqe, it is communicated to resp_tx side using a vendor
    // specific nak code. This NAK code is only used for internal purpose. i.e., resp_tx
    // wouldn't generate any NAK frame on wire with this error code. It is simply an indication
    // to the resp_tx that it should move its state to ERR. 
    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_FLUSH_RQ)
    RXDMA_DMA_CMD_PTR_SET(RESP_RX_DMA_CMD_START_FLIT_ID, 0) //BD Slot
    phvwrpair   p.cqe.status, CQ_STATUS_WQE_FLUSHED_ERR, p.cqe.error, 1
    phvwrpair   p.cqe.qid, CAPRI_RXDMA_INTRINSIC_QID, p.cqe.type, CQE_TYPE_RECV //BD Slot
    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_CQE_FLUSH_ERR_OFFSET))
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_feedback), 1
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

table_error:
    // TODO add LIF stats
    phvwr          p.common.p4_intr_global_drop, 1
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_phv_drop_mpu_only_process, r0) // Exit Slot

max_recirc_cnt_err:
    //a packet which went thru too many recirculations had to be terminated and qp had to 
    //be put into error disabled state. The recirc reason, opcode, the psn of the packet etc.
    //are remembered for further debugging.
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, max_recirc_cnt_err), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, recirc_reason), CAPRI_APP_DATA_RECIRC_REASON

    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, last_bth_opcode), CAPRI_APP_DATA_BTH_OPCODE, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, recirc_bth_psn), CAPRI_APP_DATA_BTH_PSN

    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_err_process, r0)
