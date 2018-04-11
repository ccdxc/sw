#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct rqcb1_t d;
//struct resp_rx_rqcb_process_k_t k;
struct common_p4plus_stage0_app_header_table_k k;

#define RQCB_TO_WRITE_P t1_s2s_rqcb_to_write_rkey_info
#define INFO_OUT1_P t0_s2s_rqcb_to_pt_info
//#define RSQ_BT_S2S_INFO_T struct resp_rx_rsq_backtrack_info_t 
//#define RSQ_BT_TO_S_INFO_T struct resp_rx_to_stage_backtrack_info_t
#define RQCB_TO_RQCB1_P t0_s2s_rqcb_to_rqcb1_info
#define TO_S_WB1_P to_s3_wb1_info
//#define TO_S_STATS_INFO_T struct resp_rx_to_stage_stats_info_t
#define RQCB_TO_RD_ATOMIC_P t1_s2s_rqcb_to_read_atomic_rkey_info
#define TO_S_ATOMIC_INFO_P to_s1_atomic_info
#define WQE_INFO_P t0_s2s_rqcb_to_wqe_info

#define REM_PYLD_BYTES  r6
#define RSQWQE_P r2
#define DMA_CMD_BASE r1
#define TOKEN_ID r1
#define TMP r3
#define IMM_DATA r2
#define DB_ADDR r4
#define DB_DATA r5
#define NEW_RSQ_P_INDEX r6
#define RQCB2_ADDR r6

%%
    .param    resp_rx_rqpt_process
    .param    resp_rx_dummy_rqpt_process
    .param    resp_rx_rqcb1_in_progress_process
    .param    resp_rx_write_dummy_process
    .param    resp_rx_rsq_backtrack_process
    .param    resp_rx_rqcb1_recirc_sge_process
    .param    resp_rx_dcqcn_ecn_process
    .param    resp_rx_dcqcn_cnp_process
    .param    rdma_atomic_resource_addr
    .param    resp_rx_read_mpu_only_process
    .param    resp_rx_atomic_resource_process
    .param    resp_rx_recirc_mpu_only_process

.align
resp_rx_rqcb_process:
    // table 0 valid bit would be set by the time we get into S0 because
    // to get +64 logic, p4 program would have set table 0 valid bit to TRUE.
    // we need to this bit back to 0 right way otherwise table 0 gets fired
    // unnecessarily in further stages and cause wrong behavior (mainly for write/read/atomic)
    CAPRI_SET_TABLE_0_VALID(0)
    add     r7, r0, CAPRI_APP_DATA_RAW_FLAGS

    // is this a fresh packet ?
    seq     c1, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    bcf     [!c1], recirc_pkt
    nop     //BD Slot

    //fresh packet
    // populate global fields

#  // moved to _ext program
#  CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(RQCB_ADDR_SHIFT))
#  CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, lif, CAPRI_RXDMA_INTRINSIC_LIF)

#   //Temporary code to test UDP options
#   //For now, checking on ts flag for both options ts and mss to avoid performance cost
#  bbeq     CAPRI_APP_DATA_ROCE_OPT_TS_VALID, 0, skip_roce_opt_parsing
#  CAPRI_SET_FIELD_RANGE(r3, PHV_GLOBAL_COMMON_T, qid, qtype, CAPRI_RXDMA_INTRINSIC_QID_QTYPE) //BD Slot
#  //get rqcb3 address
#  add      r6, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 3)
#  add      r5, r6, FIELD_OFFSET(rqcb3_t, roce_opt_ts_value)
#  memwr.d  r5, CAPRI_APP_DATA_ROCE_OPT_TS_VALUE_AND_ECHO
#  add      r5, r6, FIELD_OFFSET(rqcb3_t, roce_opt_mss)
#  memwr.h  r5, CAPRI_APP_DATA_ROCE_OPT_MSS

skip_roce_opt_parsing:

    // get a tokenid for the fresh packet
    phvwr  p.common.rdma_recirc_token_id, d.token_id

    CAPRI_SET_FIELD2(TO_S_WB1_P, my_token_id, d.token_id)

#   CAPRI_GET_STAGE_7_ARG(resp_rx_phv_t, r4)
#   CAPRI_SET_FIELD(r4, TO_S_STATS_INFO_T, bytes, CAPRI_APP_DATA_PAYLOAD_LEN)

start_recirc_packet:
    # subtract the pad bytes from the payload length.
    sub     REM_PYLD_BYTES, CAPRI_APP_DATA_PAYLOAD_LEN, CAPRI_APP_DATA_BTH_PAD
    crestore [c7, c6, c5, c4, c3, c2, c1], r7, (RESP_RX_FLAG_UD | RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_READ_REQ | RESP_RX_FLAG_SEND | RESP_RX_FLAG_ONLY)
    // c7: UD, c6: cswap, c5: fna, c4: write, c3: read, c2: send, c1: only

    bcf     [c7], process_ud
    //set DMA cmd ptr
    RXDMA_DMA_CMD_PTR_SET(RESP_RX_DMA_CMD_START_FLIT_ID) //BD Slot

    //Check if ECN bits are set in Packet and congestion management is enabled.                      
    bbne     d.congestion_mgmt_enable, 1, skip_cnp_receive
    sne      c2, k.rdma_bth_ecn, 3  //c2 is not used after assignment above. Re-using it.
    bcf      [c2], skip_cnp_send

    //Process sending CNP packet to the requester.
    CAPRI_RESET_TABLE_3_ARG() 
    add     r5, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT //dcqcn_cb addr
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_dcqcn_ecn_process, r5) 

skip_cnp_send:
    // Check if its CNP packet.
    sne     c2, CAPRI_APP_DATA_BTH_OPCODE, RDMA_PKT_OPC_CNP
    bcf     [c7 | c2], skip_cnp_receive

    add     r5, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT //dcqcn_cb addr // BD Slot
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_dcqcn_cnp_process, r5) 
    nop.e
    nop

skip_cnp_receive:
    // TODO: Migrate ACK_REQ flag to P4 table
    seq     c7, CAPRI_APP_DATA_BTH_ACK_REQ, 1
    or.c7   r7, r7, RESP_RX_FLAG_ACK_REQ

    bcf     [c1 | c6 | c5 | c3], process_only_rd_atomic
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7) //BD Slot

/****** Slow path: SEND/WRITE FIRST/MIDDLE/LAST ******/
process_send_write_fml:
    crestore    [c6, c5, c4, c3, c2, c1], r7, (RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_SEND | RESP_RX_FLAG_LAST | RESP_RX_FLAG_MIDDLE | RESP_RX_FLAG_FIRST) 
    //  c6: immdt, c5: write, c4: send, c3: last, c2: middle, c1: first
    //  using c5 in below code, make sure its not used
    
    // if it is a first packet, disable further speculation
    tblwr.c1    d.disable_speculation, 1

    // check if it is my turn. if not, recirc.
    seq         c5, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    cmov        TOKEN_ID, c5, d.token_id, CAPRI_APP_DATA_RECIRC_TOKEN_ID
    //  using c5 derived in above crestore, make sure its not used
    seq         c7, TOKEN_ID, d.nxt_to_go_token_id

    bcf         [!c7], recirc_wait_for_turn
    tbladd.c5   d.token_id, 1  //BD Slot

    //got my turn, do sanity checks

    // if it is the last packet, enable speculation again
    tblwr.c3    d.disable_speculation, 0

    // check serv_type
    seq         c7, CAPRI_APP_DATA_BTH_OPCODE[7:5], d.serv_type
    bcf         [!c7], inv_req_nak

    // is pkt psn same as e_psn ?
    seq         c7, d.e_psn, CAPRI_APP_DATA_BTH_PSN //BD Slot
    bcf         [!c7], seq_err_or_duplicate

    sll         r1, 1, d.log_pmtu  //BD Slot

    // packet_length should not be more than PMTU
    blt         r1, REM_PYLD_BYTES, inv_req_nak
    nop

    // last packet should be at least 1 byte
    beq.c3      REM_PYLD_BYTES, r0, inv_req_nak     
    nop

    // first/middle packets should be of pmtu size
    sne         c7, r1, REM_PYLD_BYTES
    bcf.c7      [c1|c2], inv_req_nak
    
    // increment msn if it is a last packet
    add         r1, r0, d.msn   //BD Slot
    mincr.c3    r1, 24, 1
    tblwr.c3    d.msn, r1

    // populate ack info
    phvwrpair   p.ack_info.psn, d.e_psn, p.ack_info.aeth.msn, r1
    RQ_CREDITS_GET(r1, r2, c7)
    AETH_ACK_SYNDROME_GET(r2, r1)
    phvwr       p.ack_info.aeth.syndrome, r2

    // increment e_psn
    tblmincri   d.e_psn, 24, 1

    bcf         [c4], process_send
    phvwr       p.cqwqe.qp, CAPRI_RXDMA_INTRINSIC_QID //BD Slot
     
/****** Slow path: WRITE FIRST/MIDDLE/LAST ******/
process_write:
    // load rqcb3
    add     r5, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 3)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_write_dummy_process, r5)

    // only first packet has reth header
    CAPRI_RESET_TABLE_1_ARG()

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
    phvwrpair   p.cqwqe.op_type, OP_TYPE_RDMA_OPER_WITH_IMM, p.cqwqe.imm_data_vld, 1
    b           rc_checkout
    phvwr       p.cqwqe.imm_data, IMM_DATA //BD Slot

/****** Slow path: SEND FIRST/MIDDLE/LAST ******/
process_send:
    // if SEND_FIRST, we simply need to checkout a descriptor
    bcf         [c1], rc_checkout
    crestore [c7, c6, c5], r7, (RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_IMMDT)   //BD Slot
    // c7: completion, c6: inv_rkey, c5: immdt

    setcf       c4, [c6 | c5]

    // if SEND_MIDDLE OR immediate/inv_rkey is not present, 
    // we simply need to go to in_progress path
    bcf         [c2 | !c4], send_in_progress
    phvwr.c7    p.cqwqe.op_type, OP_TYPE_SEND_RCVD  //BD Slot

    bcf         [!c6], send_check_immdt
    nop         //BD Slot

    phvwrpair   p.cqwqe.rkey_inv_vld, 1, p.cqwqe.r_key, CAPRI_RXDMA_BTH_IETH_R_KEY
    CAPRI_SET_FIELD2(TO_S_WB1_P, inv_r_key, CAPRI_RXDMA_BTH_IETH_R_KEY)

send_check_immdt:
    bcf         [!c5], send_in_progress
    seq         c7, d.immdt_as_dbell, 1     //BD Slot
    bcf         [!c7], send_skip_immdt_as_dbell
    phvwr.!c7   p.cqwqe.imm_data_vld, 1     //BD Slot

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
    phvwr       p.cqwqe.imm_data, CAPRI_RXDMA_BTH_IMMETH_IMMDATA //BD Slot

send_in_progress:
    // load rqcb3 to get wrid
    add     r5, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 3)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqcb1_in_progress_process, r5)

    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD_RANGE2(RQCB_TO_RQCB1_P, curr_wqe_ptr, num_sges, d.{curr_wqe_ptr...num_sges})
    b           exit
    phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_RQCB1_P, in_progress), d.in_progress, \
                CAPRI_PHV_FIELD(RQCB_TO_RQCB1_P, remaining_payload_bytes), REM_PYLD_BYTES //BD Slot


/******  Common logic for ONLY packets (send/write) ******/
process_only_rd_atomic:
    // for read and atomic, start DMA commands from flit 9 instead of 8
    RXDMA_DMA_CMD_PTR_SET_C(RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, !c1) //BD Slot
    seq         c7, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    bbeq        d.disable_speculation, 0, skip_token_id_check
    tbladd.c7   d.token_id, 1  //BD Slot
    cmov        TOKEN_ID, c7, d.token_id, CAPRI_APP_DATA_RECIRC_TOKEN_ID
    seq         c1, TOKEN_ID, d.nxt_to_go_token_id
    bcf         [!c1], recirc_wait_for_turn
skip_token_id_check:

    seq         c7, CAPRI_APP_DATA_BTH_OPCODE[7:5], d.serv_type //BD Slot in straight path
    bcf         [!c7], inv_req_nak
    // is pkt psn same as e_psn ?
    seq         c7, d.e_psn, CAPRI_APP_DATA_BTH_PSN //BD Slot
    bcf         [!c7], seq_err_or_duplicate

    // check if payload_len is <= pmtu
    sll         r1, 1, d.log_pmtu  //BD Slot 
    blt         r1, REM_PYLD_BYTES, inv_req_nak

    // increment msn
    add         r1, r0, d.msn   //BD Slot
    mincr       r1, 24, 1
    tblwr       d.msn, r1

    // populate ack info
    phvwrpair   p.ack_info.psn, d.e_psn, p.ack_info.aeth.msn, r1
    RQ_CREDITS_GET(r1, r2, c1)
    AETH_ACK_SYNDROME_GET(r2, r1)
    phvwr       p.ack_info.aeth.syndrome, r2

    bcf     [c6 | c5 | c3], process_read_atomic
    phvwr       p.ack_info.aeth.syndrome, r2    //BD Slot

    // increment e_psn
    tblmincri   d.e_psn, 24, 1

    // populate completion entry
    bcf         [c4], process_write_only
    phvwr       p.cqwqe.qp, CAPRI_RXDMA_INTRINSIC_QID //BD Slot

/******  Logic for SEND_ONLY packets ******/
process_send_only:
    
    // handle immdiate data and inv_r_key
    crestore    [c7, c6], r7, (RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_IMMDT)
    // c7: inv_rkey, c6: immdt
    bcf         [!c7 & !c6], rc_checkout
    phvwr       p.cqwqe.op_type, OP_TYPE_SEND_RCVD //BD Slot

    bcf         [!c7], send_only_check_immdt
    CAPRI_GET_STAGE_3_ARG(resp_rx_phv_t, r4) //BD Slot

    phvwrpair   p.cqwqe.rkey_inv_vld, 1, p.cqwqe.r_key, CAPRI_RXDMA_BTH_IETH_R_KEY

    CAPRI_SET_FIELD2(TO_S_WB1_P, inv_r_key, CAPRI_RXDMA_BTH_IETH_R_KEY)

send_only_check_immdt:
    bcf         [!c6], rc_checkout
    seq         c7, d.immdt_as_dbell, 1     //BD Slot
    bcf         [!c7], send_only_skip_immdt_as_dbell
    phvwr.!c7   p.cqwqe.imm_data_vld, 1     //BD Slot

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
    phvwr       p.cqwqe.imm_data, CAPRI_RXDMA_BTH_IMMETH_IMMDATA //BD Slot


/******  Logic for WRITE_ONLY packets ******/
process_write_only:
    crestore    [c6], r7, (RESP_RX_FLAG_IMMDT)
    
    // load rqcb3
    add     r5, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 3)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_write_dummy_process, r5)

    CAPRI_RESET_TABLE_1_ARG()
    CAPRI_SET_FIELD_RANGE2(RQCB_TO_WRITE_P, va, len, CAPRI_RXDMA_RETH_VA_R_KEY_LEN)

    bcf             [!c6], exit
    CAPRI_SET_FIELD2(RQCB_TO_WRITE_P, remaining_payload_bytes, REM_PYLD_BYTES) //BD Slot

    seq         c7, d.immdt_as_dbell, 1
    bcf         [!c7], wr_only_skip_immdt_as_dbell
    CAPRI_RXDMA_BTH_RETH_IMMETH_IMMDATA_C(IMM_DATA, c0)

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
    
wr_only_skip_immdt_as_dbell:
    CAPRI_SET_FIELD2(RQCB_TO_WRITE_P, incr_c_index, 1)
    phvwrpair   p.cqwqe.op_type, OP_TYPE_RDMA_OPER_WITH_IMM, p.cqwqe.imm_data_vld, 1
    b           rc_checkout
    phvwr       p.cqwqe.imm_data, IMM_DATA //BD Slot

/****** Logic for READ/ATOMIC packets ******/
process_read_atomic:
    // wqe_p = (void *)(hbm_addr_get(rqcb_p->rsq_base_addr) +    
    //                      (sizeof(rsqwqe_t) * p_index));
    seq         c1, d.rsq_quiesce, 1
    // if rsq_quiesce is on, use rsq_p_index_prime, else use rsq_p_index
    cmov        NEW_RSQ_P_INDEX, c1, d.rsq_pindex_prime, d.rsq_pindex
    sll         RSQWQE_P, d.rsq_base_addr, RSQ_BASE_ADDR_SHIFT
    add         RSQWQE_P, RSQWQE_P, NEW_RSQ_P_INDEX, LOG_SIZEOF_RSQWQE_T
    // p_index/c_index are in little endian
    mincr       NEW_RSQ_P_INDEX, d.log_rsq_size, 1
   
    // DMA for RSQWQE
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RSQWQE)
    
    // in case of quiesce mode, only duplicate reqs would move rsq_p_index in a 
    // lock step manner. If we receive non-duplicate, we increment only
    // rsq_p_index_prime. Standard is not clear whether we can handle fresh
    // requests while duplicate requests are being handled. If needed, we can
    // stop dropping new requests if quiesce mode is on.
    // 
    tblwr.c1    d.rsq_pindex_prime, NEW_RSQ_P_INDEX
    tblwr.!c1   d.rsq_pindex, NEW_RSQ_P_INDEX

    // common params for both read/atomic
    CAPRI_RESET_TABLE_1_ARG()
    phvwrpair   p.rsqwqe.read.r_key, CAPRI_RXDMA_RETH_R_KEY, p.rsqwqe.read.va, CAPRI_RXDMA_RETH_VA
    CAPRI_SET_FIELD_RANGE2(RQCB_TO_RD_ATOMIC_P, va, r_key, CAPRI_RXDMA_RETH_VA_R_KEY)
    phvwrpair   CAPRI_PHV_FIELD(RQCB_TO_RD_ATOMIC_P, rsq_p_index), NEW_RSQ_P_INDEX, \
                CAPRI_PHV_FIELD(RQCB_TO_RD_ATOMIC_P, skip_rsq_dbell), d.rsq_quiesce
    

    bcf         [c6 | c5], process_atomic
    phvwr       p.rsqwqe.psn, d.e_psn   //BD Slot

process_read:
    // for read, set the end of commands right here if quiesce is on.
    // for atomic, we have to execute the atomic request and hence
    // end of commands is set in atomic_resource_process function.
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, rsqwqe, rsqwqe, RSQWQE_P)
    DMA_SET_END_OF_CMDS_C(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE, c1)
    phvwr       p.rsqwqe.read.len, CAPRI_RXDMA_RETH_DMA_LEN
    CAPRI_SET_FIELD2(RQCB_TO_RD_ATOMIC_P, len, CAPRI_RXDMA_RETH_DMA_LEN)
    // do a MPU-only lookup
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_0_BITS, resp_rx_read_mpu_only_process, r0)
    
    //increment e_psn by 'n'
    // e_psn += read_len >> log_pmtu
    srl            r3, CAPRI_RXDMA_RETH_DMA_LEN, d.log_pmtu
    tblmincr       d.e_psn, 24, r3

    // e_psn += (read_len & ((1 << log_pmtu) -1)) ? 1 : 0
    add            r3, CAPRI_RXDMA_RETH_DMA_LEN, r0
    mincr          r3, d.log_pmtu, r0
    sle.e          c6, r3, r0
    tblmincri.!c6  d.e_psn, 24, 1       //Exit Slot

process_atomic:
    CAPRI_SET_FIELD2(RQCB_TO_RD_ATOMIC_P, read_or_atomic, RSQ_OP_TYPE_ATOMIC)
    CAPRI_SET_FIELD2(TO_S_ATOMIC_INFO_P, rsqwqe_ptr, RSQWQE_P)

    // do a MPU-only lookup
    addui           r3, r0, hiword(rdma_atomic_resource_addr)
    addi            r3, r3, loword(rdma_atomic_resource_addr)            
    // load 32 Bytes (256-bits) of atomic resource
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_256_BITS, resp_rx_atomic_resource_process, r3)
    
    phvwr           p.rsqwqe.read_or_atomic, RSQ_OP_TYPE_ATOMIC

    bcf             [c6], process_cswap
    // increment e_psn
    tblmincri   d.e_psn, 24, 1      //BD Slot

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
    nop         //BD Slot

duplicate_wr_send:
    // ring the ack_nak ring one more time so that a new ack is pushed out
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK)

    RESP_RX_RING_ACK_NAK_DB(DMA_CMD_BASE, \
                            CAPRI_RXDMA_INTRINSIC_LIF, \
                            CAPRI_RXDMA_INTRINSIC_QTYPE, \
                            CAPRI_RXDMA_INTRINSIC_QID, \
                            DB_ADDR, DB_DATA)

    //Generate DMA command to skip to payload end
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD_ON_ERROR)

    // release chance to next packet
    tbladd.e  d.nxt_to_go_token_id, 1  
    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 1 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/) //Exit Slot

duplicate_rd_atomic:
    // TBD: Disabling duplicate read/atomic handling for now.
    // need to change to new logic
        
//    // recirc if threre is already another duplicate req in progress
//    seq             c1, d.adjust_rsq_c_index_in_progress, 1
//    bcf             [c1], recirc_wait_for_turn
//    nop             //BD Slot
//
//    ARE_ALL_FLAGS_SET(c2, r7, RESP_RX_FLAG_READ_REQ)
//    // RETH and ATOMICETH have VA, r_key at same location 
//    cmov            r6, c2, CAPRI_RXDMA_RETH_DMA_LEN, 8
//
//    // since there is no space in stage-to-stage data, using to-stage
//    // to populate va/r_key/len so that it is accessible to backtrack_process
//    // function which may get called multiple times in successive stages
//    // during backtrack process
//    CAPRI_GET_STAGE_1_ARG(resp_rx_phv_t, r4)
//    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, va, CAPRI_RXDMA_RETH_VA)
//    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, r_key, CAPRI_RXDMA_RETH_R_KEY)
//    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, len, r6)
//
//    seq         c3, d.rsq_quiesce, 1
//    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)
//    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, search_psn, CAPRI_APP_DATA_BTH_PSN)
//    CAPRI_SET_FIELD_C(r4, RSQ_BT_S2S_INFO_T, lo_index, d.rsq_pindex_prime, c3)
//    CAPRI_SET_FIELD_C(r4, RSQ_BT_S2S_INFO_T, lo_index, RSQ_P_INDEX, !c3)
//    add         r5, r0, RSQ_P_INDEX
//    mincr       r5, d.log_rsq_size, -1
//    // in quiesce mode, make sure hi_index is 1 more than RSQ_P_INDEX.
//    // since previous mincr has decremented by 1, we are incrementing by 2 here.
//    mincr.c3    r5, d.log_rsq_size, +2
//    seq         c4, RSQ_P_INDEX, RSQ_C_INDEX
//    add         r6, r0, RSQ_C_INDEX
//    // if rsq is empty, we need to start with c_index-1
//    mincr.c4    r6, d.log_rsq_size, -1
//
//    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, hi_index, r5)
//    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, index, r6)
//    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, rsq_base_addr, d.rsq_base_addr)
//    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, log_pmtu, d.log_pmtu)
//    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, walk, RSQ_EVAL_MIDDLE)
//    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, log_rsq_size, d.log_rsq_size)
//    
//    cmov        r5, c2, RSQ_OP_TYPE_READ, RSQ_OP_TYPE_ATOMIC 
//    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, read_or_atomic, r5)
//    
//    //load entry at cindex first
//    sll         r3, d.rsq_base_addr, RSQ_BASE_ADDR_SHIFT
//    add         r3, r3, r6, LOG_SIZEOF_RSQWQE_T
//    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rsq_backtrack_process, r3)

    nop.e
    nop
    

/****** Logic for NAKs ******/

inv_req_nak:
    b           nak
    phvwr       p.ack_info.aeth.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_INV_REQ)    //BD Slot
   
seq_err_nak:
    b           nak
    phvwr       p.ack_info.aeth.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_SEQ_ERR)    //BD Slot

nak: 
    phvwrpair   p.ack_info.psn, d.e_psn, p.ack_info.aeth.msn, d.msn
      
    add         RQCB2_ADDR, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 2)
    IS_ANY_FLAG_SET(c2, r7, RESP_RX_FLAG_READ_REQ|RESP_RX_FLAG_ATOMIC_FNA|RESP_RX_FLAG_ATOMIC_CSWAP)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, !c2)
    DMA_CMD_STATIC_BASE_GET_C(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_ACK, c2)

    RESP_RX_POST_ACK_INFO_TO_TXDMA(DMA_CMD_BASE, RQCB2_ADDR, TMP, \
                                   CAPRI_RXDMA_INTRINSIC_LIF, \
                                   CAPRI_RXDMA_INTRINSIC_QTYPE, \
                                   CAPRI_RXDMA_INTRINSIC_QID, \
                                   DB_ADDR, DB_DATA)

    //Generate DMA command to skip to payload end
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD_ON_ERROR)

    // release chance to next packet
    tbladd.e  d.nxt_to_go_token_id, 1  
    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 1 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/) //Exit Slot

/****** Logic for UD ******/

process_ud:
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, r7)

    seq         c7, CAPRI_APP_DATA_BTH_OPCODE[7:5], d.serv_type
    seq         c3, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    // if it doesn't match serv_type OR not send OR not only, drop
    bcf         [!c7 | !c2 | !c1], ud_drop
    tbladd.c3   d.token_id, 1 // BD Slot

    // check if payload_len is <= pmtu
    sll         r1, 1, d.log_pmtu
    blt         r1, REM_PYLD_BYTES, ud_drop

    // check if q_key matches
    sne    c1, CAPRI_RXDMA_DETH_Q_KEY, d.q_key //BD Slot
    addi   r1, r0, 0x01234567
    sne    c2, CAPRI_RXDMA_DETH_Q_KEY, r1
    bcf    [c1 & c2] , ud_drop
    IS_ANY_FLAG_SET(c6, r7, RESP_RX_FLAG_IMMDT) //BD Slot

    // populate completion entry
    phvwrpair   p.cqwqe.op_type, OP_TYPE_SEND_RCVD, p.cqwqe.qp, CAPRI_RXDMA_INTRINSIC_QID
    phvwrpair   p.cqwqe.src_qp, CAPRI_RXDMA_DETH_SRC_QP, p.cqwqe.ipv4, 1
    CAPRI_RXDMA_DETH_SMAC(r5)
    CAPRI_RXDMA_DETH_IMMETH_SMAC1(r1)
    cmov        r1, c6, r1, r5
    phvwr           p.cqwqe.smac, r1
    phvwrpair.c6    p.cqwqe.imm_data_vld, 1, p.cqwqe.imm_data, CAPRI_RXDMA_DETH_IMMETH_DATA
    

    // in case of UD speculation is always enabled as all the 
    // packets are of SEND_ONLY type

/****** Common Logic for UD/RC RQ descriptor checkout ******/
ud_checkout:
rc_checkout:

    // checkout a descriptor
    add         r1, r0, SPEC_RQ_C_INDEX

    bbne        d.rq_in_hbm, 1, pt_process
    tblmincri   SPEC_RQ_C_INDEX, d.log_num_wqes, 1 //BD Slot

    sll         r2, r1, d.log_wqe_size
    add         r2, r2, d.hbm_rq_base_addr, HBM_SQ_BASE_ADDR_SHIFT

    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair   CAPRI_PHV_FIELD(WQE_INFO_P, remaining_payload_bytes), REM_PYLD_BYTES, \
                CAPRI_PHV_FIELD(WQE_INFO_P, curr_wqe_ptr), r2
    CAPRI_SET_FIELD2(WQE_INFO_P, dma_cmd_index, RESP_RX_DMA_CMD_PYLD_BASE)

    //MPU only
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_dummy_rqpt_process, r0)

    //get the address of the next WQE and invoke prefetch
    //ignore the boundary check
    sll         r1, 1, d.log_wqe_size
    add.e       r2, r2, r1
    cpref       r2

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
    phvwrpair   CAPRI_PHV_FIELD(INFO_OUT1_P, page_seg_offset), r5, \
                CAPRI_PHV_FIELD(INFO_OUT1_P, page_offset), r1
    CAPRI_SET_FIELD2(INFO_OUT1_P, remaining_payload_bytes, REM_PYLD_BYTES)

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqpt_process, r3)
    nop.e
    nop

ud_drop:
    phvwr.e     p.common.p4_intr_global_drop, 1
    nop
    
/****** Logic to recirc packets ******/
recirc_wait_for_turn:
    // fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_recirc_mpu_only_process, r0)

    phvwr       p.common.p4_intr_recirc, 1
    phvwr.e     p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE
    nop

/****** Logic to handle already recirculated packets ******/
recirc_pkt:
    // turn off recirc, if this thread needs recirc again, respective
    // code would enable recirc flag there.
    phvwr   p.common.p4_intr_recirc, 0

    seq     c2, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_SGE_WORK_PENDING
    bcf     [c2], recirc_sge_work_pending
    seq     c3, CAPRI_APP_DATA_RECIRC_REASON, CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE // BD Slot
    bcf     [c3], start_recirc_packet
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
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_recirc_sge_process, r0)

    nop.e
    nop

/****** Exit path ******/
exit:
    nop.e
    nop
