

struct d_struct {
    brq_pindex : 8;
};

struct k_struct {
    brq_semaphore : ADDRESS_WIDTH;
};

struct p_struct p       ;
struct k_struct k       ;
struct d_struct d       ;

ipsec_alloc_brq_pindex:
    phvwr p.brq_pindex, d.brq_pindex

ipsec_compute_brq_req_struct_addr:
    add r1, r0, k.brq_pindex
    sll r1, r1, BRQ_REQ_ENTRY_SIZE
    addi r2, r0, BRQ_REQ_BASE_ADDR
    add r2, r1, r2
    phvwr p.table_sel, TABLE_TYPE_RAW
    phvwri p.table_mpu_entry_raw, ipsec_brq_alloc_req_fill
    phvwri p.table_size, BRQ_REQ_ENTRY_SIZE
    phvwr p.table_addr, r2


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
    ipsec_int_in_desc           : ADDRESS_WIDTH;
    ipsec_int_out_desc          : ADDRESS_WIDTH;
    ipsec_int_barco_enc_cmd     : 32;
    ipsec_int_ipsec_cb_index    : 12;
    ipsec_int_headroom          : 8;
    ipsec_int_tailroom          : 8;
    ipsec_int_headroom_offset   : 16;
    ipsec_int_tailroom_offset   : 16;
    ipsec_int_pad_size          : 8;
    in_desc_entry_addr          : ADDRESS_WIDTH;
    p42p4plus_hdr_payload_start : 16;
    ipsec_cb_key_index          : 32;
    //ipsec_cb_iv_addr            : ADDRESS_WIDTH; // directly write the computed value to brq_iv_addr ??
    out_desc_entry_addr         : 30;
    
};

//c-function: ipsec_esp_v4_tun_h2n_barco_enq
ipsec_esp_v4_tun_h2n_brq_alloc_req_fill:
    // req.in_addr = phv->ipsec_int.in_desc;
    phvwr p.brq_in_addr, k.ipsec_int_in_desc
    // req.out_addr = phv->ipsec_int.out_desc;
    phvwr p.brq_out_addr, k.ipsec_int_out_desc
    // req.enc_cmd = ipsec_cb->barco_enc_cmd;
    phvwr p.brq_enc_cmd, k.ipsec_cb_barco_enc_cmd
    // load the ipsec_cb_index, multiply with ipsec_cb_size, 
    // add the iv-offset within ipsec_cb, add that to the base-address 
    // of the ipsec-cb-table to get the iv_addr for barco
    // req.iv_addr = (u64)&ipsec_cb->iv;
    add   r1, r0, d.ipsec_int_ipsec_cb_index
    sll r1, r1, IPSEC_CB_ENTRY_SIZE
    addi r1, r1, IPSEC_CB_IV_OFFSET_IN_CB
    addi r1, r1, IPSEC_CB_BASE_TABLE_ADDR
    phvwr p.brq_iv_addr, r1
    //req.auth_tag_addr = (u64)out_desc->entry.addr + phv->ipsec_int.tailroom_offset + phv->ipsec_int.pad_size + 2;
    add r1, k.out_desc_entry_addr, k.ipsec_int_tailroom_offset
    add r1, r1, k.ipsec_int_pad_size
    addi r1, r1, 2
    phvwr p.brq_auth_tag_addr, r1
    // req.hdr_size = phv->p42p4plus_hdr.payload_start;
    phvwr p.brq_hdr_size, k.p42p4plus_hdr_payload_start
    // req.key_index = ipsec_cb->key_index;
    phvwr p.brq_key_index, k.ipsec_cb_key_index
    phvwri p.brq_db_en, 1   
     
