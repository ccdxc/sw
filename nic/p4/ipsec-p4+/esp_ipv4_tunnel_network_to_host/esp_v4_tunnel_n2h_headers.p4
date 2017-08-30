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
        key_index : 32;
        iv_size   : 8;
        icv_size  : 8;
        expected_seq_no   : 32;
        last_replay_seq_no : 32;
        replay_seq_no_bmp : 64;
        barco_enc_cmd  : 32;
        ipsec_cb_index : 16;
        block_size     : 8;
        cb_pindex      : 16;
        cb_cindex      : 16;
        cb_ring_base_addr : ADDRESS_WIDTH;
        ipsec_cb_pad   : 56;
    }
}

header_type ipsec_int_header_t {
    fields {
        in_desc           : ADDRESS_WIDTH;
        out_desc          : ADDRESS_WIDTH;
        ipsec_cb_index    : 16;
        headroom          : 8;
        tailroom          : 8;
        headroom_offset   : 16;
        tailroom_offset   : 16;
        payload_start     : 16;
        buf_size          : 16;
        payload_size      : 16;
        l4_protocol       : 8;
        pad_size          : 8;
        ipsec_int_pad0    : 256;
    }
}

header_type barco_descriptor_t {
    fields {
        //scratch : 512

        A0_addr : ADDRESS_WIDTH;
        O0      : AOL_OFFSET_WIDTH;
        L0      : AOL_LENGTH_WIDTH;
        A1_addr : ADDRESS_WIDTH;
        O1      : AOL_OFFSET_WIDTH;
        L1      : AOL_LENGTH_WIDTH;
        A2_addr : ADDRESS_WIDTH;
        O2      : AOL_OFFSET_WIDTH;
        L2      : AOL_LENGTH_WIDTH;
        // Barco linked list next descriptor entry addr
        NextAddr : ADDRESS_WIDTH;
        // Below will be renamed as CB-descriptor-linked-list-next - different from barco-next-addr
        Reserved : 64;
    }
}

header_type barco_request_t {
    fields {
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
    modify_field(ipsec_cb_scratch.pc, pc); \
    modify_field(ipsec_cb_scratch.rxdma_ring_pindex, rxdma_ring_pindex); \
    modify_field(ipsec_cb_scratch.rxdma_ring_cindex, rxdma_ring_cindex); \
    modify_field(ipsec_cb_scratch.barco_ring_pindex, barco_ring_pindex); \
    modify_field(ipsec_cb_scratch.barco_ring_cindex, barco_ring_cindex); \
    modify_field(ipsec_cb_scratch.key_index, key_index); \
    modify_field(ipsec_cb_scratch.iv_size, iv_size); \
    modify_field(ipsec_cb_scratch.icv_size, icv_size); \
    modify_field(ipsec_cb_scratch.last_replay_seq_no, last_replay_seq_no); \
    modify_field(ipsec_cb_scratch.expected_seq_no, expected_seq_no); \
    modify_field(ipsec_cb_scratch.replay_seq_no_bmp, replay_seq_no_bmp); \
    modify_field(ipsec_cb_scratch.barco_enc_cmd,barco_enc_cmd); \
    modify_field(ipsec_cb_scratch.block_size, block_size); \
    modify_field(ipsec_cb_scratch.ipsec_cb_index, ipsec_cb_index); \
    modify_field(ipsec_cb_scratch.cb_pindex, cb_pindex); \
    modify_field(ipsec_cb_scratch.cb_cindex, cb_cindex); \
    modify_field(ipsec_cb_scratch.cb_ring_base_addr, cb_ring_base_addr); \
    modify_field(ipsec_cb_scratch.ipsec_cb_pad, ipsec_cb_pad);               


#define IPSEC_BARCO_DESC_SCRATH \
    modify_field(barco_desc_scratch.A0_addr,A0_addr); \
    modify_field(barco_desc_scratch.O0,O0); \
    modify_field(barco_desc_scratch.L0,L0); \
    modify_field(barco_desc_scratch.A1_addr,A1_addr); \
    modify_field(barco_desc_scratch.O1,O1); \
    modify_field(barco_desc_scratch.L1,L1); \
    modify_field(barco_desc_scratch.A2_addr,A2_addr); \
    modify_field(barco_desc_scratch.O2,O2); \
    modify_field(barco_desc_scratch.L2,L2); \
    modify_field(barco_desc_scratch.NextAddr,NextAddr); \
    modify_field(barco_desc_scratch.Reserved,Reserved); \
