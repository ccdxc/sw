#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqwqe_base_t d;
struct req_tx_sqwqe_process_k_t k;

#define INFO_OUT_T struct req_tx_wqe_to_sge_info_t

%%
    .param    req_tx_sqsge_process

req_tx_sqwqe_process:

    add    r1, r0, d.op_type
    beqi   r1, OP_TYPE_SEND, SEND
    // populate stage-2-stage data req_tx_wqe_to_sge_info_t for next stage
    add     r1, r0, offsetof(struct req_tx_phv_t, common.common_t0_s2s_s2s_data) // branch delay slot
    nop.e
    nop

SEND:
    CAPRI_SET_FIELD(r1, INFO_OUT_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r1, INFO_OUT_T, current_sge_id, 0)
    CAPRI_SET_FIELD(r1, INFO_OUT_T, current_sge_offset, 0)
    CAPRI_SET_FIELD(r1, INFO_OUT_T, num_valid_sges, d.num_sges)
    CAPRI_SET_FIELD(r1, INFO_OUT_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(r1, INFO_OUT_T, wqe_addr, k.args.wqe_addr)
    CAPRI_SET_FIELD(r1, INFO_OUT_T, dma_cmd_start_index, REQ_TX_RDMA_PAYLOAD_DMA_CMDS_START)
    CAPRI_SET_FIELD(r1, INFO_OUT_T, op_type, d.op_type)
    CAPRI_SET_FIELD(r1, INFO_OUT_T, imm_data, 0)
    CAPRI_SET_FIELD(r1, INFO_OUT_T, inv_key, 0)

    // sqcb_to_wqe_info_p->wqe_ptr + TX_SGE_OFFSET
    add     r2, k.args.wqe_addr, TXWQE_SGE_OFFSET

    add     r1, r0, offsetof(struct req_tx_phv_t, common.common_te0_phv_table_addr)
    CAPRI_NEXT_TABLE_I_READ(r1, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqsge_process, r2)

    nop.e
    nop
