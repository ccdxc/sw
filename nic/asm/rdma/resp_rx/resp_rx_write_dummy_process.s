#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_rx_phv_t p;
struct rqcb1_t d;
struct resp_rx_write_dummy_process_k_t k;

#define RKEY_INFO_T struct resp_rx_key_info_t
#define RQCB0_WB_T struct resp_rx_rqcb0_write_back_info_t
#define R_KEY r2
#define KT_BASE_ADDR r6
#define KEY_ADDR r2
#define RQCB0_ADDR r2
#define RAW_TABLE_PC r3
#define GLOBAL_FLAGS r7

%%
    .param  resp_rx_rqlkey_process
    .param  resp_rx_rqcb0_write_back_process

.align
resp_rx_write_dummy_process:

    add         GLOBAL_FLAGS, r0, k.global.flags

    CAPRI_GET_TABLE_1_ARG(resp_rx_phv_t, r4)

    seq     c1, k.args.load_reth, 1

    cmov    r1, c1, d.va, k.args.va
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, va, r1)
    add     r1, r1, k.args.remaining_payload_bytes
    tblwr   d.va, r1

    cmov    r1, c1, d.len, k.args.len
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, len, k.args.remaining_payload_bytes)
    sub     r1, r1, k.args.remaining_payload_bytes
    tblwr   d.len, r1

    cmov    R_KEY, c1, d.r_key, k.args.r_key
    tblwr   d.r_key, R_KEY

    KT_BASE_ADDR_GET(KT_BASE_ADDR, r1)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, R_KEY)

    CAPRI_SET_FIELD(r4, RKEY_INFO_T, dma_cmd_start_index, RESP_RX_DMA_CMD_PYLD_BASE)

    // tbl_id: 1, acc_ctrl: REMOTE_WRITE, cmdeop: 1, nak_code: REM_ACC_ERR
    CAPRI_SET_FIELD_RANGE(r4, RKEY_INFO_T, tbl_id, nak_code, ((TABLE_1 << 17) | (ACC_CTRL_REMOTE_WRITE << 9) | (1 << 8) | (AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR))))

    // set write back related params
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, incr_c_index, k.args.incr_c_index)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, invoke_writeback, 1)

    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqlkey_process)
    // Initiate next table lookup with 32 byte Key address (so avoid whether keyid 0 or 1)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, RAW_TABLE_PC, KEY_ADDR)

    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY)
    tblwr.c1    d.va, 0
    tblwr.c1    d.r_key, 0
    tblwr.c1    d.len, 0

    nop.e
    nop

