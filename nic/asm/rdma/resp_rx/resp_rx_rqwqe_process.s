#include "capri.h"
#include "types.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s3_t0_k k;
struct rqwqe_base_t d;

#define INFO_LKEY_T struct resp_rx_key_info_t
#define INFO_WBCB1_P t2_s2s_rqcb1_write_back_info
#define TO_S_RECIRC_P to_s1_recirc_info
#define IN_TO_S_P     to_s3_wqe_info
#define TO_S_WB1_P to_s5_wb1_info

#define SGE_OFFSET_SHIFT 32
#define SGE_OFFSET_MASK 0xffffffff

#define NUM_VALID_SGES  r3
#define SGE_P           r4
#define REM_PYLD_BYTES  r5
#define CURR_SGE_OFFSET r1[31:0]

#define T2_ARG          r6
#define RECIRC_ARG      r6

#define F_FIRST_PASS  c7

#define TMP r3
#define KT_BASE_ADDR r6
#define KEY_ADDR r6

#define IN_P    t0_s2s_rqcb_to_wqe_info
#define K_CURR_WQE_PTR CAPRI_KEY_RANGE(IN_P,curr_wqe_ptr_sbit0_ebit7, curr_wqe_ptr_sbit56_ebit63)
#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, priv_oper_enable)

%%
    .param  resp_rx_rqlkey_process
    .param  resp_rx_inv_rkey_validate_process
    .param  resp_rx_inv_rkey_process
    .param  resp_rx_rqlkey_rsvd_lkey_process
    .param  resp_rx_recirc_mpu_only_process

.align
resp_rx_rqwqe_process:

    // current_sge_id = rqcb_to_wqe_info_p->current_sge_id;
    // current_sge_offset = rqcb_to_wqe_info_p->current_sge_offset; 

    //DANGER: because of register scarcity, encode both
    // current_sge_id and current_sge_offset in r1 and free r2 
    // now r1 = (current_sge_id << 32) + current_sge_offset
    add         r1, CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit7, current_sge_offset_sbit24_ebit31), CAPRI_KEY_FIELD(IN_P, current_sge_id), SGE_OFFSET_SHIFT

    seq         c1, CAPRI_KEY_FIELD(IN_P, in_progress), 1

    //num_valid_sges = (in_progress == TRUE) ? 
    //  rqcb_to_wqe_info_info_p->num_valid_sges ? wqe_p->num_sges;
    cmov        NUM_VALID_SGES, c1, CAPRI_KEY_FIELD(IN_P, num_valid_sges), d.num_sges

    //sge_p = (in_progress == TRUE) ? d_p : (d_p + RQWQE_SGE_OFFSET)
    // we need to add SIZEOF_SGE_T_BITS because SGE is accessed from bottom to top in big-endian
    //big-endian
    cmov        SGE_P, c1, \
                (HBM_CACHE_LINE_SIZE_BITS - (1 << LOG_SIZEOF_SGE_T_BITS)),  \
                (RQWQE_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))

    phvwr.!c1   p.cqe.recv.wrid, d.wrid
    // store wrid into rqcb3
    RQCB3_WRID_ADDR_GET(r6)
    memwr.d.!c1 r6, d.wrid

    cmov        r7, c1, offsetof(struct rqwqe_base_t, rsvd2), offsetof(struct rqwqe_base_t, rsvd)

    add         REM_PYLD_BYTES, r0, CAPRI_KEY_FIELD(IN_P, remaining_payload_bytes)
   
   #CAPRI_RESET_TABLE_0_AND_1_ARG()

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

   #CAPRI_GET_TABLE_0_OR_1_ARG_NO_RESET(resp_rx_phv_t, r7, F_FIRST_PASS)
    // r2 <- sge_p->va
    CAPRI_TABLE_GET_FIELD(r2, SGE_P, SGE_T, va)

    // transfer_va = sge_p->va + current_sge_offset;
    add         r2, r2, CURR_SGE_OFFSET
    // sge_to_lkey_info_p->sge_va = transfer_va;
    CAPRI_SET_TABLE_FIELD_LOCAL(r7, INFO_LKEY_T, va, r2)
    // sge_to_lkey_info_p->sge_bytes = transfer_bytes;
    CAPRI_SET_TABLE_FIELD_LOCAL(r7, INFO_LKEY_T, len, r6)
    // sge_to_lkey_info_p->dma_cmd_start_index = dma_cmd_index;
    add         r2, r0, CAPRI_KEY_FIELD(IN_P, dma_cmd_index)
    add.!F_FIRST_PASS r2, r2, MAX_PYLD_DMA_CMDS_PER_SGE
    //cmov        r2, F_FIRST_PASS, RESP_RX_DMA_CMD_PYLD_BASE, (RESP_RX_DMA_CMD_PYLD_BASE + MAX_PYLD_DMA_CMDS_PER_SGE)
    CAPRI_SET_TABLE_FIELD_LOCAL(r7, INFO_LKEY_T, dma_cmd_start_index, r2)
    //sge_to_lkey_info_p->sge_index = index;
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, tbl_id, 1, !F_FIRST_PASS)

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

    KT_BASE_ADDR_GET2(r6, r2)

    // r2 <- sge_p->l_key
    CAPRI_TABLE_GET_FIELD(r2, SGE_P, SGE_T, l_key)

    crestore       [c6], K_PRIV_OPER_ENABLE, 0x1
    seq.c6         c6, r2, RDMA_RESERVED_LKEY_ID

    // DANGER: Do not move the instruction above.
    // tblrdp above should be reading l_key from old sge_p
    //sge_p++;
    sub.c2      SGE_P, SGE_P, 1, LOG_SIZEOF_SGE_T_BITS

    KEY_ENTRY_ADDR_GET(r6, r6, r2)
    // now r6 has key_addr

    // Initiate next table lookup with 32 byte Key address (so avoid whether keyid 0 or 1)

    CAPRI_GET_TABLE_0_OR_1_K_NO_VALID(resp_rx_phv_t, r2, F_FIRST_PASS)
    CAPRI_NEXT_TABLE_I_READ_PC_C(r2, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqlkey_rsvd_lkey_process, resp_rx_rqlkey_process, r6, c6)

    // are remaining_payload_bytes 0 ?
    seq         c5, REM_PYLD_BYTES, 0

    // are num_valid_sges <= 1 ?
    sle         c6, NUM_VALID_SGES, 1

    // set dma_cmdeop for the last table (could be T0 or T1)
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, dma_cmdeop, 1, c5)
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, invoke_writeback, 1, c5)
    CAPRI_SET_TABLE_FIELD_LOCAL(r7, INFO_LKEY_T, current_sge_offset, CURR_SGE_OFFSET)

    .csbegin
    cswitch [F_FIRST_PASS,c1]
    nop

    //!first_pass, !in_progress
    //write from d.rsvd to p.common.common_t1_s2s_s2s_data
    .cscase 0
        b       write_done
        phvwr  p.common.common_t1_s2s_s2s_data, d.rsvd[sizeof(INFO_LKEY_T):0] //BD Slot

    //!first_pass, in_progress
    //write from d.rsvd2 to p.common.common_t1_s2s_s2s_data
    .cscase 1
        b       write_done
        phvwr  p.common.common_t1_s2s_s2s_data, d.rsvd2[sizeof(INFO_LKEY_T):0] //BD Slot

    //first_pass, !in_progress
    //write from d.rsvd to p.common.common_t0_s2s_s2s_data
    .cscase 2
        b       write_done
        phvwr  p.common.common_t0_s2s_s2s_data, d.rsvd[sizeof(INFO_LKEY_T):0] //BD Slot

    //!first_pass, in_progress
    //write from d.rsvd2 to p.common.common_t0_s2s_s2s_data
    .cscase 3
        b       write_done
        phvwr  p.common.common_t0_s2s_s2s_data, d.rsvd2[sizeof(INFO_LKEY_T):0] //BD Slot

   .csend
    
write_done:

    // loop one more time ONLY if:
    // remaining_payload_bytes > 0 AND
    // did only one pass AND
    // still there are valid sges to process
    setcf       c4, [!c5 & F_FIRST_PASS & !c6]
    bcf         [c4], loop
    // make F_FIRST_PASS = FALSE only when we are going for second pass
    setcf.c4    F_FIRST_PASS, [!c0] // BD Slot

loop_exit:

    bcf         [!c5], skip_inv_rkey
    seq         c2, K_GLOBAL_FLAG(_inv_rkey), 1 // BD Slot

    .csbegin
    cswitch     [c2]
    nop

    .cscase 0

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_inv_rkey_validate_process, r0)
    b           inv_rkey_done
    nop // BD Slot

    .cscase 1

    // if invalidate rkey is present, invoke it by loading appopriate
    // key entry, else load the same program as MPU only.
    KT_BASE_ADDR_GET2(KT_BASE_ADDR, TMP)
    add         TMP, r0, CAPRI_KEY_FIELD(IN_TO_S_P, inv_r_key)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, TMP)

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_256_BITS, resp_rx_inv_rkey_validate_process, KEY_ADDR)
    b           inv_rkey_done
    nop // BD Slot

    .csend

inv_rkey_done:
skip_inv_rkey:
    CAPRI_SET_TABLE_0_VALID(1)
    CAPRI_SET_TABLE_1_VALID_C(!F_FIRST_PASS, 1)

    add         r7, r0, K_GLOBAL_FLAGS

    ARE_ALL_FLAGS_SET(c2, r7, RESP_RX_FLAG_UD|RESP_RX_FLAG_IMMDT)
    phvwr.c2    p.cqe.recv.smac[31:0], CAPRI_KEY_FIELD(IN_TO_S_P, ext_hdr_data)

    IS_ANY_FLAG_SET(c2, r7, RESP_RX_FLAG_FIRST)
    seq         c3, CAPRI_KEY_FIELD(IN_P, recirc_path), 1
    bcf         [!c2 | c3], non_first_or_recirc_pkt
    // pass the rkey to write back, since wb calls inv_rkey. Note that this s2s across multiple(two, 3 to 5) stages
    phvwr.!c3   CAPRI_PHV_FIELD(INFO_WBCB1_P, inv_r_key), CAPRI_KEY_FIELD(IN_TO_S_P, inv_r_key) //BD Slot

    // only first packet need to set num_sges and wqe_ptr values into
    // rqcb1. middle/last packets will simply use these fields from cb
    phvwrpair.c2    CAPRI_PHV_RANGE(TO_S_WB1_P, update_wqe_ptr, update_num_sges), \
                    3, \
                    CAPRI_PHV_FIELD(TO_S_WB1_P, num_sges), \
                    NUM_VALID_SGES

non_first_or_recirc_pkt:

    // if remaining payload bytes are not zero, recirc
    // to process more sges
    bcf         [!c5],  recirc
    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY) //BD Slot

    .csbegin
    cswitch     [c1]
    nop

    .cscase 0

    // in_progress, incr_nxt_to_go_token_id, incr_c_index
    phvwrpair.e   CAPRI_PHV_FIELD(TO_S_WB1_P, curr_wqe_ptr), \
                K_CURR_WQE_PTR, \
                CAPRI_PHV_RANGE(TO_S_WB1_P, in_progress, incr_c_index), \
                (1<<2 | 1<<1 | 0)

    CAPRI_SET_FIELD2(TO_S_WB1_P, current_sge_id, r1[63:32]) //Exit Slot

    .cscase 1
    
    // incr_nxt_to_go_token_id, incr_c_index
    phvwrpair.e   CAPRI_PHV_FIELD(TO_S_WB1_P, curr_wqe_ptr), \
                K_CURR_WQE_PTR, \
                CAPRI_PHV_RANGE(TO_S_WB1_P, incr_nxt_to_go_token_id, incr_c_index), \
                (1<<1 | 1) 
    nop // Exit Slot
                

    .csend

recirc:
    //TODO: check if there are anymore sges left.
    //      if not, generate NAK

    // we are recircing means, we would have consumed 2 lkey's worth of DMA commands
    add     r2, CAPRI_KEY_FIELD(IN_P, dma_cmd_index), (MAX_PYLD_DMA_CMDS_PER_SGE * 2)

    // if we reached to the max number of pyld DMA commands and still
    // pkt transfer is not complete, generate NAK 
    seq     c1, r2, RESP_RX_DMA_CMD_PYLD_BASE_END
    bcf     [c1], nak

    // store recirc info so that stage 1 program upon recirculation
    // can access this info
    CAPRI_SET_FIELD2_C(TO_S_RECIRC_P, curr_wqe_ptr, K_CURR_WQE_PTR, !c1)
    CAPRI_SET_FIELD2(TO_S_RECIRC_P, current_sge_id, r1[63:32])
    CAPRI_SET_FIELD2(TO_S_RECIRC_P, current_sge_offset, CURR_SGE_OFFSET)
    phvwrpair   CAPRI_PHV_FIELD(TO_S_RECIRC_P, remaining_payload_bytes), REM_PYLD_BYTES, \
                CAPRI_PHV_FIELD(TO_S_RECIRC_P, num_sges), NUM_VALID_SGES
    
    // fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_recirc_mpu_only_process, r0)

    // set recirc
    phvwr.e p.common.p4_intr_recirc, 1
    phvwr   p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_SGE_WORK_PENDING //Exit Slot

nak:
    //TODO: generate nak. for now, drop
    phvwr.e   p.common.p4_intr_global_drop, 1
    CAPRI_SET_ALL_TABLES_VALID(0)  //Exit Slot
