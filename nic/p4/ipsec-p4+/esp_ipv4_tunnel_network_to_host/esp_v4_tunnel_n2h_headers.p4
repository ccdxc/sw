#include "../ipsec_defines.h"


header_type ipsec_cb_metadata_t {
    fields {
        pc                      : 8;
        rsvd                    : 8;
        cosA                    : 4;
        cosB                    : 4;
        cos_sel                 : 8;
        eval_last               : 8;
        host                    : 4;
        total                   : 4;
        pid                     : 16;

        rxdma_ring_pindex : RING_INDEX_WIDTH;
        rxdma_ring_cindex : RING_INDEX_WIDTH;
        barco_ring_pindex : RING_INDEX_WIDTH;
        barco_ring_cindex : RING_INDEX_WIDTH;
        key_index : 16;
        new_key_index : 16;
        iv_size   : 8;
        icv_size  : 8;
        expected_seq_no   : 32;
        //last_replay_seq_no : 32;
        replay_seq_no_bmp : 64;
        barco_enc_cmd  : 32;
        ipsec_cb_index : 16;
        block_size     : 8;
        cb_pindex      : 8;
        cb_cindex      : 8;
        barco_pindex   : 8;
        barco_cindex   : 8;
        cb_ring_base_addr : 64;
        barco_ring_base_addr : 64;
        //iv_salt        : 32;
        vrf_vlan       : 16;
        is_v6          : 8;
    }
}

header_type ipsec_decrypt_part2_t {
    fields {
        spi : 32;
        new_spi : 32;
        last_replay_seq_no : 32;
        iv_salt : 32;
    }
}

header_type ipsec_int_header_t {
    fields {
        in_desc           : ADDRESS_WIDTH;
        out_desc          : ADDRESS_WIDTH;
        in_page           : ADDRESS_WIDTH;
        out_page          : ADDRESS_WIDTH;
        ipsec_cb_index    : 16;
        headroom          : 8;
        tailroom          : 8;
        headroom_offset   : 16;
        tailroom_offset   : 16;
        payload_start     : 16;
        buf_size          : 16;
        payload_size      : 16;
        pad_size          : 8;
        l4_protocol       : 8;
        spi               : 32;
        ipsec_int_pad     : 32;
        status_addr       : 64;
    }
}

header_type ipsec_int_pad_t {
    fields {
        ipsec_int_pad : 96;
    }
}

header_type p4plus_to_p4_ipsec_header_t {                                                                                                              
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        flags               : 8;
        udp_opt_bytes       : 8;
        rsvd                : 24;
        ip_id_delta         : 16;
        tcp_seq_delta       : 32;
        gso_start           : 14;
        gso_offset          : 14;
        byte_align_pad      : 3;
        gso_valid           : 1;
        vlan_tag            : 16;
        ipsec_pad1          : 120;
    }
}

header_type ipsec_cb_encap_header_t {
    fields {
        mac_ip_hdr : 432;
        drop_mask : 16;
        seq_no_fail : 16;
        decrypt_fail : 16;
        decrypt_cb2_pad : 32;
    }
}
 
#define IPSEC_INT_HDR_SCRATCH \
    modify_field(ipsec_int_hdr_scratch.in_desc, in_desc); \
    modify_field(ipsec_int_hdr_scratch.out_desc, out_desc); \
    modify_field(ipsec_int_hdr_scratch.in_page, in_page); \
    modify_field(ipsec_int_hdr_scratch.out_page, out_page); \
    modify_field(ipsec_int_hdr_scratch.ipsec_cb_index, ipsec_cb_index); \
    modify_field(ipsec_int_hdr_scratch.headroom, headroom); \
    modify_field(ipsec_int_hdr_scratch.tailroom, tailroom); \
    modify_field(ipsec_int_hdr_scratch.headroom_offset, headroom_offset); \
    modify_field(ipsec_int_hdr_scratch.tailroom_offset, tailroom_offset); \
    modify_field(ipsec_int_hdr_scratch.payload_start, payload_start); \
    modify_field(ipsec_int_hdr_scratch.buf_size, buf_size); \
    modify_field(ipsec_int_hdr_scratch.payload_size, payload_size); \
    modify_field(ipsec_int_hdr_scratch.pad_size, pad_size); \
    modify_field(ipsec_int_hdr_scratch.l4_protocol, l4_protocol); \
    modify_field(ipsec_int_hdr_scratch.spi, spi); \
    modify_field(ipsec_int_hdr_scratch.ipsec_int_pad, ipsec_int_pad); \
    modify_field(ipsec_int_hdr_scratch.status_addr, status_addr); \

header_type esp_header_t {
    fields {
        spi : 32;
        seqno : 32;
        iv    : 64;
    }
}

header_type barco_descriptor_t {
    fields {
        A0_addr : ADDRESS_WIDTH;
        O0      : AOL_OFFSET_WIDTH;
        L0      : AOL_LENGTH_WIDTH;
        A1_addr : ADDRESS_WIDTH;
        O1      : AOL_OFFSET_WIDTH;
        L1      : AOL_LENGTH_WIDTH;
    }
}

header_type barco_zero_content_t {
    fields {
        A2_addr : ADDRESS_WIDTH;
        O2      : AOL_OFFSET_WIDTH;
        L2      : AOL_LENGTH_WIDTH;
        nextAddress : ADDRESS_WIDTH;
        rsvd    : ADDRESS_WIDTH;
    }
}

header_type barco_request_t {
    fields {
        input_list_address                  : 64;
        output_list_address                 : 64;
        command                             : 32;
        key_desc_index                      : 32;
        iv_address                          : 64;
        auth_tag_addr                       : 64;
        header_size                         : 32;
        status_address                      : 64;
        opaque_tag_value                    : 32;
        opaque_tag_write_en                 : 1;
        rsvd1                               : 31;
        sector_size                         : 16;
        application_tag                     : 16;
        sector_num                          : 32;
        doorbell_address                    : 64;
        doorbell_data                       : 64;
    }
}

#define IPSEC_CB_SCRATCH \
    modify_field(ipsec_cb_scratch.pid, pid); \
    modify_field(ipsec_cb_scratch.total, total); \
    modify_field(ipsec_cb_scratch.host, host); \
    modify_field(ipsec_cb_scratch.eval_last, eval_last); \
    modify_field(ipsec_cb_scratch.cos_sel, cos_sel); \
    modify_field(ipsec_cb_scratch.cosB, cosB); \
    modify_field(ipsec_cb_scratch.cosA, cosA); \
    modify_field(ipsec_cb_scratch.rsvd, rsvd); \
    modify_field(ipsec_cb_scratch.rxdma_ring_pindex, rxdma_ring_pindex); \
    modify_field(ipsec_cb_scratch.rxdma_ring_cindex, rxdma_ring_cindex); \
    modify_field(ipsec_cb_scratch.barco_ring_pindex, barco_ring_pindex); \
    modify_field(ipsec_cb_scratch.barco_ring_cindex, barco_ring_cindex); \
    modify_field(ipsec_cb_scratch.key_index, key_index); \
    modify_field(ipsec_cb_scratch.new_key_index, new_key_index); \
    modify_field(ipsec_cb_scratch.iv_size, iv_size); \
    modify_field(ipsec_cb_scratch.icv_size, icv_size); \
    modify_field(ipsec_cb_scratch.expected_seq_no, expected_seq_no); \
    modify_field(ipsec_cb_scratch.replay_seq_no_bmp, replay_seq_no_bmp); \
    modify_field(ipsec_cb_scratch.barco_enc_cmd,barco_enc_cmd); \
    modify_field(ipsec_cb_scratch.block_size, block_size); \
    modify_field(ipsec_cb_scratch.ipsec_cb_index, ipsec_cb_index); \
    modify_field(ipsec_cb_scratch.cb_pindex, cb_pindex); \
    modify_field(ipsec_cb_scratch.cb_cindex, cb_cindex); \
    modify_field(ipsec_cb_scratch.barco_pindex, barco_pindex); \
    modify_field(ipsec_cb_scratch.barco_cindex, barco_cindex); \
    modify_field(ipsec_cb_scratch.cb_ring_base_addr, cb_ring_base_addr); \
    modify_field(ipsec_cb_scratch.barco_ring_base_addr, barco_ring_base_addr); \
    modify_field(ipsec_cb_scratch.vrf_vlan, vrf_vlan); \
    modify_field(ipsec_cb_scratch.is_v6, is_v6);               

#define IPSEC_CB_SCRATCH_WITH_PC \
    modify_field(ipsec_cb_scratch.pc, pc); \
    IPSEC_CB_SCRATCH 

#define IPSEC_CB_SCRATCH_PART2 \
    modify_field(ipsec_cb_part2_scratch.spi, spi); \
    modify_field(ipsec_cb_part2_scratch.new_spi, new_spi); \
    modify_field(ipsec_cb_part2_scratch.last_replay_seq_no, last_replay_seq_no); \
    modify_field(ipsec_cb_part2_scratch.iv_salt, iv_salt); \


#define IPSEC_BARCO_DESC_SCRATH \
    modify_field(barco_desc_scratch.A0_addr,A0_addr); \
    modify_field(barco_desc_scratch.O0,O0); \
    modify_field(barco_desc_scratch.L0,L0); \
    modify_field(barco_desc_scratch.A1_addr,A1_addr); \
    modify_field(barco_desc_scratch.O1,O0); \
    modify_field(barco_desc_scratch.L1,L0); \
