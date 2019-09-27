
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s1_t0_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s1_t0_k_ k;
struct rx_table_s1_t0_d d;

%%

.param  eth_rx_arm
.param  eth_rx_fetch_desc

.align
eth_rx_rss_skip:
    // Is this a request to arm the rx cq?
    bbeq            k.eth_rx_global_do_eq, 1, eth_rx_rss_skip_arm
    // Is the qstate addr programmed?
    seq             c1, k.eth_rx_to_s1_qstate_addr, 0 // BD Slot
    bcf             [c1], eth_rx_rss_skip_drop
    nop

    // Launch Qstate table in next stage, to fetch rx desc
    phvwri          p.{app_header_table0_valid...app_header_table3_valid}, TABLE_VALID_0
    phvwri          p.common_te0_phv_table_lock_en, 1
    phvwri          p.common_te0_phv_table_pc, eth_rx_fetch_desc[38:6]
    phvwr.e         p.common_te0_phv_table_addr, k.eth_rx_to_s1_qstate_addr
    phvwri.f        p.common_te0_phv_table_raw_table_size, LG2_RX_QSTATE_SIZE

eth_rx_rss_skip_arm:
    // Launch Qstate table in next stage, to arm rx cq
    phvwri          p.{app_header_table0_valid...app_header_table3_valid}, TABLE_VALID_0
    phvwri          p.common_te0_phv_table_lock_en, 1
    phvwri          p.common_te0_phv_table_pc, eth_rx_arm[38:6]
    phvwr.e         p.common_te0_phv_table_addr, k.eth_rx_to_s1_qstate_addr
    phvwri.f        p.common_te0_phv_table_raw_table_size, LG2_RX_QSTATE_SIZE

eth_rx_rss_skip_drop:
    // Reachable when RSS table is programmed and Qstate is not programmed
    // End of pipeline - Make sure no more tables will be launched
    phvwri.e        p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.f        p.p4_intr_global_drop, 1
