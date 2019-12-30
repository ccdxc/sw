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
#define INFO_SGE_T  t0_s2s_sge_info
#define INFO_WBCB1_P t2_s2s_rqcb1_write_back_info
#define IN_TO_S_P     to_s2_wqe_info
#define TO_S_WB1_P to_s5_wb1_info
#define TO_S_RECIRC_P to_s1_recirc_info
#define TO_S_INV_RKEY_P to_s4_lkey_info

#define LEN_SUM         r3
#define SGE_P           r4
#define REM_PYLD_BYTES  r5
#define CURR_SGE_OFFSET r1
#define TRANSFER_BYTES  r2

// below macros are used only in 8x4 and 16x2 case
#define SGE_OFFSET      r1[31:0]
#define SGE_ID          r1[63:32]
#define WQE_OFFSET      r6
#define ADDR_TO_LOAD    r6
#define SGE_OFFSET_SHIFT 32
#define CACHELINE_END_ADDR_OFFSET 63

#define F_FIRST_PASS  c7

#define TMP r3
#define KT_BASE_ADDR r6
#define KEY_ADDR r6
#define DMA_CMD_BASE r6
#define GLOBAL_FLAGS r6

#define IN_P    t0_s2s_rqcb_to_wqe_info
#define TO_S_STATS_INFO_P to_s7_stats_info

#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, priv_oper_enable)
#define K_REM_PYLD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15)
#define K_SPEC_PSN CAPRI_KEY_RANGE(IN_TO_S_P, spec_psn_sbit0_ebit2, spec_psn_sbit19_ebit23)
#define K_INV_R_KEY CAPRI_KEY_RANGE(IN_TO_S_P, inv_r_key_sbit0_ebit2, inv_r_key_sbit27_ebit31)
#define K_EXT_HDR_DATA CAPRI_KEY_RANGE(IN_TO_S_P, ext_hdr_data_sbit0_ebit63, ext_hdr_data_sbit64_ebit68)
#define K_CURR_WQE_PTR CAPRI_KEY_RANGE(IN_P,curr_wqe_ptr_sbit0_ebit7, curr_wqe_ptr_sbit56_ebit63)
#define K_LOG_RQ_PAGE_SIZE CAPRI_KEY_RANGE(IN_P, log_rq_page_size_sbit0_ebit2, log_rq_page_size_sbit3_ebit4)

%%
    .param  resp_rx_rqlkey_process
    .param  resp_rx_inv_rkey_validate_process
    .param  resp_rx_rqlkey_rsvd_lkey_process
    .param  resp_rx_rqcb1_write_back_mpu_only_process
    .param  resp_rx_rqsge_process
    .param  resp_rx_recirc_mpu_only_process

.align
resp_rx_rqwqe_opt_process:

    bcf         [c2 | c3 | c7], table_error

    // curr_sge_offset = spec_psn << log_pmtu
    add         TRANSFER_BYTES, r0, K_SPEC_PSN // BD Slot
    sll         TRANSFER_BYTES, TRANSFER_BYTES, CAPRI_KEY_FIELD(IN_P, log_pmtu)
    // TRANSFER_BYTES now has the number of bytes transferred so far

    // store transfer_bytes + remaining_pyld_bytes in TMP variable
    // later copy this TMP to cqe.length if it is last or only pkt
    add         REM_PYLD_BYTES, r0, K_REM_PYLD_BYTES
    add         TMP, TRANSFER_BYTES, REM_PYLD_BYTES

    seq         c5, REM_PYLD_BYTES, 0
    bcf         [c5], send_only_zero_len
    phvwr       p.cqe.recv.wrid, d.wrid //BD Slot

    seq         c1, d.num_sges, 0

    bcf         [c1], len_err_nak
    seq         c1, d.wqe_format, RQWQE_FORMAT_DEFAULT  // BD Slot
    // c1: num_sges <= 2
    bcf         [c1], decode_2x4

    seq         c1, d.wqe_format, RQWQE_FORMAT_8x4   //BD Slot
    // c1: 2 < num_sges <= 8
    sle.c1      c3, d.num_sges, 8
    sle.!c1     c3, d.num_sges, 16
    bcf         [!c3], qp_oper_err_nak

    // below steps are common to 8x4 and 16x2 WQE formats
    add         LEN_SUM, r0, r0     // BD Slot
    add         r1, r0, r0
    // first_pass = TRUE
    setcf       F_FIRST_PASS, [c0]

    CAPRI_RESET_TABLE_0_ARG()
    // wqe_to_sge_info_p->rem_pyld_bytes = rem_pyld_bytes
    CAPRI_SET_FIELD2(INFO_SGE_T, remaining_payload_bytes, K_REM_PYLD_BYTES)

    add         SGE_P, r0, RQWQE_SGE_OFFSET_BITS

decode_wqe_opt:

    // 8x4 WQE format
    /*
     *    lower 32-byte encoding
     *    0          32          63
     *    -------------------------
     *    |     len   |    len    |
     *    -------------------------
     *    |     len   |    len    |
     *    -------------------------
     *    |     len   |    len    |
     *    -------------------------
     *    |     len   |    len    |
     *    -------------------------
     */

    // 16x2 WQE format
    /*
     *    lower 32-byte encoding
     *    0    16    32    48    63
     *    -------------------------
     *    | len | len | len | len |
     *    -------------------------
     *    | len | len | len | len |
     *    -------------------------
     *    | len | len | len | len |
     *    -------------------------
     *    | len | len | len | len |
     *    -------------------------
     */

    slt         c2, SGE_ID, d.num_sges
    bcf         [!c2], len_err_nak
    // move SGE_P to point to the next len
    sub.c1      SGE_P, SGE_P, 1, LOG_SIZEOF_WQE_8x4_T_BITS  // BD Slot
    sub.!c1     SGE_P, SGE_P, 1, LOG_SIZEOF_WQE_16x2_T_BITS

    // curr_sge_offset = transfer_bytes - len_sum
    sub         r6, TRANSFER_BYTES, LEN_SUM
    add         r1, r6, SGE_ID, SGE_OFFSET_SHIFT

    // get the length of SGE
    CAPRI_TABLE_GET_FIELD_C(r6, SGE_P, WQE_8x4_T, len, c1)
    CAPRI_TABLE_GET_FIELD_C(r6, SGE_P, WQE_16x2_T, len, !c1)
    // r6: SGE len

    // update the sum of lengths
    add         LEN_SUM, LEN_SUM, r6
    // check if this packet falls in this SGE
    slt         c3, TRANSFER_BYTES, LEN_SUM

    // incr sge_id
    add.!c3     r6, SGE_ID, 1
    bcf         [!c3], decode_wqe_opt
    add.!c3     r1, SGE_OFFSET, r6, SGE_OFFSET_SHIFT // BD Slot

    // if c3 is true, we found the SGE id.
    // calculate the address to load. load sge_process
    // pass sge_id and sge_offset

    // sge_remaining_bytes = sge_p->len - current_sge_offset;
    sub         r6, r6, SGE_OFFSET
    // len = min(sge_remaining_bytes, remaining_payload_bytes);
    slt         c2, r6, REM_PYLD_BYTES
    cmov        r6, c2, r6, REM_PYLD_BYTES
    // r6: length of transfer

    // transfer_bytes += len
    add         TRANSFER_BYTES, TRANSFER_BYTES, r6

    // rem_pyld_bytes -= len
    sub         REM_PYLD_BYTES, REM_PYLD_BYTES, r6

    // are remaining_payload_bytes 0 ?
    seq         c5, REM_PYLD_BYTES, 0

    // wqe_offset = 64 + (sge_id << log_sizeof_sge)
    add         WQE_OFFSET, RQWQE_OPT_SGE_OFFSET, SGE_ID, LOG_SIZEOF_SGE_T

    bcf         [!F_FIRST_PASS], skip_sge_load
    // addr_to_load = wqe_ptr + wqe_offset
    add         ADDR_TO_LOAD, K_CURR_WQE_PTR, WQE_OFFSET // BD Slot

    // if log_rq_page_size = 0, rq is in hbm and page boundary check is not needed
    seq         c6, K_LOG_RQ_PAGE_SIZE, 0
    bcf         [c6], page_boundary_check_done
    // set start_addr
    sub         ADDR_TO_LOAD, ADDR_TO_LOAD, 2, LOG_SIZEOF_SGE_T // BD Slot
    srl         r7, ADDR_TO_LOAD, K_LOG_RQ_PAGE_SIZE
    // set end_addr
    add         r6, ADDR_TO_LOAD, CACHELINE_END_ADDR_OFFSET
    srl         r6, r6, K_LOG_RQ_PAGE_SIZE
    //  check if start and end addresses belong to the same host page
    seq         c2, r6, r7
    // c2: same page

    add         WQE_OFFSET, RQWQE_OPT_SGE_OFFSET, SGE_ID, LOG_SIZEOF_SGE_T
    add         ADDR_TO_LOAD, K_CURR_WQE_PTR, WQE_OFFSET
    // move addr_to_load back by sizeof 2 SGE's
    sub.c2      ADDR_TO_LOAD, ADDR_TO_LOAD, 2, LOG_SIZEOF_SGE_T
    // move addr_to_load back by sizeof 3 SGE's
    sub.!c2     ADDR_TO_LOAD, ADDR_TO_LOAD, 3, LOG_SIZEOF_SGE_T
    CAPRI_SET_FIELD2_C(INFO_SGE_T, page_boundary, 1, !c2)

page_boundary_check_done:
    // wqe_to_sge_info_p->sge_offset = sge_offset
    CAPRI_SET_FIELD2(INFO_SGE_T, sge_offset, SGE_OFFSET)

    CAPRI_SET_FIELD2(INFO_SGE_T, priv_oper_enable, K_PRIV_OPER_ENABLE)

    CAPRI_SET_FIELD2(INFO_SGE_T, dma_cmd_index, CAPRI_KEY_FIELD(IN_P, dma_cmd_index))

    // load rqsge_process
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqsge_process, ADDR_TO_LOAD)

skip_sge_load:
    // incr sge_id
    add         r6, SGE_ID, 1
    add         r1, SGE_OFFSET, r6, SGE_OFFSET_SHIFT

    // is sge_id < num_sges
    slt         c6, SGE_ID, d.num_sges

    // loop one more time ONLY if:
    // remaining_payload_bytes > 0 (!c5) AND
    // did only one pass (F_FIRST_PASS) AND
    // SGE_ID is less than d.num_sges (c6)
    setcf       c4, [!c5 & F_FIRST_PASS & c6]
    bcf         [c4], decode_wqe_opt
    // make F_FIRST_PASS = FALSE only when we are going for second pass
    setcf.c4    F_FIRST_PASS, [!c0] // BD Slot

    // num_sges = 1 if one pass, else num_sges = 2
    cmov        r6, F_FIRST_PASS, 1, 2
    CAPRI_SET_FIELD2(INFO_SGE_T, num_sges, r6)

    // set F_FIRST_PASS back to true, because in loop_exit
    // table1_valid is set to true if F_FIRST_PASS is false
    setcf       F_FIRST_PASS, [c0]
    b           loop_exit
    add         TMP, TRANSFER_BYTES, REM_PYLD_BYTES // BD Slot

decode_2x4:
    // decode 2x4 WQE format

    /*
     *    lower 32-byte encoding
     *    0          32          63
     *    -------------------------
     *    |          va           |
     *    -------------------------
     *    |     len   |    lkey   |
     *    -------------------------
     *    |          va           |
     *    -------------------------
     *    |     len   |    lkey   |
     *    -------------------------
     */

    // init curr_sge_offset = transfer_bytes
    // if SGE 1, curr_sge_offset will remain same
    // if SGE 2, subtract len of SGE 1 from it
    add         CURR_SGE_OFFSET, r0, TRANSFER_BYTES

    //  update SGE_P to SGE 1
    add         SGE_P, r0, (RQWQE_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))
    // check if transfer_bytes < len of SGE 1
    CAPRI_TABLE_GET_FIELD(r6, SGE_P, SGE_T, len)
    seq         c1, d.num_sges, 2
    // c1: num_sges = 2
    slt         c3, TRANSFER_BYTES, r6
    // c3: SGE1
    bcf         [c3], sge_common
    // Sometimes for MID packets also completion may be required (in case of lkey access permission failures). 
    // Hence copying wrid field always into phv's cqwqe structure in case of any SEND packet. 
    // It may or may not be used depending on whether completion is happening or not.
    phvwr       p.cqe.recv.wrid, d.wrid // BD Slot

sge2:
    // if num_sges != 2, generate NAK
    bcf         [!c1], len_err_nak
    // update SGE_P to SGE 2
    sub         SGE_P, SGE_P, 1, LOG_SIZEOF_SGE_T_BITS // BD Slot
    // curr_sge_offset -= len of SGE 1
    sub         CURR_SGE_OFFSET, CURR_SGE_OFFSET, r6

sge_common:
    tblwr.l     d.rsvd[63:0], 0
    add         r7, r0, offsetof(struct rqwqe_base_t, rsvd) 
    // first_pass = TRUE
    setcf       F_FIRST_PASS, [c0]

    // By now, SGE_P is pointing either to SGE 1 or SGE 2,
    // depending on transfer_bytes and len of SGE 1
    // we also know that SGE_P is a valid SGE pointer
   
loop:
    // r7 <- sge_p->len
    CAPRI_TABLE_GET_FIELD(r6, SGE_P, SGE_T, len)

    //sge_remaining_bytes = sge_p->len - current_sge_offset;
    sub         r6, r6, CURR_SGE_OFFSET

    //transfer_bytes = min(sge_remaining_bytes, remaining_payload_bytes);
    slt         c2, r6, REM_PYLD_BYTES
    cmov        r6, c2, r6, REM_PYLD_BYTES

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
    //add         r1, r2[31:0], r1[63:32], 32

    //  r6 <- sge_p->len
    CAPRI_TABLE_GET_FIELD(r6, SGE_P, SGE_T, len)

    // if (current_sge_offset == sge_p->len) {
    seq         c2, r6, r2
    //current_sge_offset = 0;
    add.c2      CURR_SGE_OFFSET, r0, r0

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

    // set dma_cmdeop for the last table (could be T0 or T1)
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, dma_cmdeop, 1, c5)
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, invoke_writeback, 1, c5)

    phvwr.F_FIRST_PASS  p.common.common_t0_s2s_s2s_data, d.rsvd[sizeof(INFO_LKEY_T):0]
    phvwr.!F_FIRST_PASS  p.common.common_t1_s2s_s2s_data, d.rsvd[sizeof(INFO_LKEY_T):0]

write_done:

    // loop one more time ONLY if:
    // remaining_payload_bytes > 0 (!c5) AND
    // did only one pass (F_FIRST_PASS) AND
    // there are 2 SGE's (c1) AND
    // we processed SGE1 in the firt pass (c3)
    setcf       c4, [!c5 & F_FIRST_PASS & c1 & c3]
    bcf         [c4], loop
    // make F_FIRST_PASS = FALSE only when we are going for second pass
    setcf.c4    F_FIRST_PASS, [!c0] // BD Slot

loop_exit:

    CAPRI_SET_TABLE_0_VALID(1)
    CAPRI_SET_TABLE_1_VALID_C(!F_FIRST_PASS, 1)

send_only_zero_len_join:
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    IS_ANY_FLAG_SET(c1, GLOBAL_FLAGS, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY)

    // if remaining payload bytes are not zero, recirc
    bcf         [!c5], recirc
    // for send last/only, copy total message length to cqe
    phvwr.c1    p.cqe.length, TMP //BD Slot

    seq         c2, K_GLOBAL_FLAG(_inv_rkey), 1 // BD Slot
    // skip_inv_rkey if remaining_payload_bytes > 0
    // or if NOT send with invalidate
    bcf         [!c5 | !c2], skip_inv_rkey
    // pass the rkey to write back, since wb calls inv_rkey. Note that this s2s across multiple(two, 3 to 5) stages
    phvwr       CAPRI_PHV_FIELD(INFO_WBCB1_P, inv_r_key), K_INV_R_KEY //BD Slot

    // if invalidate rkey is present, invoke it by loading appopriate
    // key entry, else load the same program as MPU only.
    KT_BASE_ADDR_GET2(KT_BASE_ADDR, TMP)
    add         TMP, r0, K_INV_R_KEY
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, TMP)

    CAPRI_SET_FIELD2(TO_S_INV_RKEY_P, user_key, K_INV_R_KEY[7:0])
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_rx_inv_rkey_validate_process, KEY_ADDR)

skip_inv_rkey:

    ARE_ALL_FLAGS_SET(c2, GLOBAL_FLAGS, RESP_RX_FLAG_UD|RESP_RX_FLAG_IMMDT)
    phvwr.c2    p.cqe.recv.smac[31:0], K_EXT_HDR_DATA

    phvwr.e     CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1
    CAPRI_SET_FIELD2_C(TO_S_WB1_P, incr_c_index, 1, c1) // Exit Slot

recirc:

    // if there are no sges left. generate NAK
    bcf     [!c6], len_err_nak
    add     r2, CAPRI_KEY_FIELD(IN_P, dma_cmd_index), (MAX_PYLD_DMA_CMDS_PER_SGE * 2)   //BD Slot

    // if we reached the max number of pyld DMA commands and still
    // pkt transfer is not complete, generate NAK
    seq     c1, r2, RESP_RX_DMA_CMD_PYLD_BASE_END
    bcf     [c1], qp_oper_err_nak

    // store recirc info so that stage 1 program upon recirculation
    // can access this info
    add     r6, K_CURR_WQE_PTR, 2, LOG_SIZEOF_SGE_T //BD Slot
    CAPRI_SET_FIELD2(TO_S_RECIRC_P, curr_wqe_ptr, r6)
    CAPRI_SET_FIELD2(TO_S_RECIRC_P, current_sge_id, SGE_ID)
    CAPRI_SET_FIELD2(TO_S_RECIRC_P, current_sge_offset, SGE_OFFSET)
    phvwrpair   CAPRI_PHV_FIELD(TO_S_RECIRC_P, remaining_payload_bytes), REM_PYLD_BYTES, \
                CAPRI_PHV_FIELD(TO_S_RECIRC_P, num_sges), d.num_sges

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
    // since num_sges is <=2, we would have consumed all of them in this pass
    // and if payload_bytes > 0,
    // there are no sges left. generate NAK

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
    CAPRI_SET_TABLE_0_VALID(0) // BD Slot

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
