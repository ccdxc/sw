#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqwqe_t d;
struct req_tx_s2_t0_k k;

#define WQE_TO_SGE_P t0_s3s_wqe_to_sge_info
#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info
#define SQCB_WRITE_BACK_RD_P t2_s2s_sqcb_write_back_info_rd
#define SQCB_WRITE_BACK_SEND_WR_P t2_s2s_sqcb_write_back_info_send_wr

#define IN_P t0_s2s_sqcb_to_wqe_info
#define IN_TO_S_P to_s2_sq_to_stage

#define TO_S4_P to_s4_sq_to_stage

#define K_LOG_PMTU CAPRI_KEY_FIELD(IN_P, log_pmtu)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit0, remaining_payload_bytes_sbit9_ebit15)
#define K_HEADER_TEMPLATE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, header_template_addr_sbit0_ebit7, header_template_addr_sbit24_ebit31)

%%
    .param    req_tx_sqsge_process
    .param    req_tx_dcqcn_enforce_process

.align
req_tx_sqwqe_process:

    bbne           CAPRI_KEY_FIELD(IN_P, poll_in_progress), 1, skip_color_check

    //color check
    seq            c1, CAPRI_KEY_FIELD(IN_P, color), d.base.color //BD Slot
    bcf            [!c1], clear_poll_in_progress

skip_color_check:

    add            r1, r0, d.base.op_type

    .brbegin
    br             r1[2:0]
    nop            // Branch Delay Slot

    .brcase OP_TYPE_SEND
        b               send_or_write
        nop             //Branch Delay slot

    .brcase OP_TYPE_SEND_INV
        b               send_or_write
        nop             //Branch Delay slot
        
    .brcase OP_TYPE_SEND_IMM
        b               send_or_write
        nop             //Branch Delay slot

    .brcase OP_TYPE_READ
        b               read
        nop             //Branch Delay slot

    .brcase OP_TYPE_WRITE
        b               set_write_reth
        nop             //Branch Delay slot

    .brcase OP_TYPE_WRITE_IMM
        b               set_write_reth
        nop             //Branch Delay slot

    .brcase OP_TYPE_CMP_N_SWAP
        b               atomic
        nop             //Branch Delay slot

    .brcase OP_TYPE_FETCH_N_ADD
        b               atomic
        nop             //Branch Delay slot 

    .brend

set_write_reth:
    phvwrpair RETH_VA, d.write.va, RETH_RKEY, d.write.r_key
    phvwr RETH_LEN, d.write.length

send_or_write:
    // If UD, add DETH hdr
    seq            c1, CAPRI_KEY_FIELD(phv_global_common, ud_service), 1
    bcf            [!c1], set_sge_arg
    seq            c2, d.base.inline_data_vld, 1 // Branch Delay Slot

    phvwrpair DETH_Q_KEY, d.ud_send.q_key, DETH_SRC_QP, K_GLOBAL_QID
    phvwr BTH_DST_QP, d.ud_send.dst_qp

    // setup cqwqe for UD completion
    phvwrpair      p.rdma_feedback.feedback_type, RDMA_UD_FEEDBACK, p.rdma_feedback.ud.wrid, d.base.wrid
    phvwrpair      p.rdma_feedback.ud.optype[3:0], d.base.op_type, p.rdma_feedback.ud.status, 0

    // For UD, length should be less than pmtu
    sll            r4, 1,  K_LOG_PMTU
    add            r5, r0, d.ud_send.length
    blt            r4, r5, ud_error
    add            r2, d.ud_send.ah_handle, r0 // Branch Delay Slot

set_sge_arg:
    bcf            [c2], inline_data
    // sge_list_addr = wqe_addr + TX_SGE_OFFSET
    mfspr          r3, spr_tbladdr //Branch Delay Slot
    add            r3, r3, TXWQE_SGE_OFFSET

    // populate stage-2-stage data req_tx_wqe_to_sge_info_t for next stage
    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD2(WQE_TO_SGE_P, in_progress, CAPRI_KEY_FIELD(IN_P, in_progress))
    CAPRI_SET_FIELD2(WQE_TO_SGE_P, first, 1)
    CAPRI_SET_FIELD2(WQE_TO_SGE_P, num_valid_sges, d.base.num_sges)
    CAPRI_SET_FIELD2(WQE_TO_SGE_P, remaining_payload_bytes, K_REMAINING_PAYLOAD_BYTES)
    CAPRI_SET_FIELD2(WQE_TO_SGE_P, dma_cmd_start_index, REQ_TX_DMA_CMD_PYLD_BASE)
    CAPRI_SET_FIELD2(WQE_TO_SGE_P, op_type, d.base.op_type)
    CAPRI_SET_FIELD_RANGE2(WQE_TO_SGE_P, imm_data, inv_key_or_ah_handle, d.{send.imm_data...send.inv_key})
    // if UD copy ah_handle
    CAPRI_SET_FIELD2_C(WQE_TO_SGE_P, ah_size, d.ud_send.ah_size, c1)
    CAPRI_SET_FIELD2_C(WQE_TO_SGE_P, inv_key_or_ah_handle, d.ud_send.ah_handle, c1)
    CAPRI_SET_FIELD_RANGE2(WQE_TO_SGE_P, poll_in_progress, color, CAPRI_KEY_RANGE(IN_P, poll_in_progress, color))

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqsge_process, r3)

    nop.e
    nop

read:
    // prepare atomic header
    #phvwr           RETH_VA_RKEY_LEN, d.{read.va...read.length}
    phvwrpair      RETH_VA, d.read.va, RETH_RKEY, d.read.r_key
    phvwr          RETH_LEN, d.read.length

    // prepare RRQWQE descriptor
    phvwrpair      RRQWQE_READ_RSP_OR_ATOMIC, RRQ_OP_TYPE_READ, RRQWQE_NUM_SGES, d.base.num_sges
    mfspr          r3, spr_tbladdr
    add            r2, r3, TXWQE_SGE_OFFSET
    phvwrpair      RRQWQE_READ_LEN, d.read.length, RRQWQE_READ_WQE_SGE_LIST_ADDR, r2

    CAPRI_RESET_TABLE_2_ARG()
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, first, 1)
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, last_pkt, 1)
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, op_type, r1)
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_RD_P, op_rd_read_len, d.read.length)
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_RD_P, op_rd_log_pmtu, K_LOG_PMTU)
    // leave rest of variables to FALSE

    add            r2, HDR_TEMPLATE_T_SIZE_BYTES, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r2)
    CAPRI_SET_TABLE_0_1_VALID(0, 0);     

    nop.e
    nop

atomic:
    // prepare atomic header
    phvwrpair      ATOMIC_VA, d.atomic.va, ATOMIC_R_KEY, d.atomic.r_key
    phvwrpair      ATOMIC_SWAP_OR_ADD_DATA, d.atomic.swap_or_add_data, ATOMIC_CMP_DATA, d.atomic.cmp_data

    // prepare RRQWQE descriptor
    phvwrpair      RRQWQE_READ_RSP_OR_ATOMIC, RRQ_OP_TYPE_ATOMIC, RRQWQE_NUM_SGES, 1
    phvwr          p.{rrqwqe.atomic.sge.va...rrqwqe.atomic.sge.l_key}, d.{atomic.sge.va...atomic.sge.l_key}
 
    CAPRI_RESET_TABLE_2_ARG()
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, first, 1)
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, last_pkt, 1)
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, op_type, r1)
    // leave rest of variables to FALSE

    add            r2, HDR_TEMPLATE_T_SIZE_BYTES, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r2)
    CAPRI_SET_TABLE_0_1_VALID(0, 0)

    nop.e
    nop

inline_data:
    phvwr          p.inline_data, d.inline_data
    DMA_CMD_STATIC_BASE_GET(r4, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_PYLD_BASE)
    DMA_PHV2PKT_START_LEN_SETUP(r4, r5, inline_data, d.send.length)
    // NOTE: it should be noted that invoke_add_headers will directly invoke
    // add_headers phase without any sge process as the data is inline.
    // The length of data is populated in length argument. All the 'length'
    // parameter values for various union cases such as read/write/send are located
    // at same offset. So, though argument passing code is passing read.length, 
    // it should work for inline data as well.

    CAPRI_SET_FIELD2(TO_S4_P, packet_len, d.send.length)

    CAPRI_RESET_TABLE_2_ARG()
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, first, 1)
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, last_pkt, 1)
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, op_type, r1)
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, hdr_template_inline, 1)
    // should work for both send/write as imm_data is located at same offset in wqe for both operations
    CAPRI_SET_FIELD_RANGE2(SQCB_WRITE_BACK_SEND_WR_P, op_send_wr_imm_data, op_send_wr_inv_key_or_ah_handle, d.{send.imm_data...send.inv_key})
    CAPRI_SET_FIELD2_C(SQCB_WRITE_BACK_SEND_WR_P, op_send_wr_inv_key_or_ah_handle, r2, c1)
    CAPRI_SET_FIELD_RANGE2(SQCB_WRITE_BACK_P, poll_in_progress, color, CAPRI_KEY_RANGE(IN_P, poll_in_progress, color))
    // leave rest of variables to FALSE

    add            r2, HDR_TEMPLATE_T_SIZE_BYTES, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r2)
    CAPRI_SET_TABLE_0_1_VALID(0, 0)     

    nop.e
    nop

exit:
ud_error:
    //For UD we can silently drop
    phvwr.e   p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)    

clear_poll_in_progress:
    CAPRI_RESET_TABLE_2_ARG()
    CAPRI_SET_FIELD2(SQCB_WRITE_BACK_P, poll_failed, 1)

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r2)
    CAPRI_SET_TABLE_0_1_VALID(0, 0)
    nop.e
    nop
