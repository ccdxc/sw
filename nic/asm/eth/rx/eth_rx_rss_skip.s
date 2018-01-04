
#include "INGRESS_p.h"
#include "ingress.h"
#include "defines.h"

struct phv_ p;
struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_d d;

%%

.param  eth_rx_fetch_desc

.align
eth_rx_rss_skip:
    // Lookup Qstate
    phvwri          p.{app_header_table0_valid...app_header_table3_valid}, (1 << 3)
    phvwri          p.common_te0_phv_table_lock_en, 1
    phvwri          p.common_te0_phv_table_pc, eth_rx_fetch_desc[38:6]
    phvwr.e         p.common_te0_phv_table_addr, k.{eth_rx_global_qstate_addr_sbit0_ebit31, eth_rx_global_qstate_addr_sbit32_ebit33}
    phvwri          p.common_te0_phv_table_raw_table_size, LG2_RX_QSTATE_SIZE
