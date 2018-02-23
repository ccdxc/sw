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
    add r2, d.in_desc, 56
    add r3, d.in_desc, 64
    phvwr p.barco_req_input_list_address, r3.dx
    phvwr p.barco_req_status_address, r2.dx
    add r1, d.out_desc, 64
    phvwr p.barco_req_output_list_address, r1.dx
    phvwr p.barco_req_iv_address, d.{in_page}.dx 
    add r1, d.pad_size, d.tailroom_offset
    addi r1, r1, 2
    addi r1, r1, ESP_FIXED_HDR_SIZE
    add r1, r1, d.out_page
    phvwr p.barco_req_auth_tag_addr, r1.dx
    phvwri p.barco_req_header_size, ESP_FIXED_HDR_SIZE_LI 
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req[33:6] 
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwr  p.common_te0_phv_table_addr, k.txdma1_global_ipsec_cb_addr
    phvwri p.app_header_table1_valid, 0

esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_fill_zero_in_out_desc:
    add r2, d.in_desc, 96
    add r3, d.out_desc, 96
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

