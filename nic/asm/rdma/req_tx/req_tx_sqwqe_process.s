#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqwqe_base_t d;
struct req_tx_sqwqe_process_k_t k;

#define INFO_OUT_T struct req_tx_wqe_to_sge_info_t

%%
    .param    req_tx_sqsge_process

.align
req_tx_sqwqe_process:

    add            r1, r0, d.op_type
    beqi           r1, OP_TYPE_SEND, send
    nop

    nop.e
    nop

send:
    // populate stage-2-stage data req_tx_wqe_to_sge_info_t for next stage
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, first, 1)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, current_sge_id, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, current_sge_offset, 0)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, num_valid_sges, d.num_sges)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, wqe_addr, k.args.wqe_addr)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, dma_cmd_start_index, REQ_TX_RDMA_PAYLOAD_DMA_CMDS_START)
    CAPRI_SET_FIELD(r7, INFO_OUT_T, op_type, d.op_type)
    //CAPRI_SET_FIELD(r7, INFO_OUT_T, imm_data, 0)
    //CAPRI_SET_FIELD(r7, INFO_OUT_T, inv_key, 0)

    // sqwqe_p->sge_list[0] = sqcb_to_wqe_info_p->wqe_ptr + TX_SGE_OFFSET
    add            r2, k.args.wqe_addr, TXWQE_SGE_OFFSET

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_sqsge_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r2)

    nop.e
    nop
