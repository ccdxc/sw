#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_ipsec_encap_txdma_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req
        .param IPSEC_PAGE_ADDR_RX
        .param IPSEC_PAGE_ADDR_TX
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header:
    add r2, d.in_desc, IPSEC_STATUS_ADDR_OFFSET 
    add r3, d.in_desc, IPSEC_SCRATCH_OFFSET 

    bgti r2, IPSEC_PAGE_ADDR_RX, esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_bad_barco_in_desc
      
    phvwr p.barco_req_input_list_address, r3.dx
    phvwr p.barco_req_status_address, r2.dx
    add r1, d.out_desc, 64
    bgti r1, IPSEC_PAGE_ADDR_TX, esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_bad_barco_out_desc
    phvwr p.barco_req_output_list_address, r1.dx
    phvwr p.barco_req_iv_address, d.{in_page}.dx 
    add r1, d.pad_size, d.tailroom_offset
    addi r1, r1, ESP_FIXED_HDR_SIZE+2
    add r1, r1, d.out_page
    phvwr p.ipsec_to_stage5_in_desc_addr, d.in_desc
    phvwr p.ipsec_to_stage5_out_desc_addr, d.out_desc
    phvwr p.barco_req_auth_tag_addr, r1.dx
    add r5, r0, k.txdma1_global_ipsec_cb_addr
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req, r5, TABLE_SIZE_512_BITS)
    nop.e
    nop 

esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_bad_barco_in_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_BAD_BARCO_IN_DESC, 1)
    //add r6, k.txdma1_global_ipsec_cb_addr, 63
    //memwr.b r6, 0xFF 
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_bad_barco_out_desc:
    //add r6, k.txdma1_global_ipsec_cb_addr, 63
    //memwr.b r6, 0xFF 
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_BAD_BARCO_OUT_DESC, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

