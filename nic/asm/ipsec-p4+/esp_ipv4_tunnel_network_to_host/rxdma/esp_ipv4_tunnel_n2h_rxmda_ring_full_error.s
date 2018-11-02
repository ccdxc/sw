#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s1_t1_k k;
struct rx_table_s1_t1_rxmda_ring_full_error_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_n2h_rxmda_ring_full_error:
    tbladd.f d.n2h_rx_drops, 1
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.app_header_table1_valid, 0
    nop 
