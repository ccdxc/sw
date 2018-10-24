#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t1_k k;
struct tx_table_s4_t1_ipsec_txdma_stats_update_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_n2h_txdma2_ipsec_update_tx_stats:
    tbladd d.n2h_tx_pkts, 1
    sub r1, k.txdma2_global_payload_size, k.txdma2_global_pad_size
    subi r1, r1, 2 
    tbladd d.n2h_tx_bytes, r1 
    phvwri.e p.app_header_table2_valid, 0
    nop 
