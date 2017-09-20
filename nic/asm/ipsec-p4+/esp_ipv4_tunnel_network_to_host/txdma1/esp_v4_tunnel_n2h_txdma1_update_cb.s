#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t0_k k;
struct tx_table_s4_t0_esp_v4_tunnel_n2h_txdma1_update_cb_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_txdma1_update_cb:
    phvwri p.app_header_table0_valid, 0
    add r1, r0, d.cb_cindex
    addi r1, r1, 1
    tblwr d.cb_cindex, r1
    nop
    tblwr d.rxdma_ring_cindex, r1
    nop.e
    nop
