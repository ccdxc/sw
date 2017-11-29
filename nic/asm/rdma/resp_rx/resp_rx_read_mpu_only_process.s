#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_rx_phv_t p;
// there is no 'd' vector for mpu-only program
struct resp_rx_read_atomic_process_k_t k;

#define RKEY_INFO_T struct resp_rx_key_info_t
#define R_KEY r2
#define KT_BASE_ADDR r6
#define KEY_ADDR r2
#define RAW_TABLE_PC r3

#define DMA_CMD_BASE r1
#define DB_ADDR r4
#define DB_DATA r5
%%
    .param  resp_rx_rqlkey_process

.align
resp_rx_read_mpu_only_process:

    CAPRI_GET_TABLE_1_ARG(resp_rx_phv_t, r4)

    CAPRI_SET_FIELD_RANGE(r4, RKEY_INFO_T, va, len, k.{args.va...args.len})

    add     R_KEY, r0, k.args.r_key

    KT_BASE_ADDR_GET(KT_BASE_ADDR, r1)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, R_KEY)

    // for read operation, we skip PT and hence there is no need 
    // to set dma_cmd_start_index and dma_cmdeop while calling 
    // key_process program. dma_cmdeop is either set after RSQWQE 
    // DMA command (in stage0) or upon RSQ Doorbell DMA command 
    // (in the current stage).
    // tbl_id: 1, acc_ctrl: REMOTE_READ, cmdeop: 0, nak_code: REM_ACC_ERR
    CAPRI_SET_FIELD_RANGE(r4, RKEY_INFO_T, tbl_id, nak_code, ((TABLE_1 << 17) | (ACC_CTRL_REMOTE_READ << 9) | (0 << 8) | (AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR))))
    
    // set write back related params
    // incr_nxt_to_go_token_id: 1, incr_c_index: 0, 
    // skip_pt: 1, invoke_writeback: 1
    CAPRI_SET_FIELD_RANGE(r4, RKEY_INFO_T, incr_nxt_to_go_token_id, invoke_writeback, (1<<3 | 0 << 2 | 1 << 1 | 1))

    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqlkey_process)
    // Initiate next table lookup with 32 byte Key address (so avoid whether keyid 0 or 1)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, RAW_TABLE_PC, KEY_ADDR)

    // ring rsq dbell if reqd
    bbeq    k.args.skip_rsq_dbell, 1, exit
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, k.global.lif, k.global.qtype, DB_ADDR)
    CAPRI_SETUP_DB_DATA(k.global.qid, RSQ_RING_ID, k.args.rsq_p_index, DB_DATA)
    // store db_data in LE format
    phvwr   p.db_data1, DB_DATA.dx

    // DMA for RSQ DoorBell
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_RSQ_DB)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data1, db_data1, DB_ADDR)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

exit:
    nop.e
    nop
