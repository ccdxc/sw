#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_rx_phv_t p;
struct rqcb0_t d;
//struct resp_rx_rqcb_process_k_t k;
struct rdma_stage0_table_k k;

#define RQCB_TO_WRITE_T struct resp_rx_rqcb_to_write_rkey_info_t
#define INFO_OUT1_T struct resp_rx_rqcb_to_pt_info_t
#define RQCB_TO_RQCB1_T struct resp_rx_rqcb_to_rqcb1_info_t
#define RAW_TABLE_PC r2

#define REM_PYLD_BYTES  r6

%%
    .param    resp_rx_rqpt_process
    .param    resp_rx_rqcb1_in_progress_process
    .param    resp_rx_write_dummy_process

.align
resp_rx_rqcb_process:

    // populate global fields
    add r3, r0, offsetof(struct phv_, common_global_global_data)

    // we don't need spr_tbladdr for now, as QSTATE_ADDR is anyway available
    // as part of raw intrinsic params
    #mfspr r1, spr_tbladdr	
    add r1, r0, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR
    srl r1, r1, RQCB_ADDR_SHIFT
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, cb_addr, r1)

    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, lif, CAPRI_RXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, qtype, CAPRI_RXDMA_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, qid, CAPRI_RXDMA_INTRINSIC_QID)
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, flags, CAPRI_APP_DATA_RAW_FLAGS)

    //set DMA cmd ptr
    RXDMA_DMA_CMD_PTR_SET(RESP_RX_DMA_CMD_START_FLIT_ID)

    // MPU GLOBAL
    // take a copy of raw_flags in r7 and keep it for further checks
    add     r7, r0, CAPRI_APP_DATA_RAW_FLAGS
    add     REM_PYLD_BYTES, r0, CAPRI_APP_DATA_PAYLOAD_LEN

    // get a tokenid
    phvwr   p.my_token_id, d.token_id

    // check if it is my turn. if not, recirc.
    seq     c1, d.token_id, d.nxt_to_go_token_id
    bcf     [!c1], recirc
    tbladd  d.token_id, 1   //BD Slot

    //got my turn, do sanity checks

    // is service type correct ?
    srl     r1, CAPRI_APP_DATA_BTH_OPCODE, BTH_OPC_SVC_SHIFT
    seq     c1, r1, d.serv_type
    bcf     [!c1], nak
    add     r2, r0, NAK_CODE_INV_REQ //BD Slot

    // check for seq_err (e_psn < bth.psn)
    // TODO: psn wraparound
    slt     c1, d.e_psn, CAPRI_APP_DATA_BTH_PSN
    bcf     [c1], nak
    add     r2, r0, NAK_CODE_SEQ_ERR //BD Slot

    // check for duplicate
    // TODO: psn wraparound
    slt     c1, CAPRI_APP_DATA_BTH_PSN, d.e_psn
    bcf     [c1], duplicate
    nop     //BD Slot

    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_FIRST|RESP_RX_FLAG_MIDDLE)
    // remaining_payload_bytes - (1 << log_pmtu)
    add     r1, r0, d.log_pmtu
    sllv    r1, 1, r1
    sub     r1, r6, r1
    // first/middle packets should be of pmtu size
    seq     c2, r1, r0
    bcf.c1  [!c2], nak
    add.c1  r2, r0, NAK_CODE_INV_REQ //BD Slot
     
    // INCREMENT E_PSN HERE
    tbladd  d.e_psn, 1

    ARE_ALL_FLAGS_SET(c1, r7, RESP_RX_FLAG_WRITE)
    bcf     [!c1], need_checkout
    nop     //BD Slot

write:
    // only first and only packets have reth header
    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_FIRST)
    IS_ANY_FLAG_SET(c2, r7, RESP_RX_FLAG_LAST)
    IS_ANY_FLAG_SET(c3, r7, RESP_RX_FLAG_ONLY)
    setcf   c4, [c1 | c3]       // FIRST | ONLY
    IS_ANY_FLAG_SET(c5, r7, RESP_RX_FLAG_IMMDT)
    setcf   c6, [c5 & c2]       // IMM & LAST
    setcf   c7, [c5 & c3]       // IMM & ONLY

    // populate immediate data. 
    // in case of WRITE_LAST_WITH_IMM, we will have immeth hdr right after bth
    // in case of WRITE_ONLY_WITH_IMM, we will have immeth hdr after bth and reth
    
    // populate cq op_type
    phvwr.c5    p.cqwqe.op_type, OP_TYPE_RDMA_OPER_WITH_IMM
    
    // populate immediate data. 
    phvwr.c5    p.cqwqe.imm_data_vld, 1
    // IMM & LAST
    phvwr.c6    p.cqwqe.imm_data, CAPRI_RXDMA_BTH_IMMETH_IMMDATA
    // TODO: waiting on NCC fix to enable this statement
    // IMM & ONLY
    //phvwr.c7    p.cqwqe.imm_data, CAPRI_RXDMA_BTH_RETH_IMMETH_IMMDATA
    
    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)

    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, load_reth, 0, c4)
    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, load_reth, 1, !c4)

    CAPRI_RXDMA_RETH_VA(r5)
    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, va, r5, c4)

    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, len, CAPRI_RXDMA_RETH_DMA_LEN, c4)
    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, r_key, CAPRI_RXDMA_RETH_R_KEY, c4)
    // TODO: DANGER: do we need to set incr_c_index to 0 otherwise ? 
    // If not, would it accidentally carry previous s2s data and increment c_index ?
    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, incr_c_index, 1, c5)
    CAPRI_SET_FIELD(r4, RQCB_TO_WRITE_T, remaining_payload_bytes, r6)

    // NOTE: key_table_base_addr is NOT known yet at this time
    // as this is still stage 0 and parallel SRAM lookup would
    // be acquiring this information. Hence we need to create
    // a bubble and wait till the next stage
    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_write_dummy_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r3)

    //TODO: tbl_id management

need_checkout:

    ARE_ALL_FLAGS_SET(c1, r7, RESP_RX_FLAG_SEND)
    ARE_ALL_FLAGS_SET(c2, r7, RESP_RX_FLAG_WRITE|RESP_RX_FLAG_IMMDT)
    ARE_ALL_FLAGS_SET(c5, r7, RESP_RX_FLAG_COMPLETION)
    seq     c3, d.in_progress, 1
    setcf   c4, [c1 & !c3]

    // don't need to set status field explicitly to 0
    //phvwr.c5    p.cqwqe.status, CQ_STATUS_SUCCESS
    phvwr.c5    p.cqwqe.qp, CAPRI_RXDMA_INTRINSIC_QID

    // populate cq op_type
    setcf       c6, [c1 & c5]
    phvwr.c6    p.cqwqe.op_type, OP_TYPE_SEND_RCVD

    // populate immediate data. 
    ARE_ALL_FLAGS_SET(c6, r7, RESP_RX_FLAG_IMMDT)
    phvwr.c6    p.cqwqe.imm_data_vld, 1
    phvwr.c6    p.cqwqe.imm_data, CAPRI_RXDMA_BTH_IMMETH_IMMDATA
    
    // checkout a RQ descriptor if it is a send AND in_progress is FALSE
    // OR write_with_imm
    bcf     [c2|c4], checkout
    nop     //BD Slot

    bcf     [!c1], exit
    nop     //BD Slot
    // by the time we reach here, it is SEND & in_progress == TRUE

    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, in_progress, d.in_progress)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, remaining_payload_bytes, REM_PYLD_BYTES)

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqcb1_in_progress_process)
    add     r3, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, 1, LOG_CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r3)


exit:
    nop.e
    nop

checkout:
    // checkout a descriptor
    add     r1, r0, RQ_C_INDEX
    sub     r2, d.log_rq_page_size, d.log_wqe_size
    // page_index = c_index >> (log_rq_page_size - log_wqe_size)
    srlv    r3, r1, r2

    // page_offset = c_index & ((1 << (log_rq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr   r1, r2, r0
    add     r2, r0, d.log_wqe_size
    sllv    r1, r1, r2

    // r3 has page_index, r1 has page_offset by now

    // page_seg_offset = page_index & 0x7
    and     r5, r3, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    sub     r3, r3, r5
    // page_index = page_index * sizeof(u64)
    sll     r3, r3, CAPRI_LOG_SIZEOF_U64
    // page_index += rqcb_p->pt_base_addr
    add     r3, r3, d.pt_base_addr
    // now r3 has page_p to load
    
    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, in_progress, 0)
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, tbl_id, 0) //TODO: set tbl_id
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, cache, d.cache)
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, page_seg_offset, r5)
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, page_offset, r1)
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, remaining_payload_bytes, REM_PYLD_BYTES)

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqpt_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r3)
    nop.e
    nop

duplicate:
    nop.e
    nop

recirc:
    phvwr   p.common.p4_intr_recirc, 1
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop

nak:
    // assumption: r2 has the nak code

    // copy nak info
    phvwr   p.ack_info.psn, d.e_psn
    phvwr   p.ack_info.aeth.msn, d.msn
    AETH_NAK_SYNDROME_GET(r3, r2)
    phvwr   p.ack_info.aeth.syndrome, r3
    
    //TODO: DMA Commands to xfer p.ack_info to rqcb1
    //TODO: ring ack_nak_ring doorbell

    // release chance to next packet
    tbladd  d.nxt_to_go_token_id, 1  
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop
