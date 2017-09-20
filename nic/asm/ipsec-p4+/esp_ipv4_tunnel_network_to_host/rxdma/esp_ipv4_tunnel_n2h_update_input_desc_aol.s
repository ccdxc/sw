#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"


struct rx_table_s3_t0_k k;
struct rx_table_s3_t0_rx_table_s3_t0_cfg_action_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc 
        .param IPSEC_CB_BASE
        .align

esp_ipv4_tunnel_n2h_update_input_desc_aol:
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc[33:6] 
    phvwri p.common_te0_phv_table_raw_table_size, 7
    phvwri p.common_te0_phv_table_lock_en, 0
    add r1, r0, k.ipsec_global_ipsec_cb_index
    sll r1, r1, IPSEC_CB_SHIFT_SIZE
    addi r1, r1, IPSEC_CB_BASE
    phvwr p.common_te0_phv_table_addr, r1 

    phvwr p.barco_desc_in_A0_addr, k.t0_s2s_in_page_addr 
    phvwri p.barco_desc_in_O0, 0
    phvwr p.barco_desc_in_L0, k.ipsec_global_packet_length

dma_cmd_to_move_input_pkt_to_mem:
    phvwri p.dma_cmd_pkt2mem_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, k.t0_s2s_in_page_addr
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_global_packet_length
    nop.e
    nop
