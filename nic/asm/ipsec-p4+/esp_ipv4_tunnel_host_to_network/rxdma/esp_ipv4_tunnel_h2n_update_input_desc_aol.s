#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s3_t0_k k;
struct rx_table_s3_t0_update_input_desc_aol_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc
        .param IPSEC_CB_BASE
        .align

esp_ipv4_tunnel_h2n_update_input_desc_aol:
    phvwri p.app_header_table0_valid, 1
    phvwri p.app_header_table2_valid, 0
    phvwri p.app_header_table3_valid, 0
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc[33:6] 
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_lock_en, 0
    add r3, r0, k.{ipsec_global_ipsec_cb_addr_sbit0_ebit31...ipsec_global_ipsec_cb_addr_sbit32_ebit33}
    phvwr p.common_te0_phv_table_addr, r3 

    phvwr p.barco_desc_in_A0_addr, k.t0_s2s_in_page_addr
    add r4, r0, k.ipsec_to_stage3_packet_len
    add r4, r4, k.ipsec_to_stage3_pad_size 
    phvwr p.barco_desc_in_L0, r4
    add r4, r0, k.ipsec_to_stage3_iv_size
    addi r4, r4, IPSEC_SALT_HEADROOM  
    phvwr p.barco_desc_in_O0, r4 

dma_cmd_to_move_input_pkt_to_mem:
    phvwri p.dma_cmd_pkt2mem_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    add r1, r0, k.ipsec_to_stage3_iv_size
    addi r1, r1, IPSEC_SALT_HEADROOM
    //addi r1, r0, 64
    add r1, r1, k.t0_s2s_in_page_addr 
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, r1 
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_to_stage3_packet_len

dma_cmd_to_write_pad_bytes:
    add r2, r1, k.ipsec_to_stage3_packet_len

    //source - from fixed hbm pad table data structure.
    phvwri p.dma_cmd_pad_byte_src_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_MEM
    phvwri p.dma_cmd_pad_byte_src_dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_SRC
    phvwri p.dma_cmd_pad_byte_src_dma_cmd_host_addr, 0
    phvwr  p.dma_cmd_pad_byte_src_dma_cmd_addr, k.ipsec_to_stage3_pad_addr 
    phvwr p.dma_cmd_pad_byte_src_dma_cmd_size, k.ipsec_to_stage3_pad_size
    //destination - to the end of the page where pkt is DMA'ed.
    phvwri p.dma_cmd_pad_byte_dst_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_MEM 
    phvwri p.dma_cmd_pad_byte_dst_dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_DST
    phvwri p.dma_cmd_pad_byte_dst_dma_cmd_host_addr, 0
    phvwr p.dma_cmd_pad_byte_dst_dma_cmd_addr, r2
    phvwr p.dma_cmd_pad_byte_dst_dma_cmd_size, k.ipsec_to_stage3_pad_size

dma_cmd_write_salt_to_in_desc:
    phvwri p.dma_cmd_iv_salt_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_iv_salt_dma_cmd_addr, k.t0_s2s_in_page_addr
    phvwri p.dma_cmd_iv_salt_dma_cmd_phv_start_addr, IPSEC_IN_DESC_IV_SALT_START
    phvwri p.dma_cmd_iv_salt_dma_cmd_phv_end_addr, IPSEC_IN_DESC_IV_SALT_END 

dma_cmd_write_iv_to_in_desc:
    add r1, r0, k.ipsec_to_stage3_iv_size
    add r2, r0, k.{ipsec_global_ipsec_cb_addr_sbit0_ebit31...ipsec_global_ipsec_cb_addr_sbit32_ebit33}
    addi r2, r2, IPSEC_CB_IV_OFFSET
    add r3, r0,  k.t0_s2s_in_page_addr
    addi r3, r3, IPSEC_SALT_HEADROOM
    phvwri p.dma_cmd_iv_src_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_MEM
    phvwri p.dma_cmd_iv_src_dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_SRC
    phvwr p.dma_cmd_iv_src_dma_cmd_addr, r2
    phvwr p.dma_cmd_iv_src_dma_cmd_size, r1
    phvwri p.dma_cmd_iv_dst_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_MEM
    phvwri p.dma_cmd_iv_dst_dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_DST
    phvwr p.dma_cmd_iv_dst_dma_cmd_addr, r3
    phvwr p.dma_cmd_iv_dst_dma_cmd_size, r1
    nop.e 
    nop
