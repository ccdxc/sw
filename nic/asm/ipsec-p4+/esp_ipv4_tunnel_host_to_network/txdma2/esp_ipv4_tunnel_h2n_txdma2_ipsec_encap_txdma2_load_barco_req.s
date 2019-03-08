#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_ipsec_encap_txdma2_load_barco_req_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc 
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_out_desc
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_ipsec_int
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req:
    phvwr p.txdma2_global_in_desc_addr, d.{input_list_address}.dx
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 10
    add r5, r0, d.{input_list_address}.dx
    subi r6, r5, 64 
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc, r5, TABLE_SIZE_512_BITS)
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_ipsec_int, r6, TABLE_SIZE_512_BITS)
    nop.e
    nop
 
