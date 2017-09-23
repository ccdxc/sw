#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqwqe_t d;
struct req_tx_sqwqe_process_k_t k;

#define INFO_OUT_T struct req_tx_wqe_to_sge_info_t

%%
    .param    req_tx_sqsge_process

.align
req_tx_sqwqe_process:

    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    add   r2, r0, offsetof(struct phv_, common_global_global_data)

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

    nop.e
    nop

write_imm:
    CAPRI_SET_FIELD(r7, INFO_OUT_T, imm_data, d.write.imm_data)
    //pass thru to write: logic

write:
    phvwr RETH_VA, d.write.va
    phvwr RETH_RKEY, d.write.r_key
    phvwr RETH_LEN, d.write.length
    b     common
    nop

send_inv:
    b     common
    CAPRI_SET_FIELD(r7, INFO_OUT_T, inv_key, d.send.inv_key) //branch delay slot

send_imm:
    CAPRI_SET_FIELD(r7, INFO_OUT_T, imm_data, d.send.imm_data)
send:

common:
    // populate stage-2-stage data req_tx_wqe_to_sge_info_t for next stage
    CAPRI_SET_FIELD(r7, INFO_OUT_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, first, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, current_sge_id, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, current_sge_offset, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, num_valid_sges, d.base.num_sges)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    //CAPRI_SET_FIELD(r7, INFO_OUT_T, wqe_addr, k.args.wqe_addr)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, dma_cmd_start_index, REQ_TX_RDMA_PAYLOAD_DMA_CMDS_START)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, op_type, d.base.op_type)
    //CAPRI_SET_FIELD(r7, INFO_OUT_T, imm_data, 0)
    //CAPRI_SET_FIELD(r7, INFO_OUT_T, inv_key, 0)

    // sqwqe_p->sge_list[0] = sqcb_to_wqe_info_p->wqe_ptr + TX_SGE_OFFSET
    add            r3, k.to_stage.wqe_addr, TXWQE_SGE_OFFSET

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqsge_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r3)

    nop.e
    nop
