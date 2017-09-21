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
#define INFO_WBCB1_T struct resp_rx_rqcb1_write_back_info_t    


#define SGE_OFFSET_SHIFT 32
#define SGE_OFFSET_MASK 0xffffffff

#define NUM_VALID_SGES  r3
#define SGE_P           r4
#define REM_PYLD_BYTES  r5
#define CURR_SGE_OFFSET r1[31:0]

#define T2_ARG          r5
#define T3_ARG          r6
#define T2_K            r5
#define T3_K            r6

#define RAW_TABLE_PC    r2
#define RAW_TABLE_PC2   r1

#define F_FIRST_PASS  c7

#define SEL_T0_OR_T1_S2S_DATA(_dst_r, _cf) \
    cmov        _dst_r, _cf, offsetof(struct resp_rx_phv_t, common.common_t0_s2s_s2s_data), offsetof(struct resp_rx_phv_t, common.common_t1_s2s_s2s_data);
#define SEL_T0_OR_T1_K(_dst_r, _cf) \
    cmov        _dst_r, _cf, offsetof(struct resp_rx_phv_t, common.common_te0_phv_table_addr), offsetof(struct resp_rx_phv_t, common.common_te1_phv_table_addr);

%%
    .param  resp_rx_rqlkey_process
    .param  resp_rx_rqcb0_write_back_process
    .param  resp_rx_rqcb1_write_back_process

.align
resp_rx_rqwqe_process:

    //MPU GLOBAL
    RQCB1_ADDR_GET(r7)

    // current_sge_id = rqcb_to_wqe_info_p->current_sge_id;
    // current_sge_offset = rqcb_to_wqe_info_p->current_sge_offset; 
    add         r1, r0, k.args.current_sge_id
    add         r2, r0, k.args.current_sge_offset

    //DANGER: because of register scarcity, encode both
    // current_sge_id and current_sge_offset in r1 and free r2 
    sll         r1, r1, SGE_OFFSET_SHIFT
    add         r1, r1, r2
    // now r1 = (current_sge_id << 32) + current_sge_offset

    seq         c1, k.args.in_progress, 1

    //num_valid_sges = (in_progress == TRUE) ? 
    //  rqcb_to_wqe_info_info_p->num_valid_sges ? wqe_p->num_sges;
    cmov        NUM_VALID_SGES, c1, k.args.num_valid_sges, d.num_sges

    //sge_p = (in_progress == TRUE) ? d_p : (d_p + RQWQE_SGE_OFFSET)
    //big-endian
    cmov        SGE_P, c1, HBM_CACHE_LINE_SIZE_BITS, RQWQE_SGE_OFFSET_BITS
    // we need to add SIZEOF_SGE_T_BITS because SGE is accessed from bottom to top in big-endian
    //big-endian
    sub         SGE_P, SGE_P, 1, LOG_SIZEOF_SGE_T_BITS

    //phv_p->cqwqe.id.wrid = wqe_p->wrid;
    phvwr.!c1   p.cqwqe.id.wrid, d.wrid
    
    // rqcb1_p->wrid = wqe_p->wrid
    //TODO: make sure wrid is at byte boundary so that below divison works
    add         r6, r7, BYTE_OFFSETOF(rqcb1_t, wrid)
    //TODO: change to DMA
    memwr.d.!c1 r6, d.wrid

    add         REM_PYLD_BYTES, r0, k.args.remaining_payload_bytes
   
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

    SEL_T0_OR_T1_S2S_DATA(r7, F_FIRST_PASS)
    // r2 <- sge_p->va
    CAPRI_TABLE_GET_FIELD(r2, SGE_P, SGE_T, va)

    // transfer_va = sge_p->va + current_sge_offset;
    add         r2, r2, CURR_SGE_OFFSET
    // sge_to_lkey_info_p->sge_va = transfer_va;
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, va, r2)
    // sge_to_lkey_info_p->sge_bytes = transfer_bytes;
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, len, r6)
    // sge_to_lkey_info_p->dma_cmd_start_index = dma_cmd_index;
    add         r2, r0, RESP_RX_DMA_CMD_PYLD_BASE
    add.!F_FIRST_PASS   r2, r2, MAX_PYLD_DMA_CMDS_PER_SGE
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, dma_cmd_start_index, r2)
    //sge_to_lkey_info_p->sge_index = index;
    cmov        r2, F_FIRST_PASS, 0, 1
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, tbl_id, r2)
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, dma_cmdeop, 0)
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, acc_ctrl, ACC_CTRL_LOCAL_WRITE)



    //remaining_payload_bytes -= transfer_bytes;
    sub         REM_PYLD_BYTES, REM_PYLD_BYTES, r6
    //current_sge_offset += transfer_bytes;
    add         r2, CURR_SGE_OFFSET, r6
    // shift right and then shift left to clear bottom 32 bits
    srl         r1, r1, SGE_OFFSET_SHIFT
    sll         r1, r1, SGE_OFFSET_SHIFT
    // now or it with new value
    or          r1, r1, r2[31:0]

    //  r6 <- sge_p->len
    CAPRI_TABLE_GET_FIELD(r6, SGE_P, SGE_T, len)

    // if (current_sge_offset == sge_p->len) {
    seq         c2, r6, r2[31:0]
    //current_sge_id++;
    srl.c2      r1, r1, SGE_OFFSET_SHIFT
    add.c2      r1, r1, 1
    //current_sge_offset = 0;
    sll.c2      r1, r1, SGE_OFFSET_SHIFT

    KT_BASE_ADDR_GET(r6, r2)

    // r2 <- sge_p->l_key
    CAPRI_TABLE_GET_FIELD(r2, SGE_P, SGE_T, l_key)

    // DANGER: Do not move the instruction above.
    // tblrdp above should be reading l_key from old sge_p
    //sge_p++;
    sub.c2      SGE_P, SGE_P, 1, LOG_SIZEOF_SGE_T_BITS

    //key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET()) +
    // ((sge_p->l_key & KEY_INDEX_MASK) * sizeof(key_entry_t));
    andi        r2, r2, KEY_INDEX_MASK
    sll         r2, r2, LOG_SIZEOF_KEY_ENTRY_T

    add         r2, r2, r6
    // now r2 has key_addr

    //aligned_key_addr = key_addr & ~HBM_CACHE_LINE_MASK;
    and         r6, r2, HBM_CACHE_LINE_SIZE_MASK
    sub         r6, r2, r6
    // r6 now has aligned_key_addr

    //key_id = (key_addr % HBM_CACHE_LINE_SIZE) / sizeof(key_entry_t);
    // compute (key_addr - aligned_key_addr) >> log_key_entry_t
    sub         r2, r2, r6
    srl         r2, r2, LOG_SIZEOF_KEY_ENTRY_T
    // r2 now has key_id
    
    CAPRI_SET_FIELD(r7, INFO_LKEY_T, key_id, r2)

    SEL_T0_OR_T1_K(r7, F_FIRST_PASS)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqlkey_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, r6)
    CAPRI_SET_TABLE_0_VALID_C(F_FIRST_PASS, 1)
    CAPRI_SET_TABLE_1_VALID_C(!F_FIRST_PASS, 1)

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

    SEL_T0_OR_T1_S2S_DATA(r7, F_FIRST_PASS)
    // set dma_cmdeop for the last table (could be T0 or T1)
    CAPRI_SET_FIELD_C(r7, INFO_LKEY_T, dma_cmdeop, 1, c5)

    add         r7, r0, k.global.flags
    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY)

    CAPRI_GET_TABLE_2_ARG(resp_rx_phv_t, T2_ARG)
    CAPRI_GET_TABLE_3_ARG(resp_rx_phv_t, T3_ARG)

    .csbegin
    cswitch     [c1]
    nop

    .cscase 0

    // if (RESP_RX_FLAGS_IS_SET(first)) {
    //      assert(rqcb_to_wqe_info_p->in_progress == FALSE);
    //      rqcb1_write_back_info_p->update_num_sges = 1;
    //      rqcb1_write_back_info_p->num_sges =
    //                                      num_valid_sges;
    //                              //coming from wqe_p
    //  } else {
    //      assert(rqcb_to_wqe_info_p->in_progress == TRUE);
    //      rqcb1_write_back_info_p->update_num_sges = 0;
    //  }


    IS_ANY_FLAG_SET(c2, r7, RESP_RX_FLAG_FIRST)
    CAPRI_SET_FIELD_C(T3_ARG, INFO_WBCB1_T, update_num_sges, 1, c2)
    CAPRI_SET_FIELD_C(T3_ARG, INFO_WBCB1_T, num_sges, NUM_VALID_SGES, c2)
    CAPRI_SET_FIELD_C(T3_ARG, INFO_WBCB1_T, update_num_sges, 0, !c2)

    // middle/first
    //  rqcb_write_back_info_p->in_progress = TRUE;
    //  rqcb_write_back_info_p->incr_nxt_to_go_token_id = 1;
    //  rqcb_write_back_info_p->incr_c_index = 0;
    //  rqcb_write_back_info_p->cache = rqcb_to_wqe_info_p->cache;
    //  rqcb_write_back_info_p->tbl_id = index;
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, in_progress, 1)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, incr_c_index, 0)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, cache, k.args.cache)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, tbl_id, TABLE_2)

    //  rqcb1_write_back_info_p->current_sge_id = current_sge_id;
    //  rqcb1_write_back_info_p->current_sge_offset = current_sge_offset;
    //  rqcb1_write_back_info_p->curr_wqe_ptr = rqcb_to_wqe_info_p->curr_wqe_ptr;
    srl         r4, r1, SGE_OFFSET_SHIFT
    CAPRI_SET_FIELD(T3_ARG, INFO_WBCB1_T, current_sge_id, r4)
    CAPRI_SET_FIELD(T3_ARG, INFO_WBCB1_T, current_sge_offset, CURR_SGE_OFFSET)
    CAPRI_SET_FIELD(T3_ARG, INFO_WBCB1_T, curr_wqe_ptr, k.args.curr_wqe_ptr)

    b       cb0_cb1_wb_exit
    nop

    .cscase 1
    
    //  rqcb_write_back_info_p->in_progress = FALSE;
    //  rqcb_write_back_info_p->incr_nxt_to_go_token_id = 1;
    //  rqcb_write_back_info_p->incr_c_index = 1;
    //  rqcb_write_back_info_p->cache = rqcb_to_wqe_info_p->cache;
    //  rqcb_write_back_info_p->tbl_id = index;
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, in_progress, 0)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, incr_c_index, 1)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, cache, k.args.cache)
    CAPRI_SET_FIELD(T2_ARG, INFO_WBCB0_T, tbl_id, TABLE_2)

    //  rqcb1_write_back_info_p->current_sge_id = 0;
    //  rqcb1_write_back_info_p->current_sge_offset = 0;
    //  rqcb1_write_back_info_p->curr_wqe_ptr = rqcb_to_wqe_info_p->curr_wqe_ptr;
    //  rqcb1_write_back_info_p->num_sges = 0;
    //  rqcb1_write_back_info_p->update_num_sges = 1;
    CAPRI_SET_FIELD(T3_ARG, INFO_WBCB1_T, current_sge_id, 0)
    CAPRI_SET_FIELD(T3_ARG, INFO_WBCB1_T, current_sge_offset, 0)
    CAPRI_SET_FIELD(T3_ARG, INFO_WBCB1_T, curr_wqe_ptr, k.args.curr_wqe_ptr)
    CAPRI_SET_FIELD(T3_ARG, INFO_WBCB1_T, num_sges, 0)
    CAPRI_SET_FIELD(T3_ARG, INFO_WBCB1_T, update_num_sges, 0)

    b       cb0_cb1_wb_exit
    nop
    .csend

cb0_cb1_wb_exit:
    
    // Current program is going to spawn 4 parallel lookups for next stage.
    // They are: T0-Lkey0, T1-Lkey1, T2-WB0, T3-WB1. 
    // T2 and T3 programs would reset their table valid bits upon completing
    // their program. Where as, either T0 or T1 program could spawn completion
    // queue related program on T2 for next to next stage. T2-WB0 invalidating
    // T2 and T0/T1-Lkey program invoking T2 could conflict with each other 
    // and there by completion queue lookup may not fire.
    // Hence putting a hack here to pass a clue to T2-WB0 program NOT to 
    // invalidate T2 in case completion is involved.

    IS_ANY_FLAG_SET(c3, r7, RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_COMPLETION)
    CAPRI_SET_FIELD_C(T2_ARG, INFO_WBCB0_T, do_not_invalidate_tbl, 1, c3)

    RQCB0_ADDR_GET(r2)
    CAPRI_GET_TABLE_2_K(resp_rx_phv_t, T2_K)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC2, resp_rx_rqcb0_write_back_process)
    CAPRI_NEXT_TABLE_I_READ(T2_K, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC2, r2)

    RQCB1_ADDR_GET(r2)
    CAPRI_GET_TABLE_3_K(resp_rx_phv_t, T3_K)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC2, resp_rx_rqcb1_write_back_process)
    CAPRI_NEXT_TABLE_I_READ(T3_K, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC2, r2)

    //TODO:
    //if ((RESP_RX_FLAGS_IS_SET(last)) || (RESP_RX_FLAGS_IS_SET(only))) {
    //  rx_post_ack_info_to_txdma(phv_p, dma_cmd_index, RESP_RX_GET_RQCB1_ADDR());
    //

    nop.e
    nop
