#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqwqe_t d;
struct req_tx_sqwqe_process_k_t k;

#define INFO_OUT1_T struct req_tx_wqe_to_sge_info_t
#define INFO_OUT2_T struct req_tx_rrqwqe_to_hdr_info_t
#define INFO_OUT3_T struct req_tx_sqcb_write_back_info_t


    #c1 - inline_data_vld
    #c2 - UD service
    #c3 - UD error
    
%%
    .param    req_tx_sqsge_process
    .param    req_tx_add_headers_process
    .param    req_tx_write_back_process

.align
req_tx_sqwqe_process:
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    seq   c1, d.base.inline_data_vld, 1

    //if service == UD, length should not be greater than one PMTU
    //TODO: See if this can be optimized. Do we really need to do this check,
    // or can we do this check in Driver itself?

    seq       c2, k.args.ud, 1
    bcf       [!c2], skip_to_common
    add       r3, r0, k.args.log_pmtu
    sllv      r3, 1, r3
    add       r4, r0, d.ud_send.length
    blt       r3, r4, ud_error
    nop

skip_to_common: 
    add            r1, r0, d.base.op_type
    beqi           r1, OP_TYPE_WRITE, write
    nop
    beqi           r1, OP_TYPE_WRITE_IMM, write_imm
    nop
    beqi           r1, OP_TYPE_SEND, send
    nop
    beqi           r1, OP_TYPE_SEND_IMM, send_imm
    nop
    beqi           r1, OP_TYPE_SEND_INV, send_inv
    nop
    beqi           r1, OP_TYPE_READ, read
    nop
    beqi           r1, OP_TYPE_CMP_N_SWAP, atomic
    nop
    beqi           r1, OP_TYPE_FETCH_N_ADD, atomic
    nop

    nop.e
    nop

atomic:
    phvwr          ATOMIC_VA, d.atomic.va
    phvwr          ATOMIC_R_KEY, d.atomic.r_key
    phvwr          ATOMIC_CMP_DATA, d.atomic.cmp_data
    phvwr          ATOMIC_SWAP_OR_ADD_DATA, d.atomic.swap_or_add_data

    CAPRI_SET_FIELD(r7, INFO_OUT2_T, first, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, last, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op_type, r1)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.atomic.sge.va, d.atomic.sge.va)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.atomic.sge.len, d.atomic.sge.len)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.atomic.sge.l_key, d.atomic.sge.l_key)
    //CAPRI_SET_FIELD(r7, INFO_OUT2_T, tbl_id, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, log_pmtu, k.args.log_pmtu)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, rrq_p_index, k.args.rrq_p_index)

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_add_headers_process)
    SQCB1_ADDR_GET(r2)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r2)

    CAPRI_GET_TABLE_1_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, first, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, last, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, op_type, r1)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, tbl_id, 1)
    //CAPRI_SET_FIELD(r7, INFO_OUT3_T, release_cb1_busy, 0)
    // leave rest of variables to FALSE

    CAPRI_GET_TABLE_1_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_write_back_process)
    SQCB0_ADDR_GET(r2)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r2)

    nop.e
    nop

read:
    phvwr          RETH_VA, d.read.va
    phvwr          RETH_RKEY, d.read.r_key
    phvwr          RETH_LEN, d.read.length

invoke_add_headers:
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, first, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, last, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op_type, r1)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.rd.read_len, d.read.length)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.rd.num_sges, d.base.num_sges)
    //CAPRI_SET_FIELD(r7, INFO_OUT2_T, tbl_id, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, log_pmtu, k.args.log_pmtu)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, rrq_p_index, k.args.rrq_p_index)

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_add_headers_process)
    SQCB1_ADDR_GET(r2)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r2)

    CAPRI_GET_TABLE_1_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, first, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, last, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, op_type, r1)
    CAPRI_SET_FIELD(r7, INFO_OUT3_T, tbl_id, 1)
    //CAPRI_SET_FIELD(r7, INFO_OUT3_T, release_cb1_busy, 0)
    // leave rest of variables to FALSE

    CAPRI_GET_TABLE_1_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_write_back_process)
    SQCB0_ADDR_GET(r2)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r2)

    nop.e
    nop

    nop.e
    nop

write_imm:
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, imm_data, d.write.imm_data)
    //pass thru to write: logic

write:
    phvwr RETH_VA, d.write.va
    phvwr RETH_RKEY, d.write.r_key
    phvwr RETH_LEN, d.write.length
    b     common
    nop

send_inv:
    b     common
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, inv_key, d.send.inv_key) //branch delay slot

send_imm:
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, imm_data, d.send.imm_data)

send:

    bcf   [!c2], common //If not UD, skip error checking
    nop
    
    // Needed only for UD service    
    phvwr DETH_Q_KEY, d.ud_send.q_key
    phvwr DETH_SRC_QP, k.global.qid
    phvwr BTH_DST_QP, d.ud_send.dst_qp
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, ah_handle, d.ud_send.ah_handle) 

common:
    bcf         [c1], inline_data
    phvwr.c1    p.inline_data, d.inline_data    //BD Slot
    // populate stage-2-stage data req_tx_wqe_to_sge_info_t for next stage
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, first, 1)
    //CAPRI_SET_FIELD(r7, INFO_OUT1_T, current_sge_id, 0)
    //CAPRI_SET_FIELD(r7, INFO_OUT1_T, current_sge_offset, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, num_valid_sges, d.base.num_sges)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    //CAPRI_SET_FIELD(r7, INFO_OUT1_T, wqe_addr, k.args.wqe_addr)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, dma_cmd_start_index, REQ_TX_RDMA_PAYLOAD_DMA_CMDS_START)
    CAPRI_SET_FIELD(r7, INFO_OUT1_T, op_type, d.base.op_type)
    //CAPRI_SET_FIELD(r7, INFO_OUT1_T, imm_data, 0)
    //CAPRI_SET_FIELD(r7, INFO_OUT1_T, inv_key, 0)

    // sqwqe_p->sge_list[0] = sqcb_to_wqe_info_p->wqe_ptr + TX_SGE_OFFSET
    add            r3, k.to_stage.wqe_addr, TXWQE_SGE_OFFSET

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqsge_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r3)

    nop.e
    nop

inline_data:
    DMA_CMD_I_BASE_GET(r4, r5, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_RDMA_PAYLOAD_DMA_CMDS_START)
    DMA_PHV2PKT_START_LEN_SETUP(r4, r5, inline_data, d.read.length)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r4)
    // should work for both send/write as imm_data is located at same offset in wqe for both operations
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.send_wr.imm_data, d.send.imm_data)
    CAPRI_SET_FIELD(r7, INFO_OUT2_T, op.send_wr.inv_key, d.send.inv_key)
    b   invoke_add_headers
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r4)  //BD Slot
    // NOTE: it should be noted that invoke_add_headers will directly invoke
    // add_headers phase without any sge process as the data is inline.
    // The length of data is populated in length argument. All the 'length'
    // parameter values for various union cases such as read/write/send are located
    // at same offset. So, though argument passing code is passing read.length, 
    // it should work for inline data as well.

    nop.e
    nop
    
ud_error:
    //For UD we can silently drop
    phvwr.e   p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0);     

    
