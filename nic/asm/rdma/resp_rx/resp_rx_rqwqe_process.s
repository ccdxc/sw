#include "capri.h"
#include "types.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"

struct resp_rx_phv_t p;
struct resp_rx_s2_t0_k k;
struct rqwqe_base_t d;

#define INFO_LKEY_T struct resp_rx_key_info_t
#define INFO_WBCB1_P t2_s2s_rqcb1_write_back_info
#define TO_S_RECIRC_P to_s1_recirc_info
#define IN_TO_S_P     to_s2_wqe_info
#define TO_S_WB1_P to_s5_wb1_info
#define TO_S_STATS_INFO_P to_s7_stats_info
#define TO_S_INV_RKEY_P to_s4_lkey_info

#define SGE_OFFSET_SHIFT 32
#define SGE_OFFSET_MASK 0xffffffff

#define NUM_VALID_SGES  r3
#define SGE_P           r4
#define REM_PYLD_BYTES  r5
#define CURR_SGE_OFFSET r1[31:0]

#define F_FIRST_PASS  c7

#define TMP r3
#define KT_BASE_ADDR r6
#define KEY_ADDR r6
#define DMA_CMD_BASE r6
#define GLOBAL_FLAGS r6
#define UD_TMP r2

#define IN_P    t0_s2s_rqcb_to_wqe_info
#define IN_TO_S_P    to_s2_wqe_info
#define K_CURR_WQE_PTR CAPRI_KEY_RANGE(IN_P,curr_wqe_ptr_sbit0_ebit7, curr_wqe_ptr_sbit56_ebit63)
#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, priv_oper_enable)
#define K_EXT_HDR_DATA CAPRI_KEY_RANGE(IN_TO_S_P, ext_hdr_data_sbit0_ebit63, ext_hdr_data_sbit64_ebit68)
#define K_INV_R_KEY CAPRI_KEY_RANGE(IN_TO_S_P, inv_r_key_sbit0_ebit2, inv_r_key_sbit27_ebit31)
#define K_REM_PYLD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15)

%%
    .param  resp_rx_rqlkey_process
    .param  resp_rx_inv_rkey_validate_process
    .param  resp_rx_rqlkey_rsvd_lkey_process
    .param  resp_rx_recirc_mpu_only_process
    .param  resp_rx_rqcb1_write_back_mpu_only_process

.align
resp_rx_rqwqe_process:
    bcf             [c2 | c3 | c7], table_error
    // current_sge_id = rqcb_to_wqe_info_p->current_sge_id;
    // current_sge_offset = rqcb_to_wqe_info_p->current_sge_offset; 

    //DANGER: because of register scarcity, encode both
    // current_sge_id and current_sge_offset in r1 and free r2 
    // now r1 = (current_sge_id << 32) + current_sge_offset
    add         r1, CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit7, current_sge_offset_sbit24_ebit31), CAPRI_KEY_FIELD(IN_P, current_sge_id), SGE_OFFSET_SHIFT // BD Slot

    add         REM_PYLD_BYTES, r0, K_REM_PYLD_BYTES
    seq         c1, CAPRI_KEY_FIELD(IN_P, in_progress), 1

    bcf         [!c1], fresh_init 
    // first_pass = TRUE
    setcf       F_FIRST_PASS, [c0]  //BD Slot

in_progress_init:
    tblwr.l     d.rsvd[63:0], 0
    #tblwr.l     d.rsvd[127:64], 0
    #tblwr.l     d.rsvd[159:128], 0

    add         NUM_VALID_SGES, r0, CAPRI_KEY_FIELD(IN_P, num_valid_sges)
    seq         c2, CAPRI_KEY_FIELD(IN_TO_S_P, page_boundary), 1
    add.!c2     SGE_P, r0, (RQWQE_OPT_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))
    add.c2      SGE_P, r0, (RQWQE_OPT_LAST_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))

    b           pre_loop
    add         r7, r0, offsetof(struct rqwqe_base_t, rsvd) //BD Slot

fresh_init:
    // fresh_init means it is a SEND_ONLY or SEND_FIRST
    // If REM_PYLD_BYTES is 0, it means it is a zero length send and also it is SEND_ONLY.
    // It is SEND_ONLY because, SEND_FIRST is always PMTU size and hence REM_PYLD_BYTES can't be 0.
    // In this case, we do not need to execute the loop and fire lkey/pt tables etc.
    seq         c5, REM_PYLD_BYTES, 0
    bcf         [c5], send_only_zero_len
    phvwr       p.cqe.recv.wrid, d.wrid //BD Slot

    // store wrid into rqcb3
    RQCB3_WRID_ADDR_GET(r6)
    memwr.d     r6, d.wrid

    tblwr.l     d.rsvd[63:0], 0
    #tblwr.l     d.rsvd[127:64], 0
    #tblwr.l     d.rsvd[159:128], 0

    add         NUM_VALID_SGES, r0, d.num_sges
    add         SGE_P, r0, (RQWQE_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))

    add         r7, r0, offsetof(struct rqwqe_base_t, rsvd)

pre_loop:
    beqi        NUM_VALID_SGES, 0, len_err_nak

loop:

    // r6 <- sge_p->len
    CAPRI_TABLE_GET_FIELD(r6, SGE_P, SGE_T, len)    //BD Slot

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

    seq            c6, r2, RDMA_RESERVED_LKEY_ID
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, rsvd_key_err, 1, c6)
    crestore.c6    [c6], K_PRIV_OPER_ENABLE, 0x1

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
    //write from d.rsvd to p.common.common_t1_s2s_s2s_data
    .cscase 1
        b       write_done
        phvwr  p.common.common_t1_s2s_s2s_data, d.rsvd[sizeof(INFO_LKEY_T):0] //BD Slot

    //first_pass, !in_progress
    //write from d.rsvd to p.common.common_t0_s2s_s2s_data
    .cscase 2
        b       write_done
        phvwr  p.common.common_t0_s2s_s2s_data, d.rsvd[sizeof(INFO_LKEY_T):0] //BD Slot

    //!first_pass, in_progress
    //write from d.rsvd to p.common.common_t0_s2s_s2s_data
    .cscase 3
        b       write_done
        phvwr  p.common.common_t0_s2s_s2s_data, d.rsvd[sizeof(INFO_LKEY_T):0] //BD Slot

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

    CAPRI_SET_TABLE_0_VALID(1)
    CAPRI_SET_TABLE_1_VALID_C(!F_FIRST_PASS, 1)

send_only_zero_len_join:
    seq         c2, K_GLOBAL_FLAG(_inv_rkey), 1 // BD Slot
    // skip_inv_rkey if remaining_payload_bytes > 0
    // or if NOT send with invalidate
    bcf         [!c5 | !c2], skip_inv_rkey
    add         r7, r0, K_GLOBAL_FLAGS  // BD Slot

    // if invalidate rkey is present, invoke it by loading appopriate
    // key entry, else load the same program as MPU only.
    KT_BASE_ADDR_GET2(KT_BASE_ADDR, TMP)
    add         TMP, r0, K_INV_R_KEY
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, TMP)

    CAPRI_SET_FIELD2(TO_S_INV_RKEY_P, user_key, K_INV_R_KEY[7:0])
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_rx_inv_rkey_validate_process, KEY_ADDR)

skip_inv_rkey:

    bbne        K_GLOBAL_FLAG(_ud), 1, skip_ud
    seq         c3, CAPRI_KEY_FIELD(IN_P, recirc_path), 1 // BD Slot

    /* Ext Header formats for UD QPs with bit positions:
     *
     * 1. SEND_IMM + VLAN
     * 68               37                 21                    5                  0
     * ------------------------------------------------------------------------------
     * | SMAC (4 bytes) | Ethtype (2 bytes) | Vlan Tag (2 bytes) | Ethtype (5 bits) |
     * ------------------------------------------------------------------------------
     *
     * 2. SEND_IMM only
     * 68               37                32
     * -------------------------------------
     * | SMAC (4 bytes) | Ethtype (5 bits) |
     * -------------------------------------
     *
     * 3. VLAN only
     * 68                  53                   37                32
     * -------------------------------------------------------------
     * | Ethtype (2 bytes) | Vlan Tag (2 bytes) | Ethtype (5 bits) |
     * -------------------------------------------------------------
     *
     * 4. SEND_ONLY, No Vlan
     * 68                64
     * --------------------
     * | Ethtype (5 bits) |
     * --------------------
     */

    bbeq        K_GLOBAL_FLAG(_immdt), 1, immdt
    nop         // BD Slot

    // is VLAN present?
    seq         c2, K_EXT_HDR_DATA[68:53], 0x8100
    phvwrpair.c2    p.cqe.recv.flags.vlan, 1, p.cqe.recv.vlan_tag, K_EXT_HDR_DATA[52:37]

    b           ud_common
    cmov        UD_TMP, c2, K_EXT_HDR_DATA[36:32], K_EXT_HDR_DATA[68:64]    // BD Slot

immdt:
    phvwr       p.cqe.recv.smac[31:0], K_EXT_HDR_DATA[68:37]

    // is VLAN present?
    seq         c2, K_EXT_HDR_DATA[36:21], 0x8100
    phvwrpair.c2    p.cqe.recv.flags.vlan, 1, p.cqe.recv.vlan_tag, K_EXT_HDR_DATA[20:5]

    cmov        UD_TMP, c2, K_EXT_HDR_DATA[4:0], K_EXT_HDR_DATA[36:32]

ud_common:
    seq         c2, UD_TMP, 0x1
    phvwr.c2    p.cqe.recv.flags.ipv4, 1

skip_ud:

    IS_ANY_FLAG_SET(c2, r7, RESP_RX_FLAG_FIRST)
    bcf         [!c2 | c3], non_first_or_recirc_pkt
    // pass the rkey to write back, since wb calls inv_rkey. Note that this s2s across multiple(two, 3 to 5) stages
    phvwr       CAPRI_PHV_FIELD(INFO_WBCB1_P, inv_r_key), K_INV_R_KEY //BD Slot

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
    // we are recircing means, we would have consumed 2 lkey's worth of DMA commands

    // if num_sges is <=2, we would have consumed all of them in this pass
    // and if payload_bytes > 0,
    // there are no sges left. generate NAK
    sle     c6, NUM_VALID_SGES, 2
    bcf     [c6], len_err_nak
    add     r2, CAPRI_KEY_FIELD(IN_P, dma_cmd_index), (MAX_PYLD_DMA_CMDS_PER_SGE * 2)   //BD Slot

    // if we reached to the max number of pyld DMA commands and still
    // pkt transfer is not complete, generate NAK 
    seq     c1, r2, RESP_RX_DMA_CMD_PYLD_BASE_END
    bcf     [c1], qp_oper_err_nak 

    // store recirc info so that stage 1 program upon recirculation
    // can access this info
    CAPRI_SET_FIELD2(TO_S_RECIRC_P, curr_wqe_ptr, K_CURR_WQE_PTR)   //BD Slot
    CAPRI_SET_FIELD2(TO_S_RECIRC_P, current_sge_id, r1[63:32])
    CAPRI_SET_FIELD2(TO_S_RECIRC_P, current_sge_offset, CURR_SGE_OFFSET)
    phvwrpair   CAPRI_PHV_FIELD(TO_S_RECIRC_P, remaining_payload_bytes), REM_PYLD_BYTES, \
                CAPRI_PHV_FIELD(TO_S_RECIRC_P, num_sges), NUM_VALID_SGES
    
    // fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_recirc_mpu_only_process, r0)

    // set recirc
    phvwr.e p.common.p4_intr_recirc, 1
    phvwr   p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_SGE_WORK_PENDING //Exit Slot

qp_oper_err_nak:
    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_cqe_error_id_vld, lif_error_id), \
                    ((1 << 5) | (1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_LOCAL_QP_OPER_ERR_OFFSET))
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_max_sge_err), 1
    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_OP_ERR)

    b           nak
    phvwrpair   p.cqe.status, CQ_STATUS_LOCAL_QP_OPER_ERR, p.cqe.error, 1 // BD Slot

len_err_nak:
    phvwrpair   p.cqe.status, CQ_STATUS_LOCAL_LEN_ERR, p.cqe.error, 1
    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_cqe_error_id_vld, lif_error_id), \
                    ((1 << 5) | (1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_LOCAL_LEN_ERR_OFFSET))
    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_insuff_sge_err), 1

    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_INV_REQ)

    // fall thru

nak:
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    // turn on ACK req bit
    // set err_dis_qp and completion flags
    or          GLOBAL_FLAGS, GLOBAL_FLAGS, RESP_RX_FLAG_ERR_DIS_QP | RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_ACK_REQ
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, GLOBAL_FLAGS)

    //Generate DMA command to skip to payload end
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)
    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/)

    // if we encounter an error here, we don't need to load
    // rqlkey and ptseg. we just need to load writeback
    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_SET_TABLE_1_VALID(0)

    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

send_only_zero_len:
    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)
    b           send_only_zero_len_join
    CAPRI_SET_TABLE_0_VALID(0)  //BD Slot

table_error:
    // set err_dis_qp and completion flags
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    or          GLOBAL_FLAGS, GLOBAL_FLAGS, RESP_RX_FLAG_ERR_DIS_QP | RESP_RX_FLAG_COMPLETION
    and         GLOBAL_FLAGS, GLOBAL_FLAGS, ~(RESP_RX_FLAG_ACK_REQ)
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, GLOBAL_FLAGS)

    phvwrpair   p.cqe.status, CQ_STATUS_LOCAL_QP_OPER_ERR, p.cqe.error, 1
    // TODO update lif_error_id if needed

    // update stats
    phvwrpair.c2   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_table_error), 1

    phvwrpair.c3   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_phv_intrinsic_error), 1

    phvwrpair.c7   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_table_resp_error), 1

    CAPRI_SET_TABLE_0_VALID(0)

    //Generate DMA command to skip to payload end if non-zero payload
    seq         c1, K_REM_PYLD_BYTES, 0
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)
    DMA_SKIP_CMD_SETUP_C(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/, !c1)

    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)
