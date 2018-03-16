#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s3_t1_k k;
struct rx_table_s3_t1_rx_table_s3_t1_cfg_action_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc
        .align

esp_ipv4_tunnel_n2h_update_output_desc_aol:
    phvwri p.{app_header_table0_valid...app_header_table1_valid}, 2
    phvwr p.barco_desc_out_A0_addr, k.{t1_s2s_out_page_addr}.dx 
    // get the correct way of giving it as a single 14 bit field
    add r2, k.ipsec_to_stage3_payload_size, ESP_FIXED_HDR_SIZE
    phvwr p.barco_desc_out_L0, r2.wx 

    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc[33:6]
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14
    phvwr p.common_te0_phv_table_addr, k.ipsec_to_stage3_ipsec_cb_addr


    phvwri p.p4_rxdma_intr_dma_cmd_ptr, N2H_RXDMA_IPSEC_DMA_COMMANDS_OFFSET
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri p.dma_cmd_post_cb_ring_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    nop.e
    nop
