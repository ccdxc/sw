// RXDMA has 4 dma operations
#define ADDRESS_WIDTH 52

#define IN_DESC_SEM_ADDR_OFFSET          1000
#define IN_PAGE_SEM_ADDR_OFFSET          1001

#define OUT_DESC_SEM_ADDR_OFFSET         1002
#define OUT_PAGE_SEM_ADDR_OFFSET         1003

struct p_struct {
        // DMA COMMANDS - END
        dma_cmd0_cmd                     :  8;
        dma_cmd0_size                    :  16;
        dma_cmd0_pad                     :  40;
        dma_cmd0_addr                    :  64;

        pad1                             : 120;

        dma_cmd1_cmd                     :  8;
        dma_cmd1_size                    :  16;
        dma_cmd1_pad                     :  40;
        dma_cmd1_addr                    :  64;
        pad2                             :  128;

        dma_cmd2_cmd                     :  8;
        dma_cmd2_size                    :  16;
        dma_cmd2_pad                     :  40;
        dma_cmd2_addr                    :  64;



        dma_cmd3_cmd                     :  8;
        dma_cmd3_size                    :  16;
        dma_cmd3_pad                     :  40;
        dma_cmd3_addr                    :  64;
        pad4                             :  64;
        // DMA COMMANDS - START
      
        in_desc_pindex                  : 8;
        in_page_pindex                  : 8;
        out_desc_pindex                 : 8;
        out_page_pindex                 : 8; 
        brq_pindex                      : 16;

        in_desc_semaphore               : ADDRESS_WIDTH;
        in_page_semaphore               : ADDRESS_WIDTH;
        out_desc_semaphore              : ADDRESS_WIDTH;
        out_page_semaphore              : ADDRESS_WIDTH;
        brq_semaphore                   : ADDRESS_WIDTH;
         
        // BARCO Request PHV - barco_symm_req_t end
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
        //barco_symm_req_t begin
        
        // nic_desc_entry_t - end
        in_desc_entry_scratch              : 64;
        in_desc_entry_free_pending         : 1;
        in_desc_entry_valid                : 1;
        in_desc_entry_addr                 : 30;
        in_desc_entry_offset               : 16;
        in_desc_entry_len                  : 16;
        // in- nic_desc_entry_t - begin

        // nic_desc_entry_t - end
        out_desc_entry_scratch              : 64;
        out_desc_entry_free_pending         : 1;
        out_desc_entry_valid                : 1;
        out_desc_entry_addr                 : 30;
        out_desc_entry_offset               : 16;
        out_desc_entry_len                  : 16;
        // out- nic_desc_entry_t - begin

        // p4plus_ipsec_internal_t - end
        ipsec_int_tailroom                        : 8;
        ipsec_int_headroom                        : 8;
        ipsec_int_headroom_offset                 : 14;
        ipsec_int_tailroom_offset                 : 14;
        ipsec_int_pad_size                        : 8;
        ipsec_int_buf_size                        : 14;
        ipsec_int_drop_mask                       : 32;
        ipsec_int_icv_size                        : 8;
        ipsec_int_iv_size                         : 8;
        ipsec_int_ipsec_cb_index                  : 12;
        ipsec_int_forward_port                    : 11;
        ipsec_int_idesc                           : ADDRESS_WIDTH;
        ipsec_int_odesc                           : ADDRESS_WIDTH;
        // p4plus_ipsec_internal_t - begin

        // IPSec-CB End
        ipsec_cb_key_index     : 32;
        ipsec_cb_barco_enc_cmd : 32;
        ipsec_cb_block_size    : 8;
        ipsec_cb_esn_hi        : 32;
        ipsec_cb_esn_lo        : 32;
        ipsec_cb_iv            : 64;
 
        // IPSec-CB Begin

        // Private PHV - START
        // other common phv end - need to be sized accordingly

        p42p4plus_hdr_payload_start     : 16;
        p42p4plus_hdr_payload_end       : 16;
        p42p4plus_hdr_forward_port      : 11;
        p42p4plus_hdr_pad               : 5;
        p42p4plus_hdr_l4_proto          : 8;
        p42p4plus_hdr_ip_hdr_size       : 8;
        p42p4plus_hdr_seq_no            :32;

        // other common phv begin(in discussions)

        // INTRINSIC - END
        table_sel                       : 4; // RAW, NONE, or configured table ID
        table_addr                      : 64;
        table_size                      : 6;
        table_key_phv                   : 8;
        table_mpu_entry_raw             : 26;
        // pointer to first DMA command
        dma_cmd_ptr                     : 8;
        sched_id                        : 17;

        type                            : 4;
        vf                              : 11;
        drop                            : 1;
        error                           : 1;
        resubmit                        : 1;
        pkt_type                        : 4;
        tm_oport                        : 8;
        tm_oq                           : 5;
        tm_iq                           : 5;
        tm_rep_en                       : 1;
        tm_rep                          : 12;
        timestamp                       : 32;
        tm_q_depth                      : 16;
        // INTRINSIC - START             

};

    
