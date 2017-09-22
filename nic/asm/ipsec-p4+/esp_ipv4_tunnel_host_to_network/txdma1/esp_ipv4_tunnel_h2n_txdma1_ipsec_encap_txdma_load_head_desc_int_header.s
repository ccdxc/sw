#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_ipsec_encap_txdma_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header:
    add r1, r0, d.in_desc
    addi r2, r1, 56
    addi r3, r1, 64
    phvwr p.barco_req_input_list_address, r3.dx
    phvwr p.barco_req_status_address, r2.dx
    add r1, r0, d.out_desc 
    addi r1, r1, 64
    phvwr p.barco_req_output_list_address, r1.dx
    add r1, r0, d.in_page 
    phvwr p.barco_req_iv_address, r1.dx 
    add r1, r0, d.pad_size
    add r1, r1, d.tailroom_offset
    addi r1, r1, 2
    addi r1, r1, ESP_FIXED_HDR_SIZE
    add r1, r1, d.out_page
    phvwr p.barco_req_auth_tag_addr, r1.dx
    //add r3, r0, k.t0_s2s_iv_size
    addi r3, r0, ESP_FIXED_HDR_SIZE 
    phvwr p.barco_req_header_size, r3.wx 

    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req[33:6] 
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwr  p.common_te0_phv_table_addr, k.ipsec_to_stage2_barco_req_addr 
    phvwri p.app_header_table1_valid, 0
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_fill_zero_in_out_desc:
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

