#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s3_t3_k k;
struct rx_table_s3_t3_update_input_desc_aol2_d d;
struct phv_ p;

%%
        .align
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
esp_ipv4_tunnel_h2n_update_input_desc_aol2:

dma_cmd_to_move_input_pkt_to_mem:
    phvwri p.app_header_table3_valid, 0
    add r1, k.ipsec_to_stage3_iv_size, IPSEC_SALT_HEADROOM+ESP_FIXED_HDR_SIZE
    add r1, r1, k.ipsec_to_stage3_iv_size
    add r1, r1, k.t3_s2s_in_page_addr
    blti  r1, CAPRI_HBM_BASE,esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_in_page 
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, r1 
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_to_stage3_packet_len[13:0]

dma_cmd_to_write_pad_bytes:
    seq c3, k.ipsec_to_stage3_pad_size, 0
    add r2, r1, k.ipsec_to_stage3_packet_len
    //source - from fixed hbm pad table data structure.
    phvwri.!c3 p.{dma_cmd_pad_byte_src_dma_cmd_mem2mem_type...dma_cmd_pad_byte_src_dma_cmd_type}, ((DMA_CMD_TYPE_MEM2MEM_TYPE_SRC << 4) | CAPRI_DMA_COMMAND_MEM_TO_MEM)
    add r1, r0, k.ipsec_to_stage3_pad_addr[51:0]
    blti  r1, CAPRI_HBM_BASE,esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_pad_bytes
 
    phvwr  p.dma_cmd_pad_byte_src_dma_cmd_addr, k.ipsec_to_stage3_pad_addr[51:0] 
    phvwr p.dma_cmd_pad_byte_src_dma_cmd_size, k.ipsec_to_stage3_pad_size
    //destination - to the end of the page where pkt is DMA'ed.
    phvwri.!c3 p.{dma_cmd_pad_byte_dst_dma_cmd_mem2mem_type...dma_cmd_pad_byte_dst_dma_cmd_type}, ((DMA_CMD_TYPE_MEM2MEM_TYPE_DST << 4) | CAPRI_DMA_COMMAND_MEM_TO_MEM)
    blti  r2, CAPRI_HBM_BASE,esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_pad_bytes
    phvwr p.dma_cmd_pad_byte_dst_dma_cmd_addr, r2
    phvwr p.dma_cmd_pad_byte_dst_dma_cmd_size, k.ipsec_to_stage3_pad_size

dma_cmd_to_write_pad_size_l4_proto:
    add r2, r2, k.ipsec_to_stage3_pad_size
    blti  r2, CAPRI_HBM_BASE,esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_tail_bytes
    phvwr p.tail_2_bytes_dma_cmd_addr, r2

dma_cmd_write_salt_to_in_desc:
    phvwr.e p.dma_cmd_iv_salt_dma_cmd_addr, k.t3_s2s_in_page_addr[51:0]
    nop




esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_in_page:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_T3_IN_PAGE_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_pad_bytes:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_PAD_ADDR_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_tail_bytes:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TAIL_BYTES_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

