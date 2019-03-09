#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_esp_v4_tunnel_n2h_txdma2_load_barco_req_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_txdma2_load_in_desc 
        .param esp_ipv4_tunnel_n2h_txdma2_load_out_desc
        .param esp_ipv4_tunnel_n2h_txdma2_load_ipsec_int
        .align
esp_ipv4_tunnel_n2h_load_barco_req:
    add r1, r0, d.{input_list_address}.dx
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_txdma2_load_in_desc, r1, TABLE_SIZE_512_BITS)
    add r2, r0, d.{output_list_address}.dx
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_txdma2_load_out_desc, r2, TABLE_SIZE_512_BITS)
    sub    r5, d.{output_list_address}.dx, 64
    phvwr  p.ipsec_to_stage2_out_desc_addr, r5 
    phvwr  p.ipsec_to_stage5_out_desc_addr, r5 
    sub r3, d.{input_list_address}.dx, 64
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_txdma2_load_ipsec_int, r3, TABLE_SIZE_512_BITS)
    phvwr.e p.txdma2_global_in_desc_addr, r3 
    nop
