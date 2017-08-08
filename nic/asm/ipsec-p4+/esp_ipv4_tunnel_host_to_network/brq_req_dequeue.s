
struct d_struct {
        brq_in_addr                         : ADDRESS_WIDTH;
        brq_out_addr                        : ADDRESS_WIDTH;
        brq_barco_enc_cmd                   : 32;
        brq_key_index                       : 32;
        brq_iv_addr                         : ADDRESS_WIDTH;
        brq_auth_tag_addr                   : ADDRESS_WIDTH;
        brq_hdr_size                        : 32;
        brq_status                          : 32;
        brq_opq_tag_value                   : 32;
        brq_db_en                           : 1;
        brq_opq_tag_en                      : 1;
        brq_rsvd                            : 30;
        brq_sec_sz                          : 16;
        brq_sec_num                         : 32;
        brq_app_tag                         : 16;

};


struct k_struct {
    brq_consumer_idx                : RING_INDEX_WIDTH ;
};

struct p_struct p       ;
struct k_struct k       ;
struct d_struct d       ;

%%
// reading c_index of barco_req_semaphore is done to get the desc-addr in barco ring(stage0).
// reading the desc-addr of barco, gives the barco request structure (stage1)
// in_desc and out_desc inside barco request structure need to be loaded to read the contents in next stage (stage2)
// ipsec_internal header accumulated during rxdma processing is loaded into phv
ipsec_esp_ipv4_tunnel_h2n_brq_dequeue:
    phvwr p.brq_in_addr, k.ipsec_int_in_desc
    phvwr p.brq_out_addr, k.ipsec_int_out_desc
    // read input-descriptor for the ipsec-internal-header accumlated during pre-barco processing


// content of in_desc has internal header accumulated and also 

//stage 2 - table 1  (nic_desc_entry_cb_t + nic_desc_entry_t)
stuct d_struct {
    ipsec_int_headroom : 8;
    ipsec_int_tailroom : 8;
    ipsec_int_headroom_offset : 16;
    ipsec_int_tailroom_offset : 16;
    ipsec_int_pad_size : 8;
    ipsec_int_buf_size : 16;
    //ipsec_int_drop_mask : 32;
    ipsec_int_iv_size : 8;
    ipsec_int_icv_size : 8;
    ipsec_int_ipsec_cb_index : 12;
    //ipsec_int_forward_port : 11;
    ipsec_int_in_desc : ADDRESS_WIDTH;
    ipsec_int_out_desc : ADDRESS_WIDTH;    
};

struct k_struct {
    in_desc_addr : ADDRESS_WIDTH;
};

struct p_struct p       ;
struct k_struct k       ;
struct d_struct d       ;

%%
// Load the input-descriptor internal header content into phv
ipsec_brq_read_input_descriptor:
    phvwr p.ipsec_int_headroom, d.ipsec_int_headroom
    phvwr p.ipsec_int_tailroom, d.ipsec_int_tailroom
    phvwr p.ipsec_int_headroom_offset, d.ipsec_int_headroom_offset
    phvwr p.ipsec_int_tailroom_offset, d.ipsec_int_tailroom_offset
    phvwr p.ipsec_int_pad_size, d.ipsec_int_pad_size
    phvwr p.ipsec_int_buf_size, d.ipsec_int_buf_size
    phvwr p.ipsec_int_ipsec_cb_index, d.ipsec_int_ipsec_cb_index
    phvwr p.in_desc_entry_addr, d.in_desc_entry_addr
    phvwr p.in_desc_entry_offset, d.in_desc_entry_offset
    phvwr p.in_desc_entry_len, d.in_desc_entry_len

// fill up intrinsic
ipsec_post_barco_fillup_v4_tun_encap_intrinsic:
    phvwri p.intrinsic_tm_oport, TM_P4_INGRESS       
    phvwri p.intrinsic_tm_oq, TM_P4_DEF_OQUEUE
    phvwri p.intrinsic_lif, IPSEC_ESP_SVC_LIF
    phvwri p.intrinsic_phv_type, IPSEC_ESP_V4_TUNNEL_H2N

ipsec_brq_read_out_descriptor:
    phvwr p.out_desc_entry_addr, d.out_desc_entry_addr
    phvwr p.out_desc_entry_offset, d.out_desc_entry_offset
    phvwr p.out_desc_entry_len, d.out_desc_entry_len


   
     
