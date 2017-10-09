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
    add r1, r0, d.addr0
    phvwr p.t0_s2s_in_page_addr, r1.dx
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_pc, esp_v4_tunnel_n2h_txdma2_build_decap_packet[33:6] 
    phvwr  p.common_te0_phv_table_addr, k.ipsec_to_stage2_ipsec_cb_addr 
    nop.e
    nop 
