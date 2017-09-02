#include "../ipsec_defines.h"

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
        pad_size          : 8;
        l4_protocol       : 8;
        //ipsec_int_pad0    : 256;
    }
}

header_type ipsec_cb_metadata_t {
    fields {
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
        spi       : 32;
        esn_lo    : 32;
        iv        : 64;
        esn_hi    : 32;
        barco_enc_cmd  : 32;
        ipsec_cb_index : 16;
        block_size     : 8;
        cb_pindex : 8;
        cb_cindex : 8;
        cb_ring_base_addr : ADDRESS_WIDTH;
        iv_salt   : 32;
        ipsec_cb_pad : 8;
    }
}

// the below phv is going to be written to the descriptor memory by action func as phv2mem
// please refer to barco document - table3 for the latest format.
header_type barco_descriptor_t {
    fields {
        A0_addr : ADDRESS_WIDTH;
        O0      : AOL_OFFSET_WIDTH;
        L0      : AOL_LENGTH_WIDTH;
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
        brq_pad1                            : 128;
        brq_pad2                            : 128;
        brq_pad3                            : 128;
        brq_pad4                            : 128;
    }
}

header_type p4plus_to_p4_ipsec_header_t {
    fields {
        app_type : 4;
        table0_valid : 1;
        table1_valid : 1;
        table2_valid : 1;
        table3_valid : 1;
        ipsec_pad1 : 256;
        ipsec_pad2 : 248;
    }
}

#define IPSEC_CB_SCRATCH \
    modify_field(ipsec_cb_scratch.rsvd, rsvd); \
    modify_field(ipsec_cb_scratch.cosA, cosA); \
    modify_field(ipsec_cb_scratch.cosB, cosB); \
    modify_field(ipsec_cb_scratch.cos_sel, cos_sel); \
    modify_field(ipsec_cb_scratch.eval_last, eval_last); \
    modify_field(ipsec_cb_scratch.host, host); \
    modify_field(ipsec_cb_scratch.total, total); \
    modify_field(ipsec_cb_scratch.pid, pid); \
    modify_field(ipsec_cb_scratch.rxdma_ring_pindex, rxdma_ring_pindex); \
    modify_field(ipsec_cb_scratch.rxdma_ring_cindex, rxdma_ring_cindex); \
    modify_field(ipsec_cb_scratch.barco_ring_pindex, barco_ring_pindex); \
    modify_field(ipsec_cb_scratch.barco_ring_cindex, barco_ring_cindex); \
    modify_field(ipsec_cb_scratch.key_index, key_index); \
    modify_field(ipsec_cb_scratch.iv_size, iv_size); \
    modify_field(ipsec_cb_scratch.icv_size, icv_size); \
    modify_field(ipsec_cb_scratch.spi, spi); \
    modify_field(ipsec_cb_scratch.esn_lo, esn_lo); \
    modify_field(ipsec_cb_scratch.iv, iv); \
    modify_field(ipsec_cb_scratch.barco_enc_cmd, barco_enc_cmd); \
    modify_field(ipsec_cb_scratch.ipsec_cb_index, ipsec_cb_index); \
    modify_field(ipsec_cb_scratch.block_size, block_size); \
    modify_field(ipsec_cb_scratch.cb_pindex, cb_pindex); \
    modify_field(ipsec_cb_scratch.cb_cindex, cb_cindex); \
    modify_field(ipsec_cb_scratch.cb_ring_base_addr, cb_ring_base_addr); \
    modify_field(ipsec_cb_scratch.iv_salt, iv_salt); \
    modify_field(ipsec_cb_scratch.ipsec_cb_pad, ipsec_cb_pad); \           
