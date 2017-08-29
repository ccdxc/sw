#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"
#include "capri-macros.h"

struct rx_table_s3_t0_k k;
struct rx_table_s3_t0_update_input_desc_aol_d d;
struct phv_ p;

%%
        .param ipsec_cb_tail_enqueue_input_desc
        .align

update_input_desc_aol:
    phvwri p.p42p4plus_hdr_table0_valid, 1
    addi r2, r0, ipsec_cb_tail_enqueue_input_desc
    srl r2, r2, 6
    phvwr p.common_te0_phv_table_pc, r2 
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_lock_en, 0
    add r3, r0, k.ipsec_global_ipsec_cb_index
    sll r3, r3, IPSEC_CB_SIZE_SHIFT
    addi r3, r3, IPSEC_CB_BASE
    phvwr p.common_te0_phv_table_addr, r3 

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
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, d.length0

dma_cmd_to_write_pad_bytes:
    add r1, r0, k.ipsec_to_stage3_pad_size
    sll r1, r1, 8
    addi r1, r1, IPSEC_PAD_BYTES_HBM_TABLE_BASE
    add r2, r0, k.t0_s2s_in_page_addr
    //add r2, r2, k.ipsec_global_frame_size
    //source - from fixed hbm pad table data structure.
    phvwri p.dma_cmd_pad_byte_src_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_MEM
    phvwri p.dma_cmd_pad_byte_src_dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_SRC
    phvwri p.dma_cmd_pad_byte_src_dma_cmd_host_addr, 0
    phvwr  p.dma_cmd_pad_byte_src_dma_cmd_addr, r1
    phvwr p.dma_cmd_pad_byte_src_dma_cmd_size, k.ipsec_to_stage3_pad_size
    //destination - to the end of the page where pkt is DMA'ed.
    phvwri p.dma_cmd_pad_byte_dst_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_MEM 
    phvwri p.dma_cmd_pad_byte_dst_dma_cmd_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_DST
    phvwri p.dma_cmd_pad_byte_dst_dma_cmd_host_addr, 0
    phvwr p.dma_cmd_pad_byte_dst_dma_cmd_addr, r2
    phvwr p.dma_cmd_pad_byte_dst_dma_cmd_size, k.ipsec_to_stage3_pad_size

