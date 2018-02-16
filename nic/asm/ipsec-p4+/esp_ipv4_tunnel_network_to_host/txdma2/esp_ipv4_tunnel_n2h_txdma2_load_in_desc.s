#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_esp_v4_tunnel_n2h_txdma2_load_in_desc_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma2_build_decap_packet 
        .param IPSEC_CB_BASE
        .align
esp_ipv4_tunnel_n2h_txdma2_load_in_desc:
    phvwr p.t0_s2s_in_page_addr, d.{addr0}.dx
    nop.e
    nop 
