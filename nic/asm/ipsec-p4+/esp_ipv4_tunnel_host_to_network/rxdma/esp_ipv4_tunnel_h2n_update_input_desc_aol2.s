#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s3_t3_k k;
struct rx_table_s3_t3_update_input_desc_aol2_d d;
struct phv_ p;

%%
        .align

esp_ipv4_tunnel_h2n_update_input_desc_aol2:

dma_cmd_to_move_input_pkt_to_mem:
    phvwri p.app_header_table3_valid, 0
    phvwri p.dma_cmd_pkt2mem_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    add r1, k.ipsec_to_stage3_iv_size, IPSEC_SALT_HEADROOM
    addi r1, r1, ESP_FIXED_HDR_SIZE
    add r1, r1, k.ipsec_to_stage3_iv_size
    add r1, r1, k.t3_s2s_in_page_addr 
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, r1 
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_to_stage3_packet_len[13:0]

dma_cmd_to_write_pad_bytes:
    seq c3, k.ipsec_to_stage3_pad_size, 0
    add r2, r1, k.ipsec_to_stage3_packet_len
    //source - from fixed hbm pad table data structure.
    phvwri.!c3 p.dma_cmd_pad_byte_src_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_MEM
    phvwri.!c3 p.dma_cmd_pad_byte_src_dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_SRC
    phvwr  p.dma_cmd_pad_byte_src_dma_cmd_addr, k.ipsec_to_stage3_pad_addr[51:0] 
    phvwr p.dma_cmd_pad_byte_src_dma_cmd_size, k.ipsec_to_stage3_pad_size
    //destination - to the end of the page where pkt is DMA'ed.
    phvwri.!c3 p.dma_cmd_pad_byte_dst_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_MEM 
    phvwri.!c3 p.dma_cmd_pad_byte_dst_dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_DST
    phvwr p.dma_cmd_pad_byte_dst_dma_cmd_addr, r2
    phvwr p.dma_cmd_pad_byte_dst_dma_cmd_size, k.ipsec_to_stage3_pad_size

dma_cmd_to_write_pad_size_l4_proto:
    phvwri p.tail_2_bytes_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    add r2, r2, k.ipsec_to_stage3_pad_size
    phvwr p.tail_2_bytes_dma_cmd_addr, r2
    phvwri p.tail_2_bytes_dma_cmd_phv_start_addr, IPSEC_TAIL_2_BYTES_PHV_START
    phvwri p.tail_2_bytes_dma_cmd_phv_end_addr, IPSEC_TAIL_2_BYTES_PHV_END

dma_cmd_write_salt_to_in_desc:
    phvwri p.dma_cmd_iv_salt_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_iv_salt_dma_cmd_addr, k.t3_s2s_in_page_addr[51:0]
    phvwri p.dma_cmd_iv_salt_dma_cmd_phv_start_addr, IPSEC_IN_DESC_IV_SALT_START
    phvwri p.dma_cmd_iv_salt_dma_cmd_phv_end_addr, IPSEC_IN_DESC_IV_SALT_END 
    nop.e 
    nop

