#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_ipsec_encap_txdma2_load_barco_req_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc 
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_ipsec_int
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req:
    add r1, k.ipsec_to_stage2_ipsec_cb_addr, IPSEC_BARCO_RECORD_RING_SW_CI_ADDR_OFFSET_H2N
    // Have to memwr all cindex, cannot optimize by writing every 16
    memwr.h r1, k.ipsec_to_stage2_barco_sw_cindex

    phvwr p.txdma2_global_in_desc_addr, d.{input_list_address}.dx
    add r5, r0, d.{input_list_address}.dx
    subi r6, r5, IPSEC_SCRATCH_OFFSET 
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc, r5, TABLE_SIZE_512_BITS)
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_ipsec_int, r6, TABLE_SIZE_512_BITS)

    seq c1, d.input_list_address, 0
    b.c1 esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_invalid_barco_req
    nop

    nop.e
    nop
 
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_invalid_barco_req:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwr p.p4_intr_global_drop, 1
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA2_INVALID_BARCO_REQ, 1)

    nop.e
    nop
    
