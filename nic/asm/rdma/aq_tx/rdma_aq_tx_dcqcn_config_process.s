#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct dcqcn_config_cb_t d;
struct aq_tx_s5_t3_k k;

#define IN_TO_S_P to_s5_info

#define TO_SQCB0_INFO_P to_s6_info
#define DCQCN_CONFIG_TO_CB_P t3_s2s_dcqcn_config_to_cb_info

#define K_DCQCN_CB_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, cb_addr_sbit0_ebit31, cb_addr_sbit32_ebit33)

%%

    .param      rdma_aq_tx_dcqcn_cb_process
.align
rdma_aq_tx_dcqcn_config_process:

    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_5
    bcf         [!c1], bubble_to_next_stage
    add         r2, K_DCQCN_CB_ADDR, r0 // BD Slot

    phvwrpair   CAPRI_PHV_FIELD(DCQCN_CONFIG_TO_CB_P, byte_cntr_thr), d.rp_byte_reset, CAPRI_PHV_FIELD(DCQCN_CONFIG_TO_CB_P, alpha_val), d.rp_initial_alpha_value

setup_dcqcn:
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_dcqcn_cb_process, r2)

bubble_to_next_stage:
    // This program should only be running in stage 4 and 5
    CAPRI_GET_TABLE_3_K(aq_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS)
    nop     // Exit Slot
