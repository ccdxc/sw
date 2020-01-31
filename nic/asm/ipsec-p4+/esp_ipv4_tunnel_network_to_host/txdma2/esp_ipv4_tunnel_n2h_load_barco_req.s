#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_esp_v4_tunnel_n2h_txdma2_load_barco_req_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_txdma2_load_in_desc 
        .param TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE
        .param esp_ipv4_tunnel_n2h_txdma2_load_out_desc
        .param esp_ipv4_tunnel_n2h_txdma2_load_ipsec_int
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .align
esp_ipv4_tunnel_n2h_load_barco_req:
    add r4, k.t0_s2s_cb_addr, IPSEC_BARCO_RECORD_RING_SW_CI_ADDR_OFFSET_N2H
    memwr.h r4, k.t0_s2s_barco_sw_cindex

    seq c1, d.input_list_address, 0
    b.c1 esp_ipv4_tunnel_n2h_invalid_barco_req
    nop

    addui       r5, r0, hiword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)
    addi        r5, r5, loword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_txdma2_load_in_desc, r5, TABLE_SIZE_512_BITS)

    add r2, r0, d.{output_list_address}.dx
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, esp_ipv4_tunnel_n2h_txdma2_load_out_desc, r2, TABLE_SIZE_512_BITS)
    sub    r5, d.{output_list_address}.dx, IPSEC_SCRATCH_OFFSET 
    phvwr  p.ipsec_to_stage2_out_desc_addr, r5 
    phvwr  p.ipsec_to_stage5_out_desc_addr, r5 
    sub r3, d.{input_list_address}.dx, IPSEC_SCRATCH_OFFSET 
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, esp_ipv4_tunnel_n2h_txdma2_load_ipsec_int, r3, TABLE_SIZE_512_BITS)
    phvwr.e p.txdma2_global_in_desc_addr, r3 
    nop

esp_ipv4_tunnel_n2h_invalid_barco_req:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwr p.p4_intr_global_drop, 1
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_TXDMA2_INVALID_BARCO_REQ, 1)

    nop.e
    nop
