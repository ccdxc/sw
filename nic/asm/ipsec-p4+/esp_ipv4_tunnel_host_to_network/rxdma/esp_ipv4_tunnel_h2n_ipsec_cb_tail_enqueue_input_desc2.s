#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t2_k k;
struct rx_table_s4_t2_ipsec_cb_tail_enqueue_input_desc2_d d;
struct phv_ p;

%%
        .param IPSEC_CB_BASE
        .align

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2:

esp_ipv4_tunnel_h2n_dma_cmd_to_write_ipsec_int_from_rxdma_to_txdma:
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_phv2mem_ipsec_int_dma_cmd_addr, k.t2_s2s_in_desc_addr
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_start_addr, IPSEC_INT_START_OFFSET
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_end_addr, IPSEC_INT_END_OFFSET

esp_ipv4_tunnel_h2n_dma_cmd_fill_esp_hdr:
    phvwri p.dma_cmd_fill_esp_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, k.t2_s2s_in_page_addr, IPSEC_SALT_HEADROOM
    add r1, r1, d.iv_size
    phvwr p.dma_cmd_fill_esp_hdr_dma_cmd_addr, r1
    phvwri p.dma_cmd_fill_esp_hdr_dma_cmd_phv_start_addr, IPSEC_ESP_HDR_PHV_START
    smeqb c3, d.flags, IPSEC_FLAGS_RANDOM_MASK, IPSEC_FLAGS_RANDOM_MASK
    phvwri.!c3 p.dma_cmd_fill_esp_hdr_dma_cmd_phv_end_addr, IPSEC_ESP_HDR_PHV_END
    phvwri.c3 p.dma_cmd_fill_esp_hdr_dma_cmd_phv_end_addr, IPSEC_ESP2_HDR_PHV_END
  
esp_ipv4_tunnel_h2n_dma_cmd_to_write_input_desc_aol:
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, k.t2_s2s_in_desc_addr, 64
    phvwr p.dma_cmd_in_desc_aol_dma_cmd_addr, r1
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_start_addr, IPSEC_IN_DESC_AOL_START
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_end_addr, IPSEC_IN_DESC_AOL_END
    
esp_ipv4_tunnel_h2n_dma_cmd_to_write_output_desc_aol:
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, k.ipsec_to_stage4_out_desc_addr, 64
    phvwr p.dma_cmd_out_desc_aol_dma_cmd_addr, r1 
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_start_addr, IPSEC_OUT_DESC_AOL_START
    phvwri.f p.dma_cmd_out_desc_aol_dma_cmd_phv_end_addr, IPSEC_OUT_DESC_AOL_END
    nop.e
    nop

