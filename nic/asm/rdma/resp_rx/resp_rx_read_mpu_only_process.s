#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
// there is no 'd' vector for mpu-only program
struct resp_rx_s1_t1_k k;

#define IN_P    t1_s2s_rqcb_to_read_atomic_rkey_info
#define OUT_P   t1_s2s_key_info
#define R_KEY r2
#define KT_BASE_ADDR r6
#define KEY_ADDR r2

#define DMA_CMD_BASE r1
#define DB_ADDR r4
#define DB_DATA r5
%%
    .param  resp_rx_rqlkey_process

.align
resp_rx_read_mpu_only_process:

    CAPRI_RESET_TABLE_1_ARG();

    phvwrpair   CAPRI_PHV_FIELD(OUT_P, va), CAPRI_KEY_FIELD(IN_P, va), \
                CAPRI_PHV_FIELD(OUT_P, len), CAPRI_KEY_RANGE(IN_P, len_sbit0_ebit23, len_sbit24_ebit31)

    add     R_KEY, r0, CAPRI_KEY_FIELD(IN_P, r_key)

    KT_BASE_ADDR_GET2(KT_BASE_ADDR, r1)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, R_KEY)

    // for read operation, we skip PT and hence there is no need 
    // to set dma_cmd_start_index and dma_cmdeop while calling 
    // key_process program. dma_cmdeop is either set after RSQWQE 
    // DMA command (in stage0) or upon RSQ Doorbell DMA command 
    // (in the current stage).
    // tbl_id: 1, acc_ctrl: REMOTE_READ, cmdeop: 0, nak_code: REM_ACC_ERR
    CAPRI_SET_FIELD_RANGE2_IMM(OUT_P, tbl_id, nak_code, ((TABLE_1 << 17) | (ACC_CTRL_REMOTE_READ << 9) | (0 << 8) | (AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR))))
    
    // set write back related params
    // incr_nxt_to_go_token_id: 1, incr_c_index: 0, 
    // skip_pt: 1, invoke_writeback: 1
    CAPRI_SET_FIELD_RANGE2_IMM(OUT_P, incr_nxt_to_go_token_id, invoke_writeback, (1<<3 | 0 << 2 | 1 << 1 | 1))

    // Initiate next table lookup with 32 byte Key address (so avoid whether keyid 0 or 1)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_rx_rqlkey_process, KEY_ADDR)

    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, K_GLOBAL_LIF, K_GLOBAL_QTYPE, DB_ADDR)
    CAPRI_SETUP_DB_DATA(K_GLOBAL_QID, RSQ_RING_ID, CAPRI_KEY_RANGE(IN_P, rsq_p_index_sbit0_ebit7, rsq_p_index_sbit8_ebit15), DB_DATA)
    // store db_data in LE format
    phvwr   p.db_data1, DB_DATA.dx

    // DMA for RSQ DoorBell
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RSQ_DB)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data1, db_data1, DB_ADDR)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

exit:
    nop.e
    nop
