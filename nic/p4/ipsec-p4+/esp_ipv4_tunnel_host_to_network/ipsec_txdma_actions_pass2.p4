#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action ipsec_encap_txdma2_initial_table 

#define tx_table_s1_t0_action ipsec_encap_txdma2_load_barco_req_ptr

#define tx_table_s2_t0_action ipsec_encap_txdma2_load_barco_req 

#define tx_table_s3_t0_action ipsec_encap_txdma2_load_in_desc 
#define tx_table_s3_t1_action ipsec_encap_txdma2_load_out_desc 
#define tx_table_s3_t2_action ipsec_encap_txdma2_load_ipsec_int

#define tx_table_s4_t0_action ipsec_build_encap_packet

#include "../../common-p4+/common_txdma.p4"
#include "esp_ipv4_tunnel_h2n_headers.p4"
#
#include "../ipsec_defines.h"



header_type ipsec_txdma2_global_t {
    fields {
        in_desc_addr   : ADDRESS_WIDTH;
        ipsec_cb_index : 16;
        iv_size : 8;
        icv_size : 8;
        pad_size : 8;
        l4_protocol : 8;
        payload_size : 16;
    }
}

header_type ipsec_table0_s2s {
    fields {
        in_page_addr : ADDRESS_WIDTH;
        out_page_addr : ADDRESS_WIDTH;
        headroom_offset : 16;
        tailroom_offset : 16;
    }
}

header_type ipsec_table1_s2s {
    fields {
        out_desc_addr : ADDRESS_WIDTH;
        out_page_addr : ADDRESS_WIDTH;
    }
}

header_type ipsec_table2_s2s {
    fields {
        in_desc_addr : ADDRESS_WIDTH;
        in_page_addr : ADDRESS_WIDTH;
    }
}

header_type ipsec_table3_s2s {
    fields {
        out_desc_addr : ADDRESS_WIDTH;
        out_page_addr : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage1_t {
    fields {
        head_desc_addr : ADDRESS_WIDTH;
        stage1_pad     : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage2_t {
    fields {
        barco_req_addr   : ADDRESS_WIDTH;
        stage2_pad     : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage3_t {
    fields {
        stage3_pad0     : ADDRESS_WIDTH;
        stage3_pad1     : ADDRESS_WIDTH;
    }
}


@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_ipsec_header_t p4plus2p4_hdr;

//unionize
@pragma pa_header_union ingress common_global 
metadata ipsec_txdma2_global_t txdma2_global;

@pragma pa_header_union ingress to_stage_1
metadata ipsec_to_stage1_t ipsec_to_stage1;

@pragma pa_header_union ingress to_stage_2
metadata ipsec_to_stage2_t ipsec_to_stage2;

@pragma pa_header_union ingress to_stage_3
metadata ipsec_to_stage3_t ipsec_to_stage3;

@pragma pa_header_union ingress common_t0_s2s
metadata ipsec_table0_s2s t0_s2s;
@pragma pa_header_union ingress common_t1_s2s
metadata ipsec_table1_s2s t1_s2s;
@pragma pa_header_union ingress common_t2_s2s
metadata ipsec_table2_s2s t2_s2s;
@pragma pa_header_union ingress common_t3_s2s
metadata ipsec_table3_s2s t3_s2s;


//TXDMA - IPsec feature specific scratch
@pragma dont_trim
metadata ipsec_int_header_t ipsec_int_header;

@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_app_hdr;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t eth_ip_hdr;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t esp_iv_hdr;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t enc_pay_load;
@pragma dont_trim
metadata dma_cmd_phv2pkt_t tail_2_bytes;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t icv_header;

@pragma scratch_metadata
metadata ipsec_cb_metadata_t ipsec_cb_scratch;




//stage 4
action ipsec_build_encap_packet()
{
    // Add intrinsic and app header
    DMA_COMMAND_PHV2PKT_FILL(intrinsic_app_hdr, 0, 32, 0)

    // Add ethernet, optional-vlan and outer-ip from input-descriptor
    DMA_COMMAND_MEM2PKT_FILL(eth_ip_hdr, t0_s2s.in_page_addr, t0_s2s.headroom_offset, 0, 0, 0) 

    // Add ESP header from IPSec-CB by adding spi,esn_lo,iv which are all contiguous in IPSec-CB
    DMA_COMMAND_MEM2PKT_FILL(esp_iv_hdr, (txdma2_global.ipsec_cb_index*IPSEC_CB_SIZE)+IPSEC_CB_BASE+ESP_BASE_OFFSET, 8+txdma2_global.iv_size, 0, 0, 0)

    // Add encrypted payload from output page size is payload_size+pad
    DMA_COMMAND_MEM2PKT_FILL(enc_pay_load, t0_s2s.out_page_addr, (txdma2_global.payload_size + txdma2_global.pad_size), 0, 0, 0)

    // Fill right values below - fill 2 bytes of pad_size and l4protocl from global phv which are contiguous
    DMA_COMMAND_PHV2PKT_FILL(tail_2_bytes, 0, 0, 0) 

    // Add ICV
    DMA_COMMAND_MEM2PKT_FILL(icv_header, t0_s2s.out_page_addr+t0_s2s.tailroom_offset+2, txdma2_global.icv_size, 1, 0, 0)
 
    modify_field(p4_txdma_intr.dma_cmd_ptr, TXDMA2_DMA_COMMANDS_OFFSET);
}
 

//stage 3 table 2 
action ipsec_encap_txdma2_load_ipsec_int(in_desc, out_desc, ipsec_cb_index,
                                         headroom_offset, tailroom_offset,
                                         pad_size, payload_start, buf_size,
                                         payload_size, l4_protocol, pad_1)
{
    modify_field(txdma2_global.pad_size, pad_size);
    modify_field(txdma2_global.l4_protocol, l4_protocol);
    modify_field(txdma2_global.payload_size, payload_size);
    modify_field(t0_s2s.tailroom_offset, tailroom_offset); 
    modify_field(t0_s2s.headroom_offset, headroom_offset);

    modify_field(p4plus2p4_hdr.table2_valid, 0);
}

//stage 3 table 1 
action ipsec_encap_txdma2_load_out_desc(addr0, offset0, length0,
                                        addr1, offset1, length1,
                                        addr2, offset2, length2,
                                        nextptr, rsvd)
{
    modify_field(t0_s2s.out_page_addr, addr0);

    modify_field(p4plus2p4_hdr.table1_valid, 0);
}

//stage 3 table 0 
action ipsec_encap_txdma2_load_in_desc(addr0, offset0, length0,
                                       addr1, offset1, length1,
                                       addr2, offset2, length2,
                                       nextptr, rsvd)
{
    modify_field(t0_s2s.in_page_addr, addr0);

    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 0);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, txdma2_global.in_desc_addr);
}




//stage 2 table0
action ipsec_encap_txdma2_load_barco_req(brq_in_addr, brq_out_addr,
                                         brq_barco_enc_cmd, brq_key_index,
                                         brq_iv_addr, brq_auth_tag_addr,
                                         brq_hdr_size, brq_status,
                                         brq_opq_tag_value, brq_db_en,
                                         brq_opq_tag_en, brq_rsvd,
                                         brq_sec_sz, brq_sec_num, brq_app_tag)
{
    modify_field(txdma2_global.in_desc_addr, brq_in_addr);

    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 7);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, brq_in_addr+64);

    modify_field(p4plus2p4_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0);
    modify_field(common_te1_phv.table_raw_table_size, 7);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, brq_out_addr+64);

    modify_field(p4plus2p4_hdr.table2_valid, 1);
    modify_field(common_te2_phv.table_pc, 0);
    modify_field(common_te2_phv.table_raw_table_size, 7);
    modify_field(common_te2_phv.table_lock_en, 0);
    modify_field(common_te2_phv.table_addr, brq_in_addr);
 
}


//stage 1
action ipsec_encap_txdma2_load_barco_req_ptr(barco_req_address)
{
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, barco_req_address);
}


//stage 0
action ipsec_encap_txdma2_initial_table(pc, rsvd, cosA, cosB, cos_sel,
                                       eval_last, host, total, pid,
                                       rxdma_ring_pindex, rxdma_ring_cindex,
                                       barco_ring_pindex, barco_ring_cindex,
                                       key_index, iv_size, icv_size, spi,
                                       esn_lo, iv, esn_hi, barco_enc_cmd,
                                       ipsec_cb_index, block_size,
                                       cb_pindex, cb_cindex, cb_ring_base_addr, ipsec_cb_pad)
{
   //IPSEC_CB_SCRATCH

    modify_field(txdma2_global.ipsec_cb_index, ipsec_cb_index);
    modify_field(txdma2_global.iv_size, iv_size);
    modify_field(txdma2_global.icv_size, icv_size);
 
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 7);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, BRQ_REQ_RING_BASE_ADDR+(BRQ_REQ_RING_ENTRY_SIZE * barco_ring_cindex));
   
}

