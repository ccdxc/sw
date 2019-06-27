#include "resp_tx.h"
#include "rqcb.h"

struct resp_tx_phv_t p;
struct dcqcn_config_cb_t d;

// Note: This stage has stage-to-stage info only in s1.
struct resp_tx_s1_t1_k k;

#define RQCB0_WB_INFO_P t1_s2s_rqcb0_write_back_info

#define RQCB_TO_DCQCN_CFG_T t1_s2s_dcqcn_config_info
#define TO_S4_P  to_s4_dcqcn_info
#define CFG_TO_DCQCN_T t0_s2s_cfg_to_dcqcn_info

#define K_DCQCN_CFG_ID CAPRI_KEY_FIELD(RQCB_TO_DCQCN_CFG_T, dcqcn_config_id)
#define K_DCQCN_CFG_BASE CAPRI_KEY_RANGE(RQCB_TO_DCQCN_CFG_T, dcqcn_config_base_sbit0_ebit23, dcqcn_config_base_sbit32_ebit33)

%%

.align
resp_tx_dcqcn_config_load_process:
    
    // Pin dcqcn_cfg to stage 1 table 2, stage2 table 2
    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_2
    bcf         [!c1], config_load
    add         r2, r0, d.rp_threshold  // BD Slot

    phvwrpair   CAPRI_PHV_FIELD(CFG_TO_DCQCN_T, alpha_timer_interval), d.rp_dce_tcp_rtt, CAPRI_PHV_FIELD(CFG_TO_DCQCN_T, timer_exp_thr), r2
    b           exit
    phvwr       CAPRI_PHV_FIELD(CFG_TO_DCQCN_T, g_val), d.rp_dce_tcp_g

config_load:
    add         r2, K_DCQCN_CFG_BASE, K_DCQCN_CFG_ID, LOG_SIZEOF_DCQCN_CONFIG_T
    CAPRI_GET_TABLE_1_K(resp_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r2)
    nop.e
    nop

exit:
    CAPRI_SET_TABLE_1_VALID(0)
    nop.e
    nop
