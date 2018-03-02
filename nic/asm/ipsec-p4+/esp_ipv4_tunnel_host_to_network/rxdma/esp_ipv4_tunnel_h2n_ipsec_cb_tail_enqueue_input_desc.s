#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t0_k k;
struct rx_table_s4_t0_ipsec_cb_tail_enqueue_input_desc_d d;
struct phv_ p;

%%
        .param IPSEC_CB_BASE
        .align

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc:
    phvwr p.ipsec_int_header_in_desc, k.t0_s2s_in_desc_addr
    phvwri p.p4_rxdma_intr_dma_cmd_ptr, H2N_RXDMA_IPSEC_DMA_COMMANDS_OFFSET 
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwr p.esp_header_spi, d.spi
    phvwr p.esp_header_seqno, d.esn_lo
    phvwr p.esp_header_iv, d.iv
    smeqb c3, d.flags, IPSEC_FLAGS_RANDOM_MASK, IPSEC_FLAGS_RANDOM_MASK

esp_ipv4_tunnel_h2n_dma_cmd_to_write_ipsec_int_from_rxdma_to_txdma:
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_phv2mem_ipsec_int_dma_cmd_addr, k.t0_s2s_in_desc_addr
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_start_addr, IPSEC_INT_START_OFFSET
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_end_addr, IPSEC_INT_END_OFFSET

esp_ipv4_tunnel_h2n_dma_cmd_fill_esp_hdr:
    phvwri p.dma_cmd_fill_esp_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, k.t0_s2s_in_page_addr, IPSEC_SALT_HEADROOM
    add r1, r1, d.iv_size
    phvwr p.dma_cmd_fill_esp_hdr_dma_cmd_addr, r1
    phvwri p.dma_cmd_fill_esp_hdr_dma_cmd_phv_start_addr, IPSEC_ESP_HDR_PHV_START
    phvwri.!c3 p.dma_cmd_fill_esp_hdr_dma_cmd_phv_end_addr, IPSEC_ESP_HDR_PHV_END
    phvwri.c3 p.dma_cmd_fill_esp_hdr_dma_cmd_phv_end_addr, IPSEC_ESP2_HDR_PHV_END
  
esp_ipv4_tunnel_h2n_dma_cmd_to_write_input_desc_aol:
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, k.t0_s2s_in_desc_addr, 64
    phvwr p.dma_cmd_in_desc_aol_dma_cmd_addr, r1
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_start_addr, IPSEC_IN_DESC_AOL_START
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_end_addr, IPSEC_IN_DESC_AOL_END
    
esp_ipv4_tunnel_h2n_dma_cmd_to_write_output_desc_aol:
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, k.ipsec_to_stage4_out_desc_addr, 64
    phvwr p.dma_cmd_out_desc_aol_dma_cmd_addr, r1 
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_start_addr, IPSEC_OUT_DESC_AOL_START
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_end_addr, IPSEC_OUT_DESC_AOL_END

esp_ipv4_tunnel_h2n_post_to_cb_ring:
    add r2, r0, d.cb_ring_base_addr_hi, 32
    add r2, r2, d.cb_ring_base_addr 
    sll r3, d.cb_pindex, IPSEC_CB_RING_ENTRY_SHIFT_SIZE
    andi r3, r3, IPSEC_CB_RING_INDEX_MASK
    add r3, r3, r2
    phvwri p.dma_cmd_post_cb_ring_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_post_cb_ring_dma_cmd_addr, r3
    phvwri p.dma_cmd_post_cb_ring_dma_cmd_phv_start_addr, IPSEC_CB_RING_IN_DESC_START
    phvwri p.dma_cmd_post_cb_ring_dma_cmd_phv_end_addr, IPSEC_CB_RING_IN_DESC_END 

esp_ipv4_tunnel_h2n_dma_cmd_incr_pindex:
    tbladd d.cb_pindex, 1
    nop
    tbland d.cb_pindex, 0x3F
    nop
esp_ipv4_tunnel_h2n_rxdma_dma_cmd_ring_doorbell:
    CAPRI_DMA_CMD_RING_DOORBELL2(doorbell_cmd_dma_cmd, LIF_IPSEC_ESP, 0, k.ipsec_global_ipsec_cb_index, 0, d.cb_pindex, db_data_pid, db_data_index) 
    phvwri          p.doorbell_cmd_dma_cmd_eop, 1
    phvwri          p.doorbell_cmd_dma_cmd_wr_fence, 1
    nop.e
    nop

