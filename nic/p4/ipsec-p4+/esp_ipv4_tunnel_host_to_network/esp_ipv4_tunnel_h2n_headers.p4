#include "../ipsec_defines.h"
#include "../ipsec_common_headers.p4"

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
        //ipsec_int_pad     : 96;
    }
}

header_type ipsec_int_pad_t {
    fields {
        ipsec_int_pad : 64;
        status : 64;
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
    
header_type esp_header_t {
    fields {
        spi : 32;
        seqno : 32;
        iv    : 64;
        iv2   : 64;
    }
}



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
        spi       : 32;
        esn_lo    : 32;
        iv        : 64;
        barco_enc_cmd  : 32;
        ipsec_cb_index : 16;
        block_size     : 8;
        cb_pindex : 8;
        cb_cindex : 8;
        barco_pindex : 8;
        barco_cindex : 8;
        cb_ring_base_addr_hi : 8;
        cb_ring_base_addr : 32;
        barco_ring_base_addr_hi : 8;
        barco_ring_base_addr : 32;
        iv_salt   : 32;
        flags : 8;
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
    modify_field(ipsec_cb_scratch.barco_pindex, barco_pindex); \
    modify_field(ipsec_cb_scratch.barco_cindex, barco_cindex); \
    modify_field(ipsec_cb_scratch.cb_ring_base_addr_hi, cb_ring_base_addr_hi); \
    modify_field(ipsec_cb_scratch.cb_ring_base_addr, cb_ring_base_addr); \
    modify_field(ipsec_cb_scratch.barco_ring_base_addr_hi, barco_ring_base_addr_hi); \
    modify_field(ipsec_cb_scratch.barco_ring_base_addr, barco_ring_base_addr); \
    modify_field(ipsec_cb_scratch.iv_salt, iv_salt); \
    modify_field(ipsec_cb_scratch.flags, flags); \          

#define IPSEC_CB_SCRATCH_WITH_PC \
    modify_field(ipsec_cb_scratch.pc, pc); \
    IPSEC_CB_SCRATCH 

