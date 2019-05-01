#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "defines.h"

struct resp_rx_phv_t p;
struct rqcb3_t d;
struct resp_rx_s1_t1_k k;

#define RKEY_INFO_P t1_s2s_rkey_info
#define RQCB0_WB_T struct resp_rx_rqcb0_write_back_info_t
#define R_KEY r4
#define KT_BASE_ADDR r6
#define KEY_ADDR r2
#define RQCB0_ADDR r2
#define GLOBAL_FLAGS r7
#define DMA_CMD_BASE r4

#define IN_P t1_s2s_rqcb_to_write_rkey_info
#define TO_S_WB1_P      to_s5_wb1_info
#define TO_S_STATS_INFO_P to_s7_stats_info
#define TO_S_CQCB_P to_s6_cqcb_info

#define K_REM_PYLD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15)
#define K_VA CAPRI_KEY_RANGE(IN_P, va_sbit0_ebit7, va_sbit8_ebit63)
#define K_LEN CAPRI_KEY_RANGE(IN_P, len_sbit0_ebit7, len_sbit24_ebit31)
#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_P, priv_oper_enable)
#define K_RKEY CAPRI_KEY_FIELD(IN_P, r_key)

%%
    .param  resp_rx_rqrkey_process
    .param  resp_rx_rqrkey_rsvd_rkey_process
    .param  resp_rx_rqcb1_write_back_mpu_only_process

.align
resp_rx_write_dummy_process:

    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS

    CAPRI_RESET_TABLE_1_ARG()

    seq     c1, CAPRI_KEY_FIELD(IN_P,load_reth), 1

    cmov    r1, c1, d.va, K_VA
    CAPRI_SET_FIELD2(RKEY_INFO_P, va, r1)
    // r1: va

    cmov    r3, c1, d.len, K_LEN
    CAPRI_SET_FIELD2(RKEY_INFO_P, len, K_REM_PYLD_BYTES)
    // r3: len

    // store the original dma_len separately
    tblwr.!c1    d.dma_len, r3

    cmov    R_KEY, c1, d.r_key, K_RKEY

    sub     r3, r3, K_REM_PYLD_BYTES
    tblwr   d.len, r3

    IS_ANY_FLAG_SET(c7, r7, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY)
    bcf     [c7], last_or_only

    add     r1, r1, K_REM_PYLD_BYTES // BD Slot
    tblwr   d.va, r1

    b       done
    tblwr.f d.r_key, R_KEY // BD Slot

last_or_only:
    tblwr    d.va, 0
    tblwr    d.r_key, 0

    // if d.len is not 0, send a NAK
    seq      c5, d.len, 0
    bcf      [!c5], inv_req_nak
    tblwr.f  d.len, 0
    
    // copy the orignal dma_len as the cqe.length. 
    // note that this cqe is used only when immediate data is present
    phvwr   p.cqe.length, d.dma_len

done:
    seq     c5, R_KEY, RDMA_RESERVED_LKEY_ID
    // c5: rsvd key
    bcf     [!c5], skip_priv_oper
    seq.c5  c5, K_PRIV_OPER_ENABLE, 1 // BD Slot
    // c5: rsvd key + priv oper enabled
    phvwr.!c5   CAPRI_PHV_FIELD(RKEY_INFO_P, rsvd_key_err), 1

skip_priv_oper:
    KT_BASE_ADDR_GET2(KT_BASE_ADDR, r1)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, R_KEY)
    
    CAPRI_SET_FIELD2(RKEY_INFO_P, user_key, R_KEY[7:0])

    IS_ANY_FLAG_SET(c4, GLOBAL_FLAGS, RESP_RX_FLAG_COMPLETION | RESP_RX_FLAG_RING_DBELL)
    sne          c3, CAPRI_KEY_FIELD(IN_P, pad), r0
    //if completion present OR pad non-zero, set cmdeop to 0
    setcf        c2, [c4 | c3]
    // dma_cmd_start_index: 2, tbl_id: 1, acc_ctrl: REMOTE_WRITE, cmdeop: 0 or 1 depending on whether above flags are set
    CAPRI_SET_FIELD_RANGE2_C(RKEY_INFO_P, dma_cmd_start_index, dma_cmdeop, ((RESP_RX_DMA_CMD_PYLD_BASE << 12) | (TABLE_1 << 9) | (ACC_CTRL_REMOTE_WRITE << 1) | 0), c2)
    CAPRI_SET_FIELD_RANGE2_C(RKEY_INFO_P, dma_cmd_start_index, dma_cmdeop, ((RESP_RX_DMA_CMD_PYLD_BASE << 12) | (TABLE_1 << 9) | (ACC_CTRL_REMOTE_WRITE << 1) | 1), !c2)

    //if completion not present and pad non-zero, overwrite the DMA_CMD_SKIP_PLD with cmd_eop=1, and skip=1
    //it would have been originally written in _ext routine with cmd_eop=0, skip=1
    setcf        c2, [!c4 & c3]
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)
    DMA_SKIP_CMD_SETUP_C(DMA_CMD_BASE, 1 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/, c2)

    // set write back related params
    phvwrpair   CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1, \
                CAPRI_PHV_FIELD(TO_S_WB1_P, incr_c_index), CAPRI_KEY_FIELD(IN_P, incr_c_index)

    // invoke rqrkey 
    //CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqrkey_process, KEY_ADDR)
    CAPRI_NEXT_TABLE1_READ_PC_CE(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqrkey_rsvd_rkey_process, resp_rx_rqrkey_process, KEY_ADDR, c5)

inv_req_nak:
    phvwrpair   p.cqe.status, CQ_STATUS_REMOTE_INV_REQ_ERR, p.cqe.error, 1
    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_INV_REQUEST_OFFSET))

    phvwrpair   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_disabled), 1, \
                CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, qp_err_dis_dma_len_err), 1

    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_INV_REQ)

    // fall thru

nak:
    // turn on ACK req bit
    // set err_dis_qp flag
    or          GLOBAL_FLAGS, GLOBAL_FLAGS, RESP_RX_FLAG_ERR_DIS_QP | RESP_RX_FLAG_ACK_REQ

    //Generate DMA command to skip to payload end
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD)
    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/)

    bbeq        K_GLOBAL_FLAG(_completion), 1, load_wb
    // if we encounter an error here, we don't need to load
    // rqrkey. we just need to load writeback
    CAPRI_SET_TABLE_1_VALID(0) // BD Slot

    phvwr       CAPRI_PHV_FIELD(TO_S_CQCB_P, async_error_event), 1
    phvwrpair   p.s1.eqwqe.code, EQE_CODE_QP_ERR_REQEST, p.s1.eqwqe.type, EQE_TYPE_QP
    phvwr       p.s1.eqwqe.qid, K_GLOBAL_QID
    phvwr       CAPRI_PHV_RANGE(TO_S_STATS_INFO_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_RESP_STAT(LIF_STATS_RESP_RX_INV_REQUEST_OFFSET))

    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, async_or_async_error_event), 1

load_wb:
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, GLOBAL_FLAGS)
    // invoke an mpu-only program which will bubble down and eventually invoke write back
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)
