
struct d_struct {
    out_desc_a0 : ADDRESS_WIDTH;
    out_desc_o0 : 32;
    out_desc_l0 : 32;
    out_desc_a1 : ADDRESS_WIDTH;
    out_desc_o1 : 32;
    out_desc_l1 : 32;
    out_desc_a2 : ADDRESS_WIDTH;
    out_desc_o2 : 32;
    out_desc_l2 : 32;
    out_desc_next_ptr : ADDRESS_WIDTH;
    out_desc_rsvd_bits : 64;
}

struct k_struct {
    out_desc_addr : ADDRESS_WIDTH;
    out_page_addr : ADDRESS_WIDTH;
};

struct p_struct p       ;
struct k_struct k       ;
struct d_struct d       ;


ipsec_out_desc_read_aol:
    phvwr p.out_desc_a0, k.out_desc_addr
    phvwri p.out_desc_o0, 0
    phvwri p.out_desc_l0, 0
    phvwri p.out_desc_a1, 0 
    phvwri p.out_desc_o1, 0
    phvwri p.out_desc_l1, 0
    phvwri p.out_desc_a2, 0 
    phvwri p.out_desc_o2, 0
    phvwri p.out_desc_l2, 0
    phvwri p.out_desc_next_ptr, 0
    phvwri p.out_desc_rsvd_bits, 0
    
ipsec_out_desc_write_aol:
    phvwri p.dma_cmd3_type, DMA_CMD_TYPE_PHV2MEM
    phvwri p.dma_cmd3_host_addr, 0
    phvwri p.dma_cmd3_addr, out_desc_addr
    phvwri p.dma_cmd3_phv_start_addr, PHV_RXDMA_BARCO_AOL_OFFSET_BEGIN
    phvwri p.dma_cmd3_phv_end_addr, PHV_RXDMA_BARCO_AOL_OFFSET_END  
 

