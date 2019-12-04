#include "capri.h"
#include "types.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"

struct resp_rx_phv_t p;
struct resp_rx_s3_t0_k k;
struct rqwqe_base_t d;

#define SGE_VA              r2
#define SGE_TRANSFER_BYTES  r3
#define SGE_P               r4
#define SGE_OFFSET          r5
#define REM_PYLD_BYTES      r1
#define DMA_CMD_BASE        r6
#define GLOBAL_FLAGS        r6

#define F_FIRST_PASS  c7     

#define IN_P t0_s2s_sge_info
#define INFO_LKEY_T struct resp_rx_key_info_t
#define TO_S_STATS_INFO_P to_s7_stats_info

#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_P, priv_oper_enable)
#define K_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, sge_offset_sbit0_ebit7, sge_offset_sbit24_ebit31)
#define K_REM_PYLD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15)
#define K_NUM_SGES CAPRI_KEY_FIELD(IN_P, num_sges)

%%
    .param  resp_rx_rqlkey_process
    .param  resp_rx_rqlkey_rsvd_lkey_process
    .param  resp_rx_rqcb1_write_back_mpu_only_process

.align
resp_rx_rqsge_process:
    bcf         [c2 | c3 | c7], table_error
    seq         c1, CAPRI_KEY_FIELD(IN_P, page_boundary), 1 // BD Slot

    add.!c1     SGE_P, r0, (RQWQE_OPT_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))
    add.c1      SGE_P, r0, (RQWQE_OPT_LAST_SGE_OFFSET_BITS - (1 << LOG_SIZEOF_SGE_T_BITS))

    // initialize the scratch area with minimal
    // instructions. Whichever fields are always populated are not initialized, but
    // important fields such as flags are initialized.
    tblwr.l     d.rsvd[63:0], 0
    //tblwr.l     d.rsvd[127:64], 0
    //tblwr.l     d.rsvd[171:128], 0

    add         r7, r0, offsetof(struct rqwqe_base_t, rsvd)
    add         REM_PYLD_BYTES, r0, K_REM_PYLD_BYTES

    // first_pass = TRUE
    setcf       F_FIRST_PASS, [c0]           

sge_loop:
    // r6 <- sge_p->len
    CAPRI_TABLE_GET_FIELD(r6, SGE_P, SGE_T, len)

    cmov        SGE_OFFSET, F_FIRST_PASS, K_SGE_OFFSET, 0
    //sge_remaining_bytes = sge_p->len - current_sge_offset;
    sub         r6, r6, SGE_OFFSET

    //transfer_bytes = min(sge_remaining_bytes, remaining_payload_bytes);
    slt         c2, r6, REM_PYLD_BYTES
    cmov        SGE_TRANSFER_BYTES, c2, r6, REM_PYLD_BYTES

    // r2 <- sge_p->va
    CAPRI_TABLE_GET_FIELD(SGE_VA, SGE_P, SGE_T, va)

    // transfer_va = sge_p->va + current_sge_offset;
    add         SGE_VA, SGE_VA, SGE_OFFSET
    // sge_to_lkey_info_p->sge_va = transfer_va;
    CAPRI_SET_TABLE_FIELD_LOCAL(r7, INFO_LKEY_T, va, SGE_VA)

    // sge_to_lkey_info_p->sge_bytes = transfer_bytes;
    CAPRI_SET_TABLE_FIELD_LOCAL(r7, INFO_LKEY_T, len, SGE_TRANSFER_BYTES)

    // sge_to_lkey_info_p->dma_cmd_start_index = dma_cmd_index;
    add         r6, r0, CAPRI_KEY_FIELD(IN_P, dma_cmd_index)
    add.!F_FIRST_PASS r6, r6, MAX_PYLD_DMA_CMDS_PER_SGE
    CAPRI_SET_TABLE_FIELD_LOCAL(r7, INFO_LKEY_T, dma_cmd_start_index, r6)

    //sge_to_lkey_info_p->sge_index = index;
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, tbl_id, 1, !F_FIRST_PASS)

    //remaining_payload_bytes -= transfer_bytes;
    sub         REM_PYLD_BYTES, REM_PYLD_BYTES, SGE_TRANSFER_BYTES

    //current_sge_offset += transfer_bytes;
    add         SGE_OFFSET, SGE_OFFSET, SGE_TRANSFER_BYTES

    KT_BASE_ADDR_GET2(r6, r2)

    // r2 <- sge_p->l_key
    CAPRI_TABLE_GET_FIELD(r2, SGE_P, SGE_T, l_key)

    seq            c6, r2, RDMA_RESERVED_LKEY_ID
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, rsvd_key_err, 1, c6)
    crestore.c6    [c6], K_PRIV_OPER_ENABLE, 0x1

    // are remaining_payload_bytes 0 ?
    seq         c5, REM_PYLD_BYTES, 0

    // DANGER: Do not move the instruction above.
    // tblrdp above should be reading l_key from old sge_p
    //sge_p++;
    sub.!c5     SGE_P, SGE_P, 1, LOG_SIZEOF_SGE_T_BITS

    KEY_ENTRY_ADDR_GET(r6, r6, r2)
    // now r6 has key_addr

    // Initiate next table lookup with 32 byte Key address (so avoid whether keyid 0 or 1)

    CAPRI_GET_TABLE_0_OR_1_K_NO_VALID(resp_rx_phv_t, r2, F_FIRST_PASS)
    CAPRI_NEXT_TABLE_I_READ_PC_C(r2, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqlkey_rsvd_lkey_process, resp_rx_rqlkey_process, r6, c6)

    // is num_valid_sges == 2 ?
    seq         c6, K_NUM_SGES, 2

    // set dma_cmdeop for the last table (could be T0 or T1)
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, dma_cmdeop, 1, c5)
    CAPRI_SET_TABLE_FIELD_LOCAL_C(r7, INFO_LKEY_T, invoke_writeback, 1, c5)

    phvwr.F_FIRST_PASS  p.common.common_t0_s2s_s2s_data, d.rsvd[sizeof(INFO_LKEY_T):0]
    phvwr.!F_FIRST_PASS  p.common.common_t1_s2s_s2s_data, d.rsvd[sizeof(INFO_LKEY_T):0]

write_done:

    // loop one more time ONLY if:
    // remaining_payload_bytes > 0 (!c5) AND
    // did only one pass (F_FIRST_PASS) AND
    // k_num_sges = 2
    setcf       c4, [!c5 & F_FIRST_PASS & c6]
    bcf         [c4], sge_loop
    // make F_FIRST_PASS = FALSE only when we are going for second pass
    setcf.c4    F_FIRST_PASS, [!c0] // BD Slot

    CAPRI_SET_TABLE_0_VALID_CE(c0, 1)
    CAPRI_SET_TABLE_1_VALID_C(!F_FIRST_PASS, 1) // Exit Slot

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
