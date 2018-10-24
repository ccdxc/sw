#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s1_t1_k k;
struct tx_table_s1_t1_ipsec_stage1_drops_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_txdma1_ring_full_error:
    tbladd d.h2n_tx_drops, 1
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.app_header_table1_valid, 0
    nop 
