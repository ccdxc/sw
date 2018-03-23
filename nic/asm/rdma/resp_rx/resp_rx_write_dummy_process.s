#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct rqcb3_t d;
struct resp_rx_s1_t1_k k;

#define RKEY_INFO_P t1_s2s_key_info
#define RQCB0_WB_T struct resp_rx_rqcb0_write_back_info_t
#define R_KEY r2
#define KT_BASE_ADDR r6
#define KEY_ADDR r2
#define RQCB0_ADDR r2
#define GLOBAL_FLAGS r7

#define IN_P t1_s2s_rqcb_to_write_rkey_info

%%
    .param  resp_rx_rqlkey_process

.align
resp_rx_write_dummy_process:

    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS

    CAPRI_RESET_TABLE_1_ARG()

    seq     c1, CAPRI_KEY_FIELD(IN_P,load_reth), 1

    cmov    r1, c1, d.va, CAPRI_KEY_FIELD(IN_P, va)
    CAPRI_SET_FIELD2(RKEY_INFO_P, va, r1)
    add     r1, r1, CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15)
    tblwr   d.va, r1

    cmov    r1, c1, d.len, CAPRI_KEY_FIELD(IN_P, len)
    CAPRI_SET_FIELD2(RKEY_INFO_P, len, CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15))
    sub     r1, r1, CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15)
    tblwr   d.len, r1

    cmov    R_KEY, c1, d.r_key, CAPRI_KEY_RANGE(IN_P, r_key_sbit0_ebit23, r_key_sbit24_ebit31)
    tblwr   d.r_key, R_KEY

    KT_BASE_ADDR_GET2(KT_BASE_ADDR, r1)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, R_KEY)

    CAPRI_SET_FIELD2(RKEY_INFO_P, dma_cmd_start_index, RESP_RX_DMA_CMD_PYLD_BASE)

    // tbl_id: 1, acc_ctrl: REMOTE_WRITE, cmdeop: 1, nak_code: REM_ACC_ERR
    CAPRI_SET_FIELD_RANGE2(RKEY_INFO_P, tbl_id, nak_code, ((TABLE_1 << 17) | (ACC_CTRL_REMOTE_WRITE << 9) | (1 << 8) | (AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR))))

    // set write back related params
    CAPRI_SET_FIELD2(RKEY_INFO_P, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD2(RKEY_INFO_P, incr_c_index, CAPRI_KEY_FIELD(IN_P, incr_c_index))
    CAPRI_SET_FIELD2(RKEY_INFO_P, invoke_writeback, 1)

    // Initiate next table lookup with 32 byte Key address (so avoid whether keyid 0 or 1)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_rx_rqlkey_process, KEY_ADDR)

    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY)
    tblwr.c1    d.va, 0
    tblwr.c1    d.r_key, 0
    tblwr.c1    d.len, 0

    nop.e
    nop

