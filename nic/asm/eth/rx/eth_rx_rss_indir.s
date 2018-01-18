
#include "INGRESS_p.h"
#include "ingress.h"
#include "defines.h"

struct phv_ p;
struct eth_rx_rss_indir_k k;
struct eth_rx_rss_indir_eth_rx_rss_indir_d d;

%%

.param  eth_rx_fetch_desc

.align
eth_rx_rss_indir:
    // This program might be executed for other APP types if all table valid
    // bits are cleared by that app in stage0. In this case, the Toeplitz
    // PHV area might contain information valid for that APP. So, we should not
    // modify the PHV.
    // RSS enable value is always set for Ethernet LIFs. So check that before
    // making any changes to the PHV.
    add             r6, r0, d.enable
    beq             r6, r0, eth_rx_rss_disabled
    nop

    // Zero-out the flits used for Toeplitz
    phvwr           p.{toeplitz_input0_data...toeplitz_key1_data}, r0
    phvwr           p.{toeplitz_input2_data...toeplitz_key2_data}, r0

    // Pass RSS hash to the driver in the completion entry
    phvwr           p.eth_rx_cq_desc_rss_hash, r1.wx

    // Compute Qstate address for d.qid
    add             r5, k.toeplitz_key2_data[33:0], d.qid, LG2_RX_QSTATE_SIZE

    // Lookup Qstate
    phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
    phvwri          p.common_te0_phv_table_lock_en, 1
    phvwri          p.common_te0_phv_table_pc, eth_rx_fetch_desc[38:6]
    phvwr.e         p.common_te0_phv_table_addr, r5
    phvwri          p.common_te0_phv_table_raw_table_size, LG2_RX_QSTATE_SIZE

eth_rx_rss_disabled:
    nop.e
    nop
