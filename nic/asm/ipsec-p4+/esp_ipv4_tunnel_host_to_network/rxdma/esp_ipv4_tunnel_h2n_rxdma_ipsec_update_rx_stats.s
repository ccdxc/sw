#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t1_k k;
struct rx_table_s4_t1_ipsec_rxdma_stats_update_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_rxdma_ipsec_update_rx_stats:
    phvwri p.app_header_table2_valid, 0
    tbladd d.h2n_rx_pkts, 1
    add r1, k.t1_s2s_payload_size, IPV4_FIXED_TUNNEL_MODE_GROWTH
    tbladd d.h2n_rx_bytes, r1 
    nop.e
    nop 
