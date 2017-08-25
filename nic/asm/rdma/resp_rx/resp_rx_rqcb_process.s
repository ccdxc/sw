#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_rx_phv_t p;
struct rqcb0_t d;
//struct resp_rx_rqcb_process_k_t k;
struct rdma_stage0_table_k k;

#define INFO_OUT1_T struct resp_rx_rqcb_to_pt_info_t
#define RAW_TABLE_PC r2

%%
    .param    resp_rx_rqpt_process

.align
resp_rx_rqcb_process:

    // populate global fields
    add r3, r0, offsetof(struct phv_, common_global_global_data)

    // enable below code after spr_tbladdr special purpose register is available in capsim
    #mfspr r1, spr_tbladdr	
    #srl r1, r1, RQCB_ADDR_SHIFT
    #CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, cb_addr, r1)

    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, lif, CAPRI_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, qtype, CAPRI_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, qid, CAPRI_INTRINSIC_QID)
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, flags, CAPRI_APP_DATA_RAW_FLAGS)

    //set DMA cmd ptr
    RXDMA_DMA_CMD_PTR_SET(RESP_RX_DMA_CMD_START_FLIT_ID)

    // MPU GLOBAL
    // take a copy of raw_flags in r7 and keep it for further checks
    add     r7, r0, CAPRI_APP_DATA_RAW_FLAGS
    add     r6, r0, CAPRI_APP_DATA_PAYLOAD_LEN

    srl     r7, r7, 8
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

    // INCREMENT E_PSN HERE
    tbladd  d.e_psn, 1
    //


    ARE_ALL_FLAGS_SET(c1, r7, RESP_RX_FLAG_SEND)
    ARE_ALL_FLAGS_SET(c2, r7, RESP_RX_FLAG_WRITE|RESP_RX_FLAG_IMMDT)
    seq     c3, d.in_progress, 1
    setcf   c4, [c1 & !c3]

    // checkout a RQ descriptor if it is a send AND in_progress is FALSE
    // OR write_with_imm
    bcf     [c2|c4], checkout
    nop     //BD Slot

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
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, remaining_payload_bytes, r6)

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
