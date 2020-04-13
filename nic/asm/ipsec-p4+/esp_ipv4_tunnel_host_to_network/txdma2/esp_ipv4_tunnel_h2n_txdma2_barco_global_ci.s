#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s6_t0_ipsec_txdma2_update_global_ci_d d;
struct phv_ p;

%%
        .align
esp_ipv4_tunnel_h2n_barco_global_ci:
    tblmincri.f d.ci, ASIC_BARCO_RING_SLOTS_SHIFT, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop
