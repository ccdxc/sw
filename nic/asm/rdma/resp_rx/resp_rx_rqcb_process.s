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
#define RSQ_BT_S2S_INFO_T struct resp_rx_rsq_backtrack_info_t 
#define RSQ_BT_TO_S_INFO_T struct resp_rx_to_stage_backtrack_info_t
#define TO_S_FWD_INFO_T struct resp_rx_to_stage_forward_info_t
#define RQCB_TO_RQCB1_T struct resp_rx_rqcb_to_rqcb1_info_t
#define RAW_TABLE_PC r2

#define REM_PYLD_BYTES  r6
#define RSQWQE_P r2
#define DMA_CMD_BASE r1
#define TMP r3
#define IMM_DATA r3
#define DB_ADDR r4
#define DB_DATA r5
#define NEW_RSQ_P_INDEX r6
#define RQCB1_ADDR r6

%%
    .param    resp_rx_rqpt_process
    .param    resp_rx_rqcb1_in_progress_process
    .param    resp_rx_write_dummy_process
    .param    resp_rx_rsq_backtrack_process

.align
resp_rx_rqcb_process:
    //add     r1, r0, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT
    //b       recirc_wait_for_turn
    //nop

    // MPU GLOBAL
    // take a copy of raw_flags in r7 and keep it for further checks
    add     r7, r0, CAPRI_APP_DATA_RAW_FLAGS

    // is this a fresh packet ?
    seq     c1, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    bcf     [!c1], recirc_pkt
    crestore [c7, c6, c5], r7, (RESP_RX_FLAG_ONLY| RESP_RX_FLAG_LAST|RESP_RX_FLAG_FIRST)    //BD Slot

    //fresh packet

    // populate global fields
    add r3, r0, offsetof(struct phv_, common_global_global_data)

    // we don't need spr_tbladdr for now, as QSTATE_ADDR is anyway available
    // as part of raw intrinsic params
    #mfspr r1, spr_tbladdr	
    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(RQCB_ADDR_SHIFT))

    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, lif, CAPRI_RXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD_RANGE(r3, PHV_GLOBAL_COMMON_T, qid, qtype, CAPRI_RXDMA_INTRINSIC_QID_QTYPE)

    // TODO: Migrate ACK_REQ flag to P4 table
    seq     c5, CAPRI_APP_DATA_BTH_ACK_REQ, 1
    or.c5   r7, r7, RESP_RX_FLAG_ACK_REQ

    CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, flags, r7)

    //set DMA cmd ptr
    RXDMA_DMA_CMD_PTR_SET(RESP_RX_DMA_CMD_START_FLIT_ID)

    // get a tokenid for the fresh packet
    phvwr   p.common.rdma_recirc_token_id, d.token_id

    // for now copy to both stage_2 and stage_3 to_stage info
    // as write is using stage_2 for write_back and send is using stage_3.
    // TODO: fix it
    CAPRI_GET_STAGE_2_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, TO_S_FWD_INFO_T, my_token_id, d.token_id)
    CAPRI_GET_STAGE_3_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, TO_S_FWD_INFO_T, my_token_id, d.token_id)

    // if it is a first packet, disable further speculation
    tblwr.c5    d.disable_speculation, 1

    // if it is the only packet, skip nxt_to_go_token_id check
    // if speculation is enabled
    seq         c4, d.disable_speculation, 0
    bcf.c7      [c4], skip_token_id_check
    // check if it is my turn. if not, recirc.
    seq         c1, d.token_id, d.nxt_to_go_token_id    //BD Slot

    bcf         [!c1], recirc_wait_for_turn
    tbladd.!c1  d.token_id, 1   //BD Slot

    //got my turn, do sanity checks

    // if it is the last packet, enable speculation again
    tblwr.c6    d.disable_speculation, 0

skip_token_id_check:
    tbladd  d.token_id, 1

    add     REM_PYLD_BYTES, r0, CAPRI_APP_DATA_PAYLOAD_LEN

    // populate some fields on ack_info so that in case we need to 
    // send NAK from anywhere, this info is already populated.
    phvwr   p.ack_info.psn, d.e_psn
    phvwr   p.ack_info.aeth.msn, d.msn

    IS_ANY_FLAG_SET(c2, r7, RESP_RX_FLAG_UD)
    
    // is service type correct ?
    srl         r1, CAPRI_APP_DATA_BTH_OPCODE, BTH_OPC_SVC_SHIFT
    seq         c1, r1, d.serv_type
    bcf         [!c1], nak
    phvwr.!c1   p.ack_info.aeth.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_INV_REQ) //BD Slot

    //For UD we dont need PSN checks and MSN updates.
    bcf     [c2], need_checkout_ud
    
    // check for seq_err (e_psn < bth.psn)
    //slt     c1, d.e_psn, CAPRI_APP_DATA_BTH_PSN
    scwlt24     c1, d.e_psn, CAPRI_APP_DATA_BTH_PSN  //BD slot.
    bcf         [c1], nak
    phvwr.c1    p.ack_info.aeth.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_SEQ_ERR) //BD Slot

    // check for duplicate
    seq     c1, CAPRI_APP_DATA_BTH_PSN, d.e_psn
    bcf     [!c1], duplicate
    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_LAST | RESP_RX_FLAG_ONLY | RESP_RX_FLAG_ATOMIC_FNA | RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_READ_REQ) //BD slot

    // if not last or only, check for ack request bit case
    bcf     [!c1 & c5], skip_msn_incr
    add         r1, r0, d.msn #BD Slot
    bcf     [!c1], check_read
    mincr       r1, 24, 1 #BD slot
    tblwr       d.msn, r1

skip_msn_incr:
    phvwr       p.ack_info.aeth.msn, r1
    // Don't need to do below instruction as it is already done above
    //phvwr       p.ack_info.psn, d.e_psn
    RQ_CREDITS_GET(r1, r2, c1)
    AETH_ACK_SYNDROME_GET(r2, r1)
    phvwr       p.ack_info.aeth.syndrome, r2

    // check for read
check_read:
    ARE_ALL_FLAGS_SET(c1, r7, RESP_RX_FLAG_READ_REQ)
    bcf     [!c1], check_write
    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_FIRST|RESP_RX_FLAG_MIDDLE) //BD Slot

read:
    // populate rsqwqe in phv and then DMA
    phvwr   p.rsqwqe.psn, d.e_psn
    CAPRI_RXDMA_RETH_VA(r5)
    phvwr   p.rsqwqe.read.va, r5
    phvwr   p.rsqwqe.read.r_key, CAPRI_RXDMA_RETH_R_KEY
    phvwr   p.rsqwqe.read.len, CAPRI_RXDMA_RETH_DMA_LEN
    // phv is already initialized to 0, don't need below instructions
    //phvwr   p.rsqwqe.read_or_atomic, RSQ_OP_TYPE_READ
    //phvwr   p.rsqwqe.read.offset, 0

    // DMA for copying phv.ack_info to RQCB1 
    add     RQCB1_ADDR, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, 1, LOG_CB_UNIT_SIZE_BYTES
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK)
    RESP_RX_POST_ACK_INFO_TO_TXDMA_NO_DB(DMA_CMD_BASE, RQCB1_ADDR, TMP)

    // wqe_p = (void *)(hbm_addr_get(rqcb_p->rsq_base_addr) +    
    //                      (sizeof(rsqwqe_t) * p_index));
    seq         c1, d.rsq_quiesce, 1
    // if rsq_quiesce is on, use rsq_p_index_prime, else use rsq_p_index
    cmov        NEW_RSQ_P_INDEX, c1, RSQ_P_INDEX_PRIME, RSQ_P_INDEX
    add         RSQWQE_P, d.rsq_base_addr, NEW_RSQ_P_INDEX, LOG_SIZEOF_RSQWQE_T
    // p_index/c_index are in little endian
    mincr       NEW_RSQ_P_INDEX, d.log_rsq_size, 1
   
    // DMA for RSQWQE
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_RSQWQE)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, rsqwqe, rsqwqe, RSQWQE_P)
    
    // in case of quiesce mode, only duplicate reqs would move rsq_p_index in a 
    // lock step manner. If we receive non-duplicate, we increment only
    // rsq_p_index_prime. Standard is not clear whether we can handle fresh
    // requests while duplicate requests are being handled. If needed, we can
    // stop dropping new requests if quiesce mode is on.
    // 
    bcf         [c1], skip_rsq_doorbell
    tblwr.c1    RSQ_P_INDEX_PRIME, NEW_RSQ_P_INDEX      //BD Slot

    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, CAPRI_RXDMA_INTRINSIC_LIF, CAPRI_RXDMA_INTRINSIC_QTYPE, DB_ADDR)
    CAPRI_SETUP_DB_DATA(CAPRI_RXDMA_INTRINSIC_QID, RSQ_RING_ID, NEW_RSQ_P_INDEX, DB_DATA)
    // store db_data in LE format
    phvwr   p.db_data1, DB_DATA.dx

    // DMA for RSQ DoorBell
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_RSQ_DB)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data1, db_data1, DB_ADDR)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

skip_rsq_doorbell:
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    // increment nxt_to_go_token_id to give control to next pkt
    tbladd  d.nxt_to_go_token_id, 1  

    // e_psn += read_len >> log_pmtu
    add            r3, d.log_pmtu, r0
    srlv           r3, CAPRI_RXDMA_RETH_DMA_LEN, r3
    tblmincr       d.e_psn, 24, r3

    // e_psn += (read_len & ((1 << log_pmtu) -1)) ? 1 : 0
    add            r3, CAPRI_RXDMA_RETH_DMA_LEN, r0
    mincr          r3, d.log_pmtu, r0
    sle            c6, r3, r0
    tblmincri.!c6  d.e_psn, 24, 1

    nop.e
    nop

check_write:

    //IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_FIRST|RESP_RX_FLAG_MIDDLE)
    // remaining_payload_bytes - (1 << log_pmtu)
    add     r1, r0, d.log_pmtu
    sllv    r1, 1, r1
    sub     r1, REM_PYLD_BYTES, r1
    // first/middle packets should be of pmtu size
    sne         c2, r1, r0
    setcf       c4, [c1 & c2]  // (FIRST OR MIDDLE) & PMTU error
    bcf         [c4], nak
    phvwr.c4    p.ack_info.aeth.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_INV_REQ) //BD Slot
     
    crestore [c5,c4,c3,c2,c1], r7, (RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_ONLY | RESP_RX_FLAG_LAST | RESP_RX_FLAG_FIRST)
    bcf     [!c4], need_checkout
    // INCREMENT E_PSN HERE
    tblmincri   d.e_psn, 24, 1 //BD slot


write:
    // only first and only packets have reth header
    setcf   c4, [c1 | c3]       // FIRST | ONLY
    setcf   c6, [c5 & c2]       // IMM & LAST
    setcf   c7, [c5 & c3]       // IMM & ONLY

    // populate immediate data. 
    // in case of WRITE_LAST_WITH_IMM, we will have immeth hdr right after bth
    // in case of WRITE_ONLY_WITH_IMM, we will have immeth hdr after bth and reth
    
    // populate cq op_type
    bcf     [!c5], skip_write_immdt
    phvwr.c5    p.cqwqe.op_type, OP_TYPE_RDMA_OPER_WITH_IMM
    
    // populate immediate data. 
    phvwr.c5    p.cqwqe.imm_data_vld, 1
    // IMM & LAST
    add.c6      IMM_DATA, r0, CAPRI_RXDMA_BTH_IMMETH_IMMDATA
    CAPRI_RXDMA_BTH_RETH_IMMETH_IMMDATA_C(IMM_DATA, c7)
    phvwr.c5    p.cqwqe.imm_data, IMM_DATA

    //overwrite c5, if immdt_as_dbell is 1
    seq.c5 c5, d.immdt_as_dbell, 0

skip_write_immdt:

    CAPRI_GET_TABLE_1_ARG(resp_rx_phv_t, r4)

    //CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, load_reth, 0, c4)
    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, load_reth, 1, !c4)

    CAPRI_RXDMA_RETH_VA(r5)
    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, va, r5, c4)

    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, len, CAPRI_RXDMA_RETH_DMA_LEN, c4)
    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, r_key, CAPRI_RXDMA_RETH_R_KEY, c4)
    CAPRI_SET_FIELD_C(r4, RQCB_TO_WRITE_T, incr_c_index, 1, c5)
    CAPRI_SET_FIELD(r4, RQCB_TO_WRITE_T, remaining_payload_bytes, REM_PYLD_BYTES)

    // NOTE: key_table_base_addr is NOT known yet at this time
    // as this is still stage 0 and parallel SRAM lookup would
    // be acquiring this information. Hence we need to create
    // a bubble and wait till the next stage
    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_write_dummy_process)
    add     r5, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, 1, LOG_CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r5)

    //TODO: tbl_id management

need_checkout:

    crestore [c7, c5, c2, c1], r7, (RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_SEND) 
    seq     c3, d.in_progress, 1
    setcf   c4, [c1 & !c3]

    // don't need to set status field explicitly to 0
    //phvwr.c7    p.cqwqe.status, CQ_STATUS_SUCCESS
    phvwr.c7    p.cqwqe.qp, CAPRI_RXDMA_INTRINSIC_QID

    // populate cq op_type
    setcf       c6, [c1 & c7]
    phvwr.c6    p.cqwqe.op_type, OP_TYPE_SEND_RCVD

    phvwr.c5    p.cqwqe.rkey_inv_vld, 1 
    phvwr.c5    p.cqwqe.r_key, CAPRI_RXDMA_BTH_IETH_R_KEY
    
    bcf     [!c2], skip_immdt

    // populate immediate data. 
    ARE_ALL_FLAGS_SET(c6, r7, RESP_RX_FLAG_IMMDT|RESP_RX_FLAG_SEND)
    phvwr.c6    p.cqwqe.imm_data_vld, 1
    phvwr.c6    p.cqwqe.imm_data, CAPRI_RXDMA_BTH_IMMETH_IMMDATA

    // If immdt_as_dbell flag is set, adjust the flags
    seq     c6, d.immdt_as_dbell, 1
    bcf     [!c6], skip_immdt_as_dbell
    add.c1  IMM_DATA, r0, CAPRI_RXDMA_BTH_IMMETH_IMMDATA
    and     r7, r7, ~(RESP_RX_FLAG_IMMDT)
    and.!c1 r7, r7, ~(RESP_RX_FLAG_COMPLETION)
    or      r7, r7, RESP_RX_FLAG_RING_DBELL
    add r2, r0, offsetof(struct phv_, common_global_global_data)
    CAPRI_SET_FIELD(r2, PHV_GLOBAL_COMMON_T, flags, r7)
    phvwr       p.cqwqe.imm_data_vld, 0
    phvwr       p.cqwqe.imm_data, 0 

    //IMM_DATA has imm_data
    //format: <lif(11), qtype(3), qid(18)>

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_IMMDT_AS_DBELL)
    RESP_RX_POST_IMMDT_AS_DOORBELL(DMA_CMD_BASE, \
                                   IMM_DATA[31:21], \
                                   IMM_DATA[20:18], \
                                   IMM_DATA[17:0], \
                                   DB_ADDR, DB_DATA)
    DMA_SET_END_OF_CMDS_C(struct capri_dma_cmd_pkt2mem_t, DMA_CMD_BASE, !c1)

skip_immdt:
skip_immdt_as_dbell:

    // checkout a RQ descriptor if it is a send AND in_progress is FALSE
    // OR write_with_imm
    ARE_ALL_FLAGS_SET(c2, r7, RESP_RX_FLAG_WRITE|RESP_RX_FLAG_IMMDT)
    bcf     [c2|c4], checkout
    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4) //BD Slot

    bcf     [!c1], exit
    nop     //BD Slot
    // by the time we reach here, it is SEND & in_progress == TRUE

    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, in_progress, d.in_progress)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, remaining_payload_bytes, REM_PYLD_BYTES)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, inv_r_key, CAPRI_RXDMA_BTH_IETH_R_KEY)

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqcb1_in_progress_process)
    add     r3, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, 1, LOG_CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r3)


exit:
    nop.e
    nop

need_checkout_ud:

    //UD_TODO: I need to do length check to make sure it is less than PMTU.
    ARE_ALL_FLAGS_SET(c6, r7, RESP_RX_FLAG_IMMDT|RESP_RX_FLAG_SEND)

    add     r1, r0, d.log_pmtu
    sllv    r1, 1, r1
    blt     r1, REM_PYLD_BYTES, ud_drop
    
    //Need to check q_key comparision
    add    r1, r0, CAPRI_RXDMA_DETH_Q_KEY   //BD Slot
    add    r2, r0, d.q_key
    bne    r1, r2, ud_drop
    
    // don't need to set status field explicitly to 0
    //phvwr    p.cqwqe.status, CQ_STATUS_SUCCESS
    phvwr    p.cqwqe.qp, CAPRI_RXDMA_INTRINSIC_QID //BD Slot

    // populate cq op_type
    phvwr       p.cqwqe.op_type, OP_TYPE_SEND_RCVD

    // populate src_qp
    phvwr       p.cqwqe.src_qp, CAPRI_RXDMA_DETH_SRC_QP

    //populate ipv4 or ipv6
    phvwr       p.cqwqe.ipv4, 1

    //populate smac
    phvwr.!c6       p.cqwqe.smac, CAPRI_RXDMA_DETH_SMAC
    phvwr.c6       p.cqwqe.smac, CAPRI_RXDMA_DETH_IMMETH_SMAC
    
    // populate immediate data. 
    phvwr.c6    p.cqwqe.imm_data_vld, 1
    phvwr.c6    p.cqwqe.imm_data, CAPRI_RXDMA_DETH_IMMETH_DATA

    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4) 

checkout:
    // checkout a descriptor
    add         r1, r0, SPEC_RQ_C_INDEX
    tblmincri   SPEC_RQ_C_INDEX, d.log_num_wqes, 1
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
    
    //CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)
    //CAPRI_SET_FIELD(r4, INFO_OUT1_T, in_progress, 0)
    //CAPRI_SET_FIELD(r4, INFO_OUT1_T, tbl_id, 0) //TODO: set tbl_id
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, cache, d.cache)
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, page_seg_offset, r5)
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, page_offset, r1)
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, remaining_payload_bytes, REM_PYLD_BYTES)
    CAPRI_SET_FIELD(r4, INFO_OUT1_T, inv_r_key, CAPRI_RXDMA_BTH_IETH_R_KEY)

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqpt_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r3)
    nop.e
    nop

duplicate:

    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_ATOMIC_FNA|RESP_RX_FLAG_ATOMIC_CSWAP|RESP_RX_FLAG_READ_REQ)
    bcf             [!c1], duplicate_wr_send
    nop             //BD Slot

duplicate_rd_atomic:

    // recirc if threre is already another duplicate req in progress
    seq             c1, d.adjust_rsq_c_index_in_progress, 1
    bcf             [c1], recirc_wait_for_turn
    nop             //BD Slot

    ARE_ALL_FLAGS_SET(c2, r7, RESP_RX_FLAG_READ_REQ)
    // RETH and ATOMICETH have VA, r_key at same location 
    CAPRI_RXDMA_RETH_VA(r1)
    add             r5, r0, CAPRI_RXDMA_RETH_R_KEY
    cmov            r6, c2, CAPRI_RXDMA_RETH_DMA_LEN, 8

    // since there is no space in stage-to-stage data, using to-stage
    // to populate va/r_key/len so that it is accessible to backtrack_process
    // function which may get called multiple times in successive stages
    // during backtrack process
    CAPRI_GET_STAGE_0_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, va, r1)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, r_key, r5)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, len, r6)

    CAPRI_GET_STAGE_1_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, va, r1)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, r_key, r5)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, len, r6)

    CAPRI_GET_STAGE_2_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, va, r1)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, r_key, r5)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, len, r6)

    CAPRI_GET_STAGE_3_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, va, r1)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, r_key, r5)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, len, r6)

    CAPRI_GET_STAGE_4_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, va, r1)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, r_key, r5)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, len, r6)

    CAPRI_GET_STAGE_5_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, va, r1)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, r_key, r5)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, len, r6)

    CAPRI_GET_STAGE_6_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, va, r1)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, r_key, r5)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, len, r6)

    CAPRI_GET_STAGE_7_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, va, r1)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, r_key, r5)
    CAPRI_SET_FIELD(r4, RSQ_BT_TO_S_INFO_T, len, r6)


    seq         c3, d.rsq_quiesce, 1
    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, search_psn, CAPRI_APP_DATA_BTH_PSN)
    CAPRI_SET_FIELD_C(r4, RSQ_BT_S2S_INFO_T, lo_index, RSQ_P_INDEX_PRIME, c3)
    CAPRI_SET_FIELD_C(r4, RSQ_BT_S2S_INFO_T, lo_index, RSQ_P_INDEX, !c3)
    add         r5, r0, RSQ_P_INDEX
    mincr       r5, d.log_rsq_size, -1
    // in quiesce mode, make sure hi_index is 1 more than RSQ_P_INDEX.
    // since previous mincr has decremented by 1, we are incrementing by 2 here.
    mincr.c3    r5, d.log_rsq_size, +2
    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, hi_index, r5)
    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, index, RSQ_C_INDEX)
    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, rsq_base_addr, d.rsq_base_addr)
    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, log_pmtu, d.log_pmtu)
    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, walk, RSQ_EVAL_MIDDLE)
    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, log_rsq_size, d.log_rsq_size)
    
    cmov        r5, c2, RSQ_OP_TYPE_READ, RSQ_OP_TYPE_ATOMIC 
    CAPRI_SET_FIELD(r4, RSQ_BT_S2S_INFO_T, read_or_atomic, r5)
    
    //load entry at cindex first
    add         r3, d.rsq_base_addr, RSQ_C_INDEX, LOG_SIZEOF_RSQWQE_T
    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rsq_backtrack_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r3)

    
    nop.e
    nop
    

duplicate_wr_send:
    // duplicate write/send, there is nothing to do.
    // release chance to next packet
    tbladd          d.nxt_to_go_token_id, 1  

    // ring the ack_nak ring one more time so that a new ack is pushed out
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK)

    RESP_RX_RING_ACK_NAK_DB(DMA_CMD_BASE, \
                            CAPRI_RXDMA_INTRINSIC_LIF, \
                            CAPRI_RXDMA_INTRINSIC_QTYPE, \
                            CAPRI_RXDMA_INTRINSIC_QID, \
                            DB_ADDR, DB_DATA)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE);
    nop.e
    nop

recirc_wait_for_turn:
    phvwr   p.common.p4_intr_recirc, 1
    //phvwr   p.common.p4_intr_global_drop, 1
    phvwr   p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_INORDER_WORK_NOT_DONE
    nop.e
    nop

nak:
    // If this is UD packet, silently drop
    IS_ANY_FLAG_SET(c2, r7, RESP_RX_FLAG_UD)
    bcf [c2], ud_drop
    nop
    
    add     RQCB1_ADDR, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, 1, LOG_CB_UNIT_SIZE_BYTES
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_ACK)

    RESP_RX_POST_ACK_INFO_TO_TXDMA(DMA_CMD_BASE, RQCB1_ADDR, TMP, \
                                   CAPRI_RXDMA_INTRINSIC_LIF, \
                                   CAPRI_RXDMA_INTRINSIC_QTYPE, \
                                   CAPRI_RXDMA_INTRINSIC_QID, \
                                   DB_ADDR, DB_DATA)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE);

    // release chance to next packet
    tbladd  d.nxt_to_go_token_id, 1  
    nop.e
    nop

ud_drop:

    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop
    

recirc_pkt:

