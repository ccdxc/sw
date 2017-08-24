#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t0_k k;
struct rx_table_s4_t0_esp_v4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_d d;
struct phv_ p;

%%
        .align

esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc:
    phvwr p.ipsec_int_header_in_desc, k.t0_s2s_in_desc_addr

dma_cmd_to_move_input_pkt_to_mem:
    phvwri p.dma_cmd_pkt2mem_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT 
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, k.t0_s2s_in_page_addr
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, d.length0

dma_cmd_to_write_ipsec_int_from_rxdma_to_txdma:
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_phv2mem_ipsec_int_dma_cmd_addr, k.t0_s2s_in_desc_addr
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_start_addr, IPSEC_INT_START_OFFSET
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_phv_end_addr, IPSEC_INT_END_OFFSET

dma_cmd_to_write_input_desc_aol:
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, r0, k.t0_s2s_in_desc_addr
    addi r1, r1, 64
    phvwr p.dma_cmd_in_desc_aol_dma_cmd_addr, r1
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_start_addr, IPSEC_IN_DESC_AOL_START
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_phv_end_addr, IPSEC_IN_DESC_AOL_END
    
dma_cmd_to_write_output_desc_aol:
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_out_desc_aol_dma_cmd_addr, k.ipsec_to_stage4_out_desc_addr
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_start_addr, IPSEC_IN_DESC_AOL_START
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_phv_end_addr, IPSEC_IN_DESC_AOL_END
 
dma_cmd_to_update_tail_desc:
    phvwri p.dma_cmd_tail_desc_addr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r1, r0, k.ipsec_global_ipsec_cb_index
    sll r1, r1, IPSEC_CB_SHIFT_SIZE
    addi r1, r1, IPSEC_CB_BASE
    phvwr p.dma_cmd_tail_desc_addr_dma_cmd_type, r1 
    phvwri p.dma_cmd_tail_desc_addr_dma_cmd_phv_start_addr, IPSEC_IN_DESC_AOL_START
    phvwri p.dma_cmd_tail_desc_addr_dma_cmd_phv_end_addr, IPSEC_IN_DESC_AOL_END

