#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t1_k k;
struct tx_table_s1_t1_esp_v4_tunnel_n2h_txdma2_increment_global_ci_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_txdma2_increment_global_ci:
    add r1, d.ci, 1
    and r1, r1, IPSEC_BARCO_RING_INDEX_MASK
    tblwr d.ci, r1
    nop.e
    nop
