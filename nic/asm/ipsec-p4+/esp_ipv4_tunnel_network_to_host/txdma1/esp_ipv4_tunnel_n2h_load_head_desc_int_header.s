#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_write_barco_req
        .param IPSEC_PAGE_ADDR_RX
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .align
esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header:

    phvwr p.ipsec_to_stage5_in_desc_addr, d.in_desc
    phvwr p.ipsec_to_stage5_out_desc_addr, d.out_desc
    
    add r2, d.in_desc, IPSEC_STATUS_ADDR_OFFSET 
    add r3, d.in_desc, IPSEC_SCRATCH_OFFSET 
    bgti r2, IPSEC_PAGE_ADDR_RX, esp_ipv4_tunnel_n2h_txdma1_bad_barco_in_desc
    nop
    phvwr p.barco_req_input_list_address, r3.dx
    phvwr p.barco_req_status_address, r2.dx
    add r1, d.out_desc, IPSEC_SCRATCH_OFFSET 
    bgti r1, IPSEC_PAGE_ADDR_RX, esp_ipv4_tunnel_n2h_txdma1_bad_barco_out_desc
    nop
    phvwr p.barco_req_output_list_address, r1.dx

    seq c1, d.spi, k.ipsec_to_stage2_spi
    phvwri.!c1 p.ipsec_to_stage3_new_key, 1 
     
    // iv_address = in_page+payload_start-4
    add r1, d.in_page, d.headroom_offset
    addi r1, r1, ESP_FIXED_HDR_SIZE 
    phvwr p.barco_req_iv_address, r1.dx 

    add r1, d.tailroom_offset, d.in_page
    phvwr p.barco_req_auth_tag_addr, r1.dx

    add r5, r0, k.txdma1_global_ipsec_cb_addr
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_v4_tunnel_n2h_write_barco_req, r5, TABLE_SIZE_512_BITS)
    nop.e
    nop

esp_ipv4_tunnel_n2h_txdma1_bad_barco_in_desc:
    //add r6, k.txdma1_global_ipsec_cb_addr, 63
    //memwr.b r6, 0xFF 
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_BAD_BARCO_IN_DESC, 1)
    phvwri.e p.txdma1_global_flags, 1
    nop

esp_ipv4_tunnel_n2h_txdma1_bad_barco_out_desc:
    //add r6, k.txdma1_global_ipsec_cb_addr, 63
    //memwr.b r6, 0xFF 
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_BAD_BARCO_OUT_DESC, 1)
    phvwri.e p.txdma1_global_flags, 1
    nop

