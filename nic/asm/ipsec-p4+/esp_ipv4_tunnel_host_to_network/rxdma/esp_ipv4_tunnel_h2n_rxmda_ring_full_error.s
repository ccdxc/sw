#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s1_t2_k k;
struct rx_table_s1_t2_rxmda_ring_full_error_d d;
struct phv_ p;

%%
        .align
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N

esp_ipv4_tunnel_h2n_rxmda_ring_full_error:
    tbladd.f d.h2n_rx_drops, 1
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table2_valid...app_header_table3_valid}, 0
    nop 
