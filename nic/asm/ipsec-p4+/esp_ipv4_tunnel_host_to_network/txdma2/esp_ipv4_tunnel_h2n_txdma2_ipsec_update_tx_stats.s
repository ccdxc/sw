#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t2_k k;
struct tx_table_s4_t2_ipsec_txdma2_stats_update_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_update_tx_stats:
    phvwri p.app_header_table2_valid, 0
    tbladd d.h2n_tx_pkts, 1
    add r1, k.txdma2_global_payload_size, IPV4_FIXED_TUNNEL_MODE_GROWTH
    add r1, r1, k.txdma2_global_pad_size
    tbladd d.h2n_tx_bytes, r1 
    nop.e
    nop 
