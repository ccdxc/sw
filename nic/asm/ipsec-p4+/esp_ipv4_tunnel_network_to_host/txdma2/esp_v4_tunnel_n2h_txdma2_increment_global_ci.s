#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t1_k k;
struct tx_table_s1_t1_esp_v4_tunnel_n2h_txdma2_increment_global_ci_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_txdma2_increment_global_ci:
    tblmincri.f d.ci, IPSEC_BARCO_RING_WIDTH, 1
    nop.e
    nop
