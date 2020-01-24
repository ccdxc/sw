#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct dcqcn_config_cb_t d;
struct aq_tx_s4_t3_k k;

#define QP_MAX_RATE 100000
#define IN_TO_S_P to_s4_info

#define DMA_CMD_BASE    r6
#define K_DCQCN_CB_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, cb_addr_sbit0_ebit31, cb_addr_sbit32_ebit33)

%%
.align
rdma_aq_tx_dcqcn_config_process:

    add         r2, K_DCQCN_CB_ADDR, r0
    seq         c1, d.rp_qp_rate, QP_MAX_RATE

    phvwrpair   p.dcqcn_cb.byte_counter_thr, d.rp_byte_reset, p.dcqcn_cb.alpha_value, d.rp_initial_alpha_value
    phvwr       p.dcqcn_cb.rate_enforced, d.rp_qp_rate
    phvwr       p.dcqcn_cb.target_rate, QP_MAX_RATE
    phvwr       p.dcqcn_cb.max_rate_reached, 1
    add         r3, r0, d.rp_token_bucket_size
    phvwrpair   p.dcqcn_cb.cur_avail_tokens, r3, p.dcqcn_cb.token_bucket_size, r3
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, AQ_TX_DMA_CMD_START_FLIT_ID, AQ_TX_DMA_CMD_DCQCN_CB)
    CAPRI_SET_TABLE_3_VALID(0)
    DMA_HBM_PHV2MEM_SETUP_E(DMA_CMD_BASE, dcqcn_cb, dcqcn_cb, r2)
