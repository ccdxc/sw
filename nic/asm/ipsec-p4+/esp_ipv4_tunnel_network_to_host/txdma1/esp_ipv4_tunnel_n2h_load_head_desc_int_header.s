#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_write_barco_req
        .align
esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header:
    add r2, d.in_desc, 56
    add r3, d.in_desc, 64
    phvwr p.barco_req_input_list_address, r3.dx
    phvwr p.barco_req_status_address, r2.dx
    add r1, d.out_desc, 64
    phvwr p.barco_req_output_list_address, r1.dx

    seq c1, d.spi, k.ipsec_to_stage2_spi
    phvwri.!c1 p.ipsec_to_stage3_new_key, 1 
     
    // iv_address = in_page+payload_start-4
    add r1, d.in_page, d.headroom_offset
    addi r1, r1, ESP_FIXED_HDR_SIZE 
    phvwr p.barco_req_iv_address, r1.dx 

    add r1, d.tailroom_offset, d.in_page
    phvwr p.barco_req_auth_tag_addr, r1.dx

    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_pc, esp_v4_tunnel_n2h_write_barco_req[33:6]
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14 
    phvwr p.common_te0_phv_table_addr, k.txdma1_global_ipsec_cb_addr 

     
esp_ipv4_tunnel_n2h_txdma1_ipsec_encap_fill_zero_in_out_desc:
    add r2, d.in_desc, 96
    add r3, d.out_desc, 96

    phvwr p.brq_in_desc_zero_dma_cmd_addr, r2 
     
    phvwr p.brq_out_desc_zero_dma_cmd_addr, r3 

    nop.e
    nop
