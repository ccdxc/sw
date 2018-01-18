#include "capri.h"
#include "types.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqwqe_process_k_t k;
struct rqwqe_base_t d;

#define INFO_LKEY_T struct resp_rx_key_info_t
#define INFO_WBCB0_T struct resp_rx_rqcb0_write_back_info_t
#define TO_S_RECIRC_T struct resp_rx_to_stage_recirc_info_t
#define TO_S_WB1_T struct resp_rx_to_stage_wb1_info_t


#define SGE_OFFSET_SHIFT 32
#define SGE_OFFSET_MASK 0xffffffff

#define NUM_VALID_SGES  r3
#define SGE_P           r4
#define REM_PYLD_BYTES  r5
#define CURR_SGE_OFFSET r1[31:0]

#define T2_ARG          r5
#define T2_K            r5
#define RECIRC_ARG      r6
#define WB1_ARG         r6

#define F_FIRST_PASS  c7

%%
    .param  resp_rx_rqlkey_process
    .param  resp_rx_rqcb0_write_back_process

.align
resp_rx_rqwqe_process:

    // current_sge_id = rqcb_to_wqe_info_p->current_sge_id;
    // current_sge_offset = rqcb_to_wqe_info_p->current_sge_offset; 

    //DANGER: because of register scarcity, encode both
    // current_sge_id and current_sge_offset in r1 and free r2 
    // now r1 = (current_sge_id << 32) + current_sge_offset
    add         r1, k.args.current_sge_offset, k.args.current_sge_id, SGE_OFFSET_SHIFT

    seq         c1, k.args.in_progress, 1

    //num_valid_sges = (in_progress == TRUE) ? 
    //  rqcb_to_wqe_info_info_p->num_valid_sges ? wqe_p->num_sges;
    cmov        NUM_VALID_SGES, c1, k.args.num_valid_sges, d.num_sges

    //sge_p = (in_progress == TRUE) ? d_p : (d_p + RQWQE_SGE_OFFSET)
    // we need to add SIZEOF_SGE_T_BITS because SGE is accessed from bottom to top in big-endian
    //big-endian
    cmov        SGE_P, c1, \
                (HBM_CACHE_LINE_SIZE_BITS - (1 << LOG_SIZEOF_SGE_T_BITS)),  \
                (RQWQE_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))

    //phv_p->cqwqe.id.wrid = wqe_p->wrid;
    phvwr.!c1   p.cqwqe.id.wrid, d.wrid
    
    // rqcb1_p->wrid = wqe_p->wrid
    //TODO: make sure wrid is at byte boundary so that below divison works
    // wrid address needs some work as offset does not directly gives. 
    RQCB1_WRID_ADDR_GET(r6)
    //TODO: change to DMA
    memwr.d.!c1 r6, d.wrid

    add         REM_PYLD_BYTES, r0, k.args.remaining_payload_bytes
   
    CAPRI_RESET_TABLE_0_AND_1_ARG(resp_rx_phv_t)

    // first_pass = TRUE
    setcf       F_FIRST_PASS, [c0]
loop:
    // r6 <- sge_p->len
    CAPRI_TABLE_GET_FIELD(r6, SGE_P, SGE_T, len)

    //sge_remaining_bytes = sge_p->len - current_sge_offset;
    sub         r6, r6, CURR_SGE_OFFSET

    //transfer_bytes = min(sge_remaining_bytes, remaining_payload_bytes);
    slt         c2, r6, REM_PYLD_BYTES
    cmov        r6, c2, r6, REM_PYLD_BYTES

    CAPRI_GET_TABLE_0_OR_1_ARG_NO_RESET(resp_rx_phv_t, r7, F_FIRST_PASS)
    // r2 <- sge_p->va
    CAPRI_TABLE_GET_FIELD(r2, SGE_P, SGE_T, va)

    // transfer_va = sge_p->va + current_sge_offset;
    add         r2, r2, CURR_SGE_OFFSET
    // sge_to_lkey_info_p->sge_va = transfer_va;
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, va, r2)
    // sge_to_lkey_info_p->sge_bytes = transfer_bytes;
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, len, r6)
    // sge_to_lkey_info_p->dma_cmd_start_index = dma_cmd_index;
    add         r2, r0, k.args.dma_cmd_index
    add.!F_FIRST_PASS r2, r2, MAX_PYLD_DMA_CMDS_PER_SGE
    //cmov        r2, F_FIRST_PASS, RESP_RX_DMA_CMD_PYLD_BASE, (RESP_RX_DMA_CMD_PYLD_BASE + MAX_PYLD_DMA_CMDS_PER_SGE)
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, dma_cmd_start_index, r2)
    //sge_to_lkey_info_p->sge_index = index;
    CAPRI_SET_FIELD_C(r7, INFO_LKEY_T, tbl_id, 1, !F_FIRST_PASS)
    //CAPRI_SET_FIELD(r7, INFO_LKEY_T, dma_cmdeop, 0)
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, acc_ctrl, ACC_CTRL_LOCAL_WRITE)
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, nak_code, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_OP_ERR))

    //remaining_payload_bytes -= transfer_bytes;
    sub         REM_PYLD_BYTES, REM_PYLD_BYTES, r6
    //current_sge_offset += transfer_bytes;
    add         r2, CURR_SGE_OFFSET, r6
    // shift right and then shift left to clear bottom 32 bits
    add         r1, r2[31:0], r1[63:32], 32

    //  r6 <- sge_p->len
    CAPRI_TABLE_GET_FIELD(r6, SGE_P, SGE_T, len)

    // if (current_sge_offset == sge_p->len) {
    seq         c2, r6, r2[31:0]
    //current_sge_id++;
    add.c2      r1, r1[63:32], 1
    //current_sge_offset = 0;
    sll.c2      r1, r1, SGE_OFFSET_SHIFT

    KT_BASE_ADDR_GET(r6, r2)

    // r2 <- sge_p->l_key
    CAPRI_TABLE_GET_FIELD(r2, SGE_P, SGE_T, l_key)

    // DANGER: Do not move the instruction above.
    // tblrdp above should be reading l_key from old sge_p
    //sge_p++;
    sub.c2      SGE_P, SGE_P, 1, LOG_SIZEOF_SGE_T_BITS

    KEY_ENTRY_ADDR_GET(r6, r6, r2)
    // now r6 has key_addr

    // Initiate next table lookup with 32 byte Key address (so avoid whether keyid 0 or 1)

    CAPRI_GET_TABLE_0_OR_1_K_NO_VALID(resp_rx_phv_t, r2, F_FIRST_PASS)
    CAPRI_NEXT_TABLE_I_READ_PC(r2, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_rx_rqlkey_process, r6)

    // are remaining_payload_bytes 0 ?
    seq         c5, REM_PYLD_BYTES, 0

    // are num_valid_sges <= 1 ?
    sle         c6, NUM_VALID_SGES, 1

    // loop one more time ONLY if:
    // remaining_payload_bytes > 0 AND
    // did only one pass AND
    // still there are valid sges to process
    setcf       c4, [!c5 & F_FIRST_PASS & !c6]
    bcf         [c4], loop
    // make F_FIRST_PASS = FALSE only when we are going for second pass
    setcf.c4    F_FIRST_PASS, [!c0] // BD Slot
    
exit:

    CAPRI_SET_TABLE_0_VALID(1)
    CAPRI_SET_TABLE_1_VALID_C(!F_FIRST_PASS, 1)

    // set dma_cmdeop for the last table (could be T0 or T1)
    CAPRI_SET_FIELD_C(r7, INFO_LKEY_T, dma_cmdeop, 1, c5)

    add         r7, r0, k.global.flags

    ARE_ALL_FLAGS_SET(c2, r7, RESP_RX_FLAG_UD|RESP_RX_FLAG_IMMDT)
    phvwr.c2    p.cqwqe.smac[31:0], k.{to_stage.s2.ext_hdr.ext_hdr_data[95:64]}

    IS_ANY_FLAG_SET(c2, r7, RESP_RX_FLAG_FIRST)
    seq         c3, k.args.recirc_path, 1
    bcf         [!c2 | c3], non_first_or_recirc_pkt
    CAPRI_GET_STAGE_4_ARG(resp_rx_phv_t, WB1_ARG) //BD Slot

    // only first packet need to set num_sges and wqe_ptr values into
    // rqcb1. middle/last packets will simply use these fields from cb
    //CAPRI_SET_FIELD_C(WB1_ARG, TO_S_WB1_T, update_num_sges, 1, c2)    
    //CAPRI_SET_FIELD_C(WB1_ARG, TO_S_WB1_T, update_wqe_ptr, 1, c2)
    CAPRI_SET_FIELD_RANGE_C(WB1_ARG, TO_S_WB1_T, update_wqe_ptr, update_num_sges, 3, c2)
    CAPRI_SET_FIELD_C(WB1_ARG, TO_S_WB1_T, num_sges, NUM_VALID_SGES, c2)

non_first_or_recirc_pkt:

    // if remaining payload bytes are not zero, recirc
    // to process more sges
    bcf         [!c5],  recirc
    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY) //BD Slot
    CAPRI_GET_TABLE_2_ARG(resp_rx_phv_t, T2_ARG)

    .csbegin
    cswitch     [c1]
    nop

    .cscase 0

    //CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, in_progress, 1)
    //CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD_RANGE(T2_ARG, INFO_WBCB0_T, in_progress, incr_nxt_to_go_token_id, 3)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, cache, k.args.cache)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, tbl_id, TABLE_2)

    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, current_sge_id, r1[63:32])

    b       cb0_cb1_wb_exit
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, current_sge_offset, CURR_SGE_OFFSET) //BD Slot

    .cscase 1
    
    b       cb0_cb1_wb_exit
    // incr_nxt_to_go_token_id, incr_c_index, tbl_id
    CAPRI_SET_FIELD_RANGE(T2_ARG, INFO_WBCB0_T, incr_nxt_to_go_token_id, tbl_id, (1<<4 | 1<<3 | TABLE_2)) //BD Slot
    .csend

cb0_cb1_wb_exit:
    
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, curr_wqe_ptr, k.args.curr_wqe_ptr)
    // Current program is going to spawn 4 parallel lookups for next stage.
    // They are: T0-Lkey0, T1-Lkey1, T2-WB0, T3-WB1. 
    // T2 and T3 programs would reset their table valid bits upon completing
    // their program. Where as, either T0 or T1 program could spawn completion
    // queue related program on T2 for next to next stage. T2-WB0 invalidating
    // T2 and T0/T1-Lkey program invoking T2 could conflict with each other 
    // and there by completion queue lookup may not fire.
    // Hence putting a hack here to pass a clue to T2-WB0 program NOT to 
    // invalidate T2 in case completion is involved.

    //IS_ANY_FLAG_SET(c3, r7, RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_COMPLETION)
    //CAPRI_SET_FIELD_C(T2_ARG, INFO_WBCB0_T, do_not_invalidate_tbl, 1, c3)

    RQCB0_ADDR_GET(r2)
    CAPRI_GET_TABLE_2_K(resp_rx_phv_t, T2_K)
    CAPRI_NEXT_TABLE_I_READ_PC(T2_K, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqcb0_write_back_process, r2)

    nop.e
    nop


recirc:
    //TODO: check if there are anymore sges left.
    //      if not, generate NAK

    // we are recircing means, we would have consumed 2 lkey's worth of DMA commands
    add     r2, k.args.dma_cmd_index, (MAX_PYLD_DMA_CMDS_PER_SGE * 2)

    // if we reached to the max number of pyld DMA commands and still
    // pkt transfer is not complete, generate NAK 
    seq     c1, r2, RESP_RX_DMA_CMD_PYLD_BASE_END
    bcf     [c1], nak
    CAPRI_GET_STAGE_1_ARG(resp_rx_phv_t, RECIRC_ARG) //BD Slot

    // store recirc info so that stage 1 program upon recirculation
    // can access this info
    
    CAPRI_SET_FIELD(RECIRC_ARG, TO_S_RECIRC_T, dma_cmd_index, r2)
    CAPRI_SET_FIELD(RECIRC_ARG, TO_S_RECIRC_T, curr_wqe_ptr, k.args.curr_wqe_ptr)
    CAPRI_SET_FIELD(RECIRC_ARG, TO_S_RECIRC_T, current_sge_id, r1[63:32])
    CAPRI_SET_FIELD(RECIRC_ARG, TO_S_RECIRC_T, current_sge_offset, CURR_SGE_OFFSET)
    CAPRI_SET_FIELD(RECIRC_ARG, TO_S_RECIRC_T, num_sges, NUM_VALID_SGES)
    CAPRI_SET_FIELD(RECIRC_ARG, TO_S_RECIRC_T, remaining_payload_bytes, REM_PYLD_BYTES)
    
    // set recirc
    phvwr.e p.common.p4_intr_recirc, 1
    phvwr   p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_SGE_WORK_PENDING //Exit Slot

nak:
    //TODO: generate nak. for now, drop
    phvwr.e   p.common.p4_intr_global_drop, 1
    CAPRI_SET_ALL_TABLES_VALID(0)  //Exit Slot
