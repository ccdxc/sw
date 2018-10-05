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
#define IN_TO_S_P     to_s3_wqe_info
#define TO_S_WB1_P to_s5_wb1_info

#define SGE_P           r4
#define REM_PYLD_BYTES  r5
#define CURR_SGE_OFFSET r1
#define TRANSFER_BYTES  r2

#define F_FIRST_PASS  c7

#define TMP r3
#define KT_BASE_ADDR r6
#define KEY_ADDR r6
#define DMA_CMD_BASE r6
#define GLOBAL_FLAGS r6

#define IN_P    t0_s2s_rqcb_to_wqe_info
#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, priv_oper_enable)

%%
    .param  resp_rx_rqlkey_process
    .param  resp_rx_inv_rkey_validate_process
    .param  resp_rx_rqlkey_rsvd_lkey_process
    .param  resp_rx_rqcb1_write_back_mpu_only_process

.align
resp_rx_rqwqe_opt_process:

    // num_valid_sges == 2 ?
    seq         c1, d.num_sges, 2
    // c1: num_sges == 2

    // curr_sge_offset = spec_psn << log_pmtu
    add         TRANSFER_BYTES, r0, CAPRI_KEY_FIELD(IN_TO_S_P, spec_psn)
    sll         TRANSFER_BYTES, TRANSFER_BYTES, CAPRI_KEY_FIELD(IN_P, log_pmtu)
    // TRANSFER_BYTES now has the number of bytes transferred so far

    // init curr_sge_offset = transfer_bytes
    // if SGE 1, curr_sge_offset will remain same
    // if SGE 2, subtract len of SGE 1 from it
    add         CURR_SGE_OFFSET, r0, TRANSFER_BYTES

    //  update SGE_P to SGE 1
    add         SGE_P, r0, (RQWQE_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))
    // check if transfer_bytes < len of SGE 1
    CAPRI_TABLE_GET_FIELD(r6, SGE_P, SGE_T, len)
    slt         c3, TRANSFER_BYTES, r6
    // c3: SGE1
    bcf         [c3], sge_common
    // Sometimes for MID packets also completion may be required (in case of lkey access permission failures). 
    // Hence copying wrid field always into phv's cqwqe structure in case of any SEND packet. 
    // It may or may not be used depending on whether completion is happening or not.
    phvwr       p.cqe.recv.wrid, d.wrid // BD Slot

sge2:
    // if num_sges != 2, generate NAK
    bcf         [!c1], nak
    // update SGE_P to SGE 2
    sub         SGE_P, SGE_P, 1, LOG_SIZEOF_SGE_T_BITS // BD Slot
    // curr_sge_offset -= len of SGE 1
    sub         CURR_SGE_OFFSET, CURR_SGE_OFFSET, r6

sge_common:
    tblwr.l     d.rsvd[63:0], 0
    add         r7, r0, offsetof(struct rqwqe_base_t, rsvd) 
    add         REM_PYLD_BYTES, r0, CAPRI_KEY_FIELD(IN_P, remaining_payload_bytes)
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

    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    // if remaining payload bytes are not zero, generate NAK
    bcf         [!c5], nak
    IS_ANY_FLAG_SET(c1, GLOBAL_FLAGS, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY) //BD Slot

    CAPRI_SET_TABLE_0_VALID(1)
    // skip_inv_rkey if NOT send with invalidate
    bbeq        K_GLOBAL_FLAG(_inv_rkey), 0, skip_inv_rkey
    CAPRI_SET_TABLE_1_VALID_C(!F_FIRST_PASS, 1) // BD Slot

    // pass the rkey to write back, since wb calls inv_rkey. Note that this s2s across multiple(two, 3 to 5) stages
    phvwr       CAPRI_PHV_FIELD(INFO_WBCB1_P, inv_r_key), CAPRI_KEY_FIELD(IN_TO_S_P, inv_r_key)

    // if invalidate rkey is present, invoke it by loading appopriate
    // key entry, else load the same program as MPU only.
    KT_BASE_ADDR_GET2(KT_BASE_ADDR, TMP)
    add         TMP, r0, CAPRI_KEY_FIELD(IN_TO_S_P, inv_r_key)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, TMP)

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_rx_inv_rkey_validate_process, KEY_ADDR)

skip_inv_rkey:

    ARE_ALL_FLAGS_SET(c2, GLOBAL_FLAGS, RESP_RX_FLAG_UD|RESP_RX_FLAG_IMMDT)
    phvwr.c2    p.cqe.recv.smac[31:0], CAPRI_KEY_FIELD(IN_TO_S_P, ext_hdr_data)

    phvwr.e     CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1
    CAPRI_SET_FIELD2_C(TO_S_WB1_P, incr_c_index, 1, c1) // Exit Slot

nak:
    // since num_sges is <=2, we would have consumed all of them in this pass
    // and if payload_bytes > 0,
    // there are no sges left. generate NAK
    phvwrpair      p.cqe.status, CQ_STATUS_LOCAL_LEN_ERR, p.cqe.error, 1 // BD Slot
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

    phvwr          p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_OP_ERR)

    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)
