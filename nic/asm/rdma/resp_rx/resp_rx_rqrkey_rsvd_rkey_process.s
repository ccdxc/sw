#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s2_t1_k k;
struct key_entry_aligned_t d;

#define DMA_CMD_BASE r1
#define TMP r3
#define DB_ADDR r4
#define DB_DATA r5
#define GLOBAL_FLAGS r6
#define RQCB2_ADDR r7
#define RQCB1_ADDR r7
#define T2_ARG r5

#define LKEY_TO_PT_INFO_T   struct resp_rx_lkey_to_pt_info_t
#define INFO_WBCB1_P t2_s2s_rqcb1_write_back_info
#define TO_S_WB_P    to_s5_wb1_info

#define IN_P t1_s2s_rkey_info
#define IN_TO_S_P to_s4_lkey_info

#define K_VA CAPRI_KEY_RANGE(IN_P, va_sbit0_ebit7, va_sbit8_ebit63)
#define K_LEN CAPRI_KEY_FIELD(IN_P, len)
#define K_CURR_SGE_OFFSET CAPRI_KEY_FIELD(IN_P, current_sge_offset)

%%
    .param  resp_rx_rqcb1_write_back_mpu_only_process

.align
resp_rx_rqrkey_rsvd_rkey_process:

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_RX_DMA_CMD_START_FLIT_ID, CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index))
    // r1 has DMA_CMD_BASE

    //STORAGE_USE_CASE
    crestore            [c1], d.{override_lif_vld}, 0x1
    //Always assume resvered lkey based address to be of host_addr
    DMA_PKT2MEM_SETUP_OVERRIDE_LIF(DMA_CMD_BASE, c0, K_LEN, K_VA, c1, d.override_lif)
    
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS 

    bbne        CAPRI_KEY_FIELD(IN_P, dma_cmdeop), 1, load_write_back
    IS_ANY_FLAG_SET(c2, GLOBAL_FLAGS, RESP_RX_FLAG_COMPLETION) // BD Slot

    DMA_SET_END_OF_CMDS_C(struct capri_dma_cmd_pkt2mem_t, DMA_CMD_BASE, !c2)
    
load_write_back:
    CAPRI_SET_TABLE_1_VALID(0)
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)
