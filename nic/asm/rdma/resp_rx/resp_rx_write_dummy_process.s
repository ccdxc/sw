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
#define ALIGNED_KEY_ADDR r6
#define KEY_ID r2
#define RQCB0_ADDR r2
#define RAW_TABLE_PC r3
#define GLOBAL_FLAGS r7

%%
    .param  resp_rx_rqlkey_process
    .param  resp_rx_rqcb0_write_back_process

.align
resp_rx_write_dummy_process:

    add         GLOBAL_FLAGS, r0, k.global.flags

    CAPRI_GET_TABLE_0_ARG(resp_rx_phv_t, r4)

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

    //key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET()) +
    // ((sge_p->l_key & KEY_INDEX_MASK) * sizeof(key_entry_t));
    andi        r2, R_KEY, KEY_INDEX_MASK
    sll         r2, r2, LOG_SIZEOF_KEY_ENTRY_T

    add         KEY_ADDR, r2, KT_BASE_ADDR

    //aligned_key_addr = key_addr & ~HBM_CACHE_LINE_MASK;
    and         r6, KEY_ADDR, HBM_CACHE_LINE_SIZE_MASK
    sub         ALIGNED_KEY_ADDR, KEY_ADDR, r6

    //key_id = (key_addr % HBM_CACHE_LINE_SIZE) / sizeof(key_entry_t);
    // compute (key_addr - aligned_key_addr) >> log_key_entry_t
    sub         KEY_ID, KEY_ADDR, ALIGNED_KEY_ADDR
    srl         KEY_ID, KEY_ID, LOG_SIZEOF_KEY_ENTRY_T

    CAPRI_SET_FIELD(r4, RKEY_INFO_T, key_id, KEY_ID)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, dma_cmd_start_index, RESP_RX_DMA_CMD_PYLD_BASE)

    CAPRI_SET_FIELD(r4, RKEY_INFO_T, tbl_id, TABLE_0)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, dma_cmdeop, 1)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, acc_ctrl, ACC_CTRL_REMOTE_WRITE)

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqlkey_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, ALIGNED_KEY_ADDR)

    IS_ANY_FLAG_SET(c1, r7, RESP_RX_FLAG_LAST|RESP_RX_FLAG_ONLY)
    tblwr.c1    d.va, 0
    tblwr.c1    d.r_key, 0
    tblwr.c1    d.len, 0

    CAPRI_GET_TABLE_1_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RQCB0_WB_T, tbl_id, TABLE_1)
    CAPRI_SET_FIELD(r4, RQCB0_WB_T, in_progress, 0)
    CAPRI_SET_FIELD(r4, RQCB0_WB_T, incr_nxt_to_go_token_id, 1)
    CAPRI_SET_FIELD(r4, RQCB0_WB_T, incr_c_index, k.args.incr_c_index)

    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqcb0_write_back_process)   
    RQCB0_ADDR_GET(RQCB0_ADDR)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB0_ADDR)

    nop.e
    nop

