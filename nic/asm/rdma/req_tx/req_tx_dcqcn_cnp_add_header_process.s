#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"

struct req_tx_phv_t p;
struct dcqcn_cnp_add_header_t d;
struct req_tx_s4_t3_k k;

#define TO_S_STATS_INFO_P to_s7_stats_info
#define SQCB_TO_DCQCN_CFG_T t3_s2s_dcqcn_config_info

#define K_CNP_DSCP CAPRI_KEY_RANGE(SQCB_TO_DCQCN_CFG_T, np_cnp_dscp_sbit0_ebit1, np_cnp_dscp_sbit2_ebit5)

#define DMA_CMD_BASE        r1

%%
    .param      req_tx_stats_process

.align
req_tx_dcqcn_cnp_add_header_process:
    // invoke stats as mpu only, branch to dcqcn_cnp_sent in req_tx_stats
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_stats_process, r0)

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_CNP_HEADER_TEMPLATE_2)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, cnp_ipv6_hdr, cnp_ipv6_hdr)
    phvwr     p.cnp_ipv6_hdr.version, d.version
    phvwr.e   p.cnp_ipv6_hdr.flow_label, d.flow_label
    phvwr     p.cnp_ipv6_hdr.cnp_tc.dscp, K_CNP_DSCP

