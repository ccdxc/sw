#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"


struct rx_table_s3_t0_k k;
struct rx_table_s3_t0_rx_table_s3_t0_cfg_action_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc 
        .align

esp_ipv4_tunnel_n2h_update_input_desc_aol:
    phvwri p.p42p4plus_hdr_table0_valid, 1
    addi r2, r0, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc 
    srl r2, r2, 6
    phvwr p.common_te0_phv_table_pc, r2 
    phvwri p.common_te0_phv_table_raw_table_size, 7
    phvwri p.common_te0_phv_table_lock_en, 0
    add r1, r0, k.ipsec_global_ipsec_cb_index
    sll r1, r1, IPSEC_CB_SIZE_SHIFT
    addi r1, r1, IPSEC_CB_BASE
    phvwr p.common_te0_phv_table_addr, r1 

    phvwr p.barco_desc_in_A0_addr, k.t0_s2s_in_page_addr 
    // get the correct way of giving it as a single 14 bit field
    //phvwr p.barco_desc_in_L0, k.ipsec_global_frame_size
    phvwri p.barco_desc_in_O0, 0

    phvwri p.barco_desc_in_A1_addr, 0
    phvwri p.barco_desc_in_L1, 0
    phvwri p.barco_desc_in_O1, 0

    phvwri p.barco_desc_in_A2_addr, 0
    phvwri p.barco_desc_in_L2, 0
    phvwri p.barco_desc_in_O2, 0

    phvwri p.barco_desc_in_NextAddr, 0
    phvwri p.barco_desc_in_Reserved, 0

dma_cmd_to_move_input_pkt_to_mem:
    phvwri p.dma_cmd_pkt2mem_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, k.t0_s2s_in_page_addr
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_global_packet_length
