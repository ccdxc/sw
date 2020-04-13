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
    add r6, k.ipsec_global_in_desc_addr, IPSEC_PAGE_OFFSET
    add r1, r1, r6 
    blti  r1, ASIC_HBM_BASE,esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_in_page 
    nop
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, r1 
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_to_stage3_packet_len[13:0]

    seq c3, k.ipsec_to_stage3_pad_size, 0
    add r2, r1, k.ipsec_to_stage3_packet_len

    addui r4, r0, (IPSEC_PAD_BYTES_0_7 >> 32)
    addi  r4, r4, (IPSEC_PAD_BYTES_0_7 & 0xFFFFFFFF) 
    phvwr p.pad_bytes_pad_bytes_0_7, r4
    
    addui r4, r0, (IPSEC_PAD_BYTES_8_15 >> 32)
    addi  r4, r4, (IPSEC_PAD_BYTES_8_15 & 0xFFFFFFFF) 
    phvwr p.pad_bytes_pad_bytes_8_15, r4
    
    CAPRI_DMA_CMD_PHV2MEM_SETUP_WITH_LEN(dma_cmd_pad_mem_dma_cmd, r2, pad_bytes_pad_bytes_0_7, k.ipsec_to_stage3_pad_size) 
    phvwri p.dma_cmd_pad_mem_dma_cmd_cache, 1
    add r2, r2, k.ipsec_to_stage3_pad_size
    blti  r2, ASIC_HBM_BASE,esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_tail_bytes
    nop
    phvwr p.tail_2_bytes_dma_cmd_addr, r2

    blti  r2, ASIC_HBM_BASE, esp_ipv4_tunnel_h2n_update_input_desc_aol2_illegal_dma_in_page
    nop
    phvwr.e p.dma_cmd_iv_salt_dma_cmd_addr, r6 
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

