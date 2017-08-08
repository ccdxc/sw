#include "../../assembly.h"

struct d_struct {
    in_desc_a0 : ADDRESS_WIDTH;
    in_desc_o0 : 32;
    in_desc_l0 : 32;
    in_desc_a1 : ADDRESS_WIDTH;
    in_desc_o1 : 32;
    in_desc_l1 : 32;
    in_desc_a2 : ADDRESS_WIDTH;
    in_desc_o2 : 32;
    in_desc_l2 : 32;
    in_desc_next_ptr : ADDRESS_WIDTH;
    in_desc_rsvd_bits : 64;
};

struct k_struct {
    in_desc_addr : ADDRESS_WIDTH;
    in_page_addr : ADDRESS_WIDTH;
    intrinsic_frame_size : 14;
    ipsec_int_pad_size : 8;
};

struct p_struct p       ;
struct k_struct k       ;
struct d_struct d       ;

ipsec_in_desc_read_aol:
    phvwr p.in_desc_a0, k.in_page_addr
    phvwri p.in_desc_o0, 0
    // subtract intrinsics from frame_size
    add r1, r0, k.intrinsic_frame_size
    subi r1, r1, RXDMA_IPSEC_HW_SW_INTRINSIC
    phvwri p.in_desc_l0, r1 
    phvwri p.in_desc_a1, 0 
    phvwri p.in_desc_o1, 0
    phvwri p.in_desc_l1, 0
    phvwri p.in_desc_a2, 0 
    phvwri p.in_desc_o2, 0
    phvwri p.in_desc_l2, 0
    phvwri p.in_desc_next_ptr, 0
    phvwri p.in_desc_rsvd_bits, 0

ipsec_in_desc_write_aol:
    phvwri p.dma_cmd2_type, DMA_CMD_TYPE_PHV2MEM
    phvwri p.dma_cmd2_host_addr, 0
    addi   r1, k.in_desc_addr, 64 
    phvwri p.dma_cmd2_addr, r1 
    phvwri p.dma_cmd2_phv_start_addr, PHV_RXDMA_BARCO_AOL_OFFSET_BEGIN
    phvwri p.dma_cmd2_phv_end_addr, PHV_RXDMA_BARCO_AOL_OFFSET_END

dma_cmd_to_move_input_pkt_to_mem:
    phvwri p.dma_cmd0_type, DMA_CMD_TYPE_PKT2MEM
    phvwri p.dma_cmd0_host_addr, 0
    phvwri p.dma_cmd0_addr, k.in_page_addr
    phvwri p.dma_cmd0_size, k.intrinsic_frame_size

dma_cmd_to_write_pad_bytes:
    add r1, r0, k.ipsec_int_pad_size
    sll r1, r1, 8
    addi r1, r1, IPSEC_PAD_BYTES_HBM_TABLE_BASE
    add r2, r0, k.in_page_addr 
    add r2, r2, k.intrinsic_frame_size
    //source - from fixed hbm pad table data structure.
    phvwri p.dma_cmd4_type, DMA_CMD_TYPE_MEM2MEM
    phvwri p.dma_cmd4_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_SRC
    phvwri p.dma_cmd4_host_addr, 0
    phvwri p.dma_cmd4_addr, r1
    phvwri p.dma_cmd4_size, k.ipsec_int_pad_size
    //destination - to the end of the page where pkt is DMA'ed.
    phvwri p.dma_cmd5_type, DMA_CMD_TYPE_MEM2MEM
    phvwri p.dma_cmd5_mem2mem_type, DMA_CMD_TYPE_MEM2MEM_TYPE_DST
    phvwri p.dma_cmd5_host_addr, 0
    phvwri p.dma_cmd5_addr, r2
    phvwri p.dma_cmd5_size, k.ipsec_int_pad_size

dma_cmd_to_write_ipsec_int_from_rxdma_to_txdma:
    phvwri p.dma_cmd1_type, DMA_CMD_TYPE_PHV2MEM
    phvwri p.dma_cmd1_host_addr, 0
    phvwri p.dma_cmd1_addr, r1 
    phvwri p.dma_cmd1_phv_start_addr, PHV_RXDMA_IPSEC_INT_HDR_OFFSET_BEGIN
    phvwri p.dma_cmd1_phv_end_addr, PHV_RXDMA_IPSEC_INT_HDR_OFFSET_END 
 


