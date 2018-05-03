#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rqcb2_t d;
struct resp_tx_s1_t0_k k;

#define RSQWQE_P    r3

#define RSQ_BT_INFO_P t0_s2s_bt_info
#define RSQ_BT_TO_S_INFO_T struct resp_tx_to_stage_bt_info_t
#define RSQ_BT_TO_S_K_P to_s1_bt_info
#define K_RSQ_BASE_ADDR CAPRI_KEY_RANGE(RSQ_BT_TO_S_K_P, rsq_base_addr_sbit0_ebit5, rsq_base_addr_sbit30_ebit31)
#define K_SEARCH_INDEX CAPRI_KEY_RANGE(RSQ_BT_TO_S_K_P, search_index_sbit0_ebit5, search_index_sbit14_ebit15)
#define K_BT_IN_PROGRESS CAPRI_KEY_FIELD(RSQ_BT_TO_S_K_P, bt_in_progress)
#define K_READ_RSP_IN_PROGRESS CAPRI_KEY_FIELD(RSQ_BT_TO_S_K_P, read_rsp_in_progress)

%%
    .param      resp_tx_rsqwqe_bt_process

resp_tx_rqcb2_bt_process:

    // copy bt_info from rqcb2 to rsqwqe stage
    CAPRI_RESET_TABLE_0_ARG()
    CAPRI_SET_FIELD_RANGE2(RSQ_BT_INFO_P, read_or_atomic, len, d.{bt_info.read_or_atomic...bt_info.len})
    
    CAPRI_GET_STAGE_NEXT_ARG(resp_tx_phv_t, r1)
    CAPRI_SET_FIELD_RANGE(r1, RSQ_BT_TO_S_INFO_T, log_rsq_size, bt_in_progress, CAPRI_KEY_RANGE(RSQ_BT_TO_S_K_P, log_rsq_size, bt_in_progress))

    // load the rsqwqe at search_index
    sll         RSQWQE_P, K_RSQ_BASE_ADDR, RSQ_BASE_ADDR_SHIFT
    add         RSQWQE_P, RSQWQE_P, K_SEARCH_INDEX, LOG_SIZEOF_RSQWQE_T
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_tx_rsqwqe_bt_process, RSQWQE_P)
