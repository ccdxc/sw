#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t1_k k;
struct tx_table_s2_t1_esp_v4_tunnel_n2h_txdma2_load_out_desc_d d;
struct phv_ p;

%%
        .align
        .param esp_ipv4_tunnel_n2h_txdma2_load_pad_size_l4_proto
esp_ipv4_tunnel_n2h_txdma2_load_out_desc:
    phvwr p.t0_s2s_out_page_addr, d.{addr0}.dx
    add r3, r0, d.length0
    add r2, d.{addr0}.dx, r3.wx
    subi r2, r2, 2
    phvwri p.app_header_table1_valid, 0

    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_txdma2_load_pad_size_l4_proto[33:6]
    phvwr  p.common_te0_phv_table_addr, r2 

    nop.e
    nop 
