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
    add r1, r0, d.in_desc
    addi r2, r1, 56
    addi r3, r1, 64
    phvwr p.barco_req_input_list_address, r3.dx
    phvwr p.barco_req_status_address, r2.dx
    add r1, r0, d.out_desc 
    addi r1, r1, 64
    phvwr p.barco_req_output_list_address, r1.dx

    // iv_address = in_page+payload_start-4
    add r1, r0, d.in_page
    add r1, r1, d.headroom_offset
    addi r1, r1, ESP_FIXED_HDR_SIZE 
    phvwr p.barco_req_iv_address, r1.dx 

    add r1, r0, d.tailroom_offset
    add r1, r1, d.in_page
    phvwr p.barco_req_auth_tag_addr, r1.dx

    // Header size is iv_size+ESP_FIXED header size 
    add r3, r0, k.t0_s2s_iv_size
    addi r3, r0, ESP_FIXED_HDR_SIZE 
    phvwr p.barco_req_header_size, r3.wx

    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_pc, esp_v4_tunnel_n2h_write_barco_req[33:6]
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_lock_en, 0
    phvwr p.common_te0_phv_table_addr, d.in_desc 

     
esp_ipv4_tunnel_n2h_txdma1_ipsec_encap_fill_zero_in_out_desc:
    addi r1, r0, 96 
    add r2, r1, d.in_desc
    add r3, r1, d.out_desc

    phvwri p.brq_in_desc_zero_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.brq_in_desc_zero_dma_cmd_addr, r2 
    phvwri p.brq_in_desc_zero_dma_cmd_phv_start_addr, IPSEC_DESC_ZERO_CONTENT_START
    phvwri p.brq_in_desc_zero_dma_cmd_phv_end_addr, IPSEC_DESC_ZERO_CONTENT_END
     
    phvwri p.brq_out_desc_zero_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.brq_out_desc_zero_dma_cmd_addr, r3 
    phvwri p.brq_out_desc_zero_dma_cmd_phv_start_addr, IPSEC_DESC_ZERO_CONTENT_START
    phvwri p.brq_out_desc_zero_dma_cmd_phv_end_addr, IPSEC_DESC_ZERO_CONTENT_END

    nop.e
    nop
