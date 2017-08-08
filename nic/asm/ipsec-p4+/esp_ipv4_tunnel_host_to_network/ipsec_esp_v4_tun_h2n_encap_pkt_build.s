struct d_struct {
    ipsec_cb_key_negotiated : 1;
    ipsec_cb_rekey_in_progress : 1;
    ipsec_cb_key_index : 32;
    ipsec_cb_iv_size : 8;
    ipsec_cb_icv_size : 8;
    ipsec_cb_spi : 32;
    ipsec_cb_esn_lo : 32;
    ipsec_cb_iv : 64;
    ipsec_cb_esn_hi : 32;
    ipsec_cb_barco_enc_cmd : 32;
    ipsec_cb_ipsec_cb_index : 12;
    ipsec_cb_block_size : 8;
    // some pad 
};

struct k_struct {
    ipsec_int_headroom_offset : 16;
    in_desc_entry_addr        : ADDRESS_WIDTH;
    ipsec_int_ipsec_cb_index  : 12;
    ipsec_int_iv_size         : 8;
    ipsec_int_icv_size        : 8;
    ipsec_int_pad_size        : 8;
}; 

struct p_struct p       ;
struct k_struct k       ;
struct d_struct d       ;

%%


ipsec_esp_v4_tun_h2n_encap_pkt_build:
    // First add the intrinsic+p4plus2p4_hdr in one shot
    phvwri p.dma_cmd0_cmd, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.dma_cmd0_size, IPSEC_PHV_HEADROOM_RX
    phvwr p.dma_cmd0_addr, PHV_BASE_START
    // Add the input-descriptors ethernet+optional-vlan+outer-IP headers
    phvwri p.dma_cmd1_cmd, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.dma_cmd1_size,k.ipsec_int_headroom_offset
    phvwr p.dma_cmd1_addr, k.in_desc_entry_addr
    // Add the ESP header and IV
    // First get to the ipsec-cb and ipsec-cb is organized such that spi,seqno-low,iv are continguous
    add r1, r0, k.ipsec_int_ipsec_cb_index 
    sll r1, r1, IPSEC_CB_ENTRY_SIZE
    addi r1, r1, IPSEC_CB_BASE_TABLE_ADDR
    add r1, r0, IPSEC_CB_SEQNO_OFFSET
    add r2, r0, k.ipsec_int_iv_size 
    addi r2, r2, ESP_BASE
    phvwri p.dma_cmd2_cmd CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.dma_cmd2_size, r2
    phvwr p.dma_cmd2_addr, r1
    // add the payload now
    add r1, k.ipsec_int_payload_size, k.ipsec_int_pad_size;
    phvwr p.dma_cmd3_cmd CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.dma_cmd3_size, r1 
    phvwr p.dma_cmd3_addr, k.out_desc_entry_addr
    // add the 2 bytes
    phvwri p.dma_cmd4_cmd, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.dma_cmd4_size, 2
    //make sure pad-size and l4 protocol are contiguous in phv
    phvwr p.dma_cmd4_addr, PHV_OFFSET_PAD_SIZE 
    // add the icv - ICV is written to the end of output descriptor starting from tailroom-offset by Barco
    add r1, k.out_desc_addr, k.tailroom_offset
    addi r1, r1, 2 // pls recheck if tailroom offset includes 2 bytes or not
    phvwr p.dma_cmd5_addr, r1
    phvwr p.dma_cmd5_size, k.ipsec_int_icv_size
    phvwri p.dma_cmd5_cmd, CAPRI_DMA_COMMAND_MEM_TO_PKT 
    //increment seqno and iv - should we do this from stats ??
    tblmincri d.ipsec_cb_esn_lo, 1
    tblmincri d.ipsec_cb_iv, 1
     
