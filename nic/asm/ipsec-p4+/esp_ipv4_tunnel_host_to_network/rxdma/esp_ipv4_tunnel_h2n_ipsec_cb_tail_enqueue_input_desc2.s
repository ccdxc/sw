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
    add r2, r0, k.t2_s2s_in_desc_addr
    blti  r2, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma
    phvwr p.dma_cmd_phv2mem_ipsec_int_dma_cmd_addr, k.t2_s2s_in_desc_addr

esp_ipv4_tunnel_h2n_dma_cmd_fill_esp_hdr:
    add r1, k.t2_s2s_in_page_addr, IPSEC_SALT_HEADROOM
    add r1, r1, d.iv_size
    blti  r1, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma
    phvwr p.dma_cmd_fill_esp_hdr_dma_cmd_addr, r1
    smeqb c3, d.flags, IPSEC_FLAGS_RANDOM_MASK, IPSEC_FLAGS_RANDOM_MASK
    phvwri.!c3 p.{dma_cmd_fill_esp_hdr_dma_cmd_phv_end_addr...dma_cmd_fill_esp_hdr_dma_cmd_phv_start_addr}, ((IPSEC_ESP_HDR_PHV_END << 10) | IPSEC_ESP_HDR_PHV_START)
    phvwri.c3 p.{dma_cmd_fill_esp_hdr_dma_cmd_phv_end_addr...dma_cmd_fill_esp_hdr_dma_cmd_phv_start_addr}, ((IPSEC_ESP2_HDR_PHV_END << 10) | IPSEC_ESP_HDR_PHV_START)
  
esp_ipv4_tunnel_h2n_dma_cmd_to_write_input_desc_aol:
    add r1, k.t2_s2s_in_desc_addr, 64
    blti  r1, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma
    phvwr p.dma_cmd_in_desc_aol_dma_cmd_addr, r1
    
esp_ipv4_tunnel_h2n_dma_cmd_to_write_output_desc_aol:
    add r1, k.ipsec_to_stage4_out_desc_addr, 64
    blti  r1, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma
    phvwr p.dma_cmd_out_desc_aol_dma_cmd_addr, r1 
    phvwri.e p.{dma_cmd_out_desc_aol_dma_cmd_phv_end_addr...dma_cmd_out_desc_aol_dma_cmd_type}, ((IPSEC_OUT_DESC_AOL_END << 18) | (IPSEC_OUT_DESC_AOL_START << 8) | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    nop

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma:
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    nop.e
    nop
