#include "resp_rx.h"
#include "rqcb.h"

struct resp_rx_phv_t p;
struct dcqcn_config_cb_t d;

struct resp_rx_s1_t1_k k;

#define IN_TO_S_P   to_s1_dcqcn_info
#define TO_S3_P  to_s3_dcqcn_info

#define K_DCQCN_CFG_ID CAPRI_KEY_FIELD(IN_TO_S_P, dcqcn_cfg_id)

%%

.align
resp_rx_dcqcn_config_load_process:
    
    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_2
    bcf         [!c1], config_load
    nop    // BD Slot

    phvwr.e     CAPRI_PHV_FIELD(TO_S3_P, min_time_btwn_cnps), d.rp_rate_reduce_monitor_period
    CAPRI_SET_TABLE_1_VALID(0)	// Exit Slot

config_load:
    KT_BASE_ADDR_GET2(r2, r3)
    sll         r3, KEY_ENTRY_SIZE_BYTES, K_GLOBAL_LOG_NUM_KT_ENTRIES
    add	        r2, r2, r3
    add         r2, r2, K_DCQCN_CFG_ID, LOG_SIZEOF_DCQCN_CONFIG_T
    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r2)
