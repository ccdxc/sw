#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action ipsec_encap_txdma2_initial_table 

#define tx_table_s1_t0_action ipsec_encap_txdma2_dummy 
#
#define tx_table_s2_t0_action ipsec_encap_txdma2_load_barco_req 

#define tx_table_s3_t0_action ipsec_encap_txdma2_load_in_desc 
#define tx_table_s3_t1_action ipsec_encap_txdma2_load_out_desc 
#define tx_table_s3_t2_action ipsec_encap_txdma2_load_ipsec_int

#define tx_table_s4_t0_action ipsec_build_encap_packet
#define tx_table_s4_t1_action ipsec_build_encap_packet2

#include "../../common-p4+/common_txdma.p4"
#include "esp_ipv4_tunnel_h2n_headers.p4"
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
        barco_desc_addr : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage2_t {
    fields {
        head_desc_addr : ADDRESS_WIDTH;
        stage1_pad     : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage3_t {
    fields {
        barco_req_addr   : ADDRESS_WIDTH;
        ipsec_cb_addr : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage4_t {
    fields {
        ipsec_cb_addr : ADDRESS_WIDTH;
        flags         : 6;
        is_nat_t      : 1;
        is_v6         : 1;
        barco_error   : 8;
        stage3_pad1     : 48;
    }
}

header_type ipsec_to_stage5_t {
    fields {
        ipsec_cb_addr : ADDRESS_WIDTH;
        stage4_pad1     : ADDRESS_WIDTH;
    }
}

@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t p4plus2p4_hdr;

//unionize
@pragma pa_header_union ingress common_global 
metadata ipsec_txdma2_global_t txdma2_global;

@pragma pa_header_union ingress to_stage_1
metadata ipsec_to_stage1_t ipsec_to_stage1;

@pragma pa_header_union ingress to_stage_2
metadata ipsec_to_stage2_t ipsec_to_stage2;

@pragma pa_header_union ingress to_stage_3
metadata ipsec_to_stage3_t ipsec_to_stage3;

@pragma pa_header_union ingress to_stage_4
metadata ipsec_to_stage4_t ipsec_to_stage4;

@pragma pa_header_union ingress to_stage_5
metadata ipsec_to_stage5_t ipsec_to_stage5;

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
metadata dma_cmd_phv2pkt_t ipsec_app_hdr;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t eth_hdr;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t ip_hdr;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t esp_iv_hdr;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t enc_pay_load;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t icv_header;

@pragma scratch_metadata
metadata ipsec_cb_metadata_t ipsec_cb_scratch;

@pragma scratch_metadata
metadata ipsec_txdma2_global_t txdma2_global_scratch;

@pragma scratch_metadata
metadata ipsec_to_stage4_t ipsec_to_stage4_scratch;

@pragma scratch_metadata
metadata ipsec_to_stage3_t ipsec_to_stage3_scratch;

@pragma scratch_metadata
metadata ipsec_to_stage2_t ipsec_to_stage2_scratch;

@pragma scratch_metadata
metadata ipsec_to_stage1_t ipsec_to_stage1_scratch;

@pragma scratch_metadata
metadata ipsec_table0_s2s t0_s2s_scratch;

@pragma scratch_metadata
metadata ipsec_table1_s2s t1_s2s_scratch;

@pragma scratch_metadata
metadata barco_request_t barco_req_scratch;

@pragma scratch_metadata
metadata ipsec_int_header_t ipsec_int_hdr_scratch;
@pragma scratch_metadata
metadata ipsec_int_pad_t ipsec_int_pad_scratch;


#define TXDMA2_GLOBAL_SCRATCH_INIT \
    modify_field(txdma2_global_scratch.in_desc_addr, txdma2_global.in_desc_addr); \
    modify_field(txdma2_global_scratch.ipsec_cb_index, txdma2_global.ipsec_cb_index); \
    modify_field(txdma2_global_scratch.iv_size, txdma2_global.iv_size); \
    modify_field(txdma2_global_scratch.icv_size, txdma2_global.icv_size); \
    modify_field(txdma2_global_scratch.pad_size, txdma2_global.pad_size); \
    modify_field(txdma2_global_scratch.l4_protocol, txdma2_global.l4_protocol); \
    modify_field(txdma2_global_scratch.payload_size, txdma2_global.payload_size); \

#define TXDMA2_T0_S2S_SCRATCH \
    modify_field(t0_s2s_scratch.in_page_addr, t0_s2s.in_page_addr); \
    modify_field(t0_s2s_scratch.out_page_addr, t0_s2s.out_page_addr); \
    modify_field(t0_s2s_scratch.headroom_offset, t0_s2s.headroom_offset); \
    modify_field(t0_s2s_scratch.tailroom_offset, t0_s2s.tailroom_offset); \
    
#define TXDMA2_T1_S2S_SCRATCH \
    modify_field(t1_s2s_scratch.out_desc_addr, t1_s2s.out_desc_addr); \
    modify_field(t1_s2s_scratch.out_page_addr, t1_s2s.out_page_addr); \
    
#define BARCO_REQ_SCRTATCH_SET \
    modify_field(barco_req_scratch.input_list_address,input_list_address); \
    modify_field(barco_req_scratch.output_list_address,output_list_address); \
    modify_field(barco_req_scratch.command,command); \
    modify_field(barco_req_scratch.key_desc_index,key_desc_index); \
    modify_field(barco_req_scratch.iv_address,iv_address); \
    modify_field(barco_req_scratch.auth_tag_addr,auth_tag_addr); \
    modify_field(barco_req_scratch.header_size,header_size); \
    modify_field(barco_req_scratch.status_address,status_address); \
    modify_field(barco_req_scratch.opaque_tag_value,opaque_tag_value); \
    modify_field(barco_req_scratch.opaque_tag_write_en,opaque_tag_write_en); \
    modify_field(barco_req_scratch.rsvd1,rsvd1); \
    modify_field(barco_req_scratch.sector_size,sector_size); \
    modify_field(barco_req_scratch.application_tag,application_tag); \
 
#define IPV4_HEADER_SIZE 20

//stage 4
action ipsec_build_encap_packet2()
{
    TXDMA2_GLOBAL_SCRATCH_INIT
    TXDMA2_T1_S2S_SCRATCH
    modify_field(ipsec_to_stage4_scratch.ipsec_cb_addr, ipsec_to_stage4.ipsec_cb_addr);
    modify_field(ipsec_to_stage4_scratch.is_v6, ipsec_to_stage4.is_v6);
    modify_field(ipsec_to_stage4_scratch.is_nat_t, ipsec_to_stage4.is_nat_t);
    modify_field(ipsec_to_stage4_scratch.flags, ipsec_to_stage4.flags);
    modify_field(ipsec_to_stage4_scratch.barco_error, ipsec_to_stage4.barco_error);
}

//stage 4
action ipsec_build_encap_packet()
{
    TXDMA2_GLOBAL_SCRATCH_INIT
    TXDMA2_T0_S2S_SCRATCH

    modify_field(ipsec_to_stage4_scratch.ipsec_cb_addr, ipsec_to_stage4.ipsec_cb_addr);
    modify_field(ipsec_to_stage4_scratch.is_v6, ipsec_to_stage4.is_v6);
    modify_field(ipsec_to_stage4_scratch.is_nat_t, ipsec_to_stage4.is_nat_t);
    modify_field(ipsec_to_stage4_scratch.flags, ipsec_to_stage4.flags);
    modify_field(ipsec_to_stage4_scratch.barco_error, ipsec_to_stage4.barco_error);
    // Add intrinsic and app header
    DMA_COMMAND_PHV2PKT_FILL(intrinsic_app_hdr, 0, 32, 0)

    // Add ethernet, optional-vlan and outer-ip from input-descriptor
    DMA_COMMAND_MEM2PKT_FILL(eth_hdr, t0_s2s.in_page_addr, t0_s2s.headroom_offset, 0, 0, 0) 
    DMA_COMMAND_MEM2PKT_FILL(ip_hdr, t0_s2s.in_page_addr+t0_s2s.headroom_offset, IPV4_HEADER_SIZE, 0, 0, 0) 

    // Add ESP header from IPSec-CB by adding spi,esn_lo,iv which are all contiguous in IPSec-CB
    //DMA_COMMAND_MEM2PKT_FILL(esp_iv_hdr, ipsec_to_stage4.ipsec_cb_addr+ESP_BASE_OFFSET, 8+txdma2_global.iv_size, 0, 0, 0)

    // Add encrypted payload from output page size is payload_size+pad
    //DMA_COMMAND_MEM2PKT_FILL(enc_pay_load, t0_s2s.out_page_addr, (txdma2_global.payload_size + txdma2_global.pad_size), 0, 0, 0)

    // Add ICV
    //DMA_COMMAND_MEM2PKT_FILL(icv_header, t0_s2s.out_page_addr+t0_s2s.tailroom_offset+2, txdma2_global.icv_size, 1, 0, 0)
 
    modify_field(p4_txdma_intr.dma_cmd_ptr, TXDMA2_DMA_COMMANDS_OFFSET);
}
 

//stage 3 table 2 
action ipsec_encap_txdma2_load_ipsec_int(in_desc, out_desc, in_page, out_page, 
                                         ipsec_cb_index, headroom, tailroom, 
                                         headroom_offset, tailroom_offset,
                                         payload_start, buf_size,
                                         payload_size, pad_size, l4_protocol, pad, status)
{
    IPSEC_INT_HDR_SCRATCH
    modify_field(ipsec_int_pad_scratch.ipsec_int_pad, pad);
    modify_field(ipsec_int_pad_scratch.status, status);

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

    modify_field(ipsec_to_stage3_scratch.ipsec_cb_addr, ipsec_to_stage3.ipsec_cb_addr);
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 0);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, txdma2_global.in_desc_addr);
}

//stage 2 table0
action ipsec_encap_txdma2_load_barco_req(input_list_address, output_list_address,
                                         command, key_desc_index,
                                         iv_address, auth_tag_addr,
                                         header_size, status_address,
                                         opaque_tag_value, opaque_tag_write_en,
                                         rsvd1, sector_size, application_tag)
{
    BARCO_REQ_SCRTATCH_SET
    modify_field(txdma2_global.in_desc_addr, input_list_address);

    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 7);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, input_list_address+64);

    modify_field(p4plus2p4_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0);
    modify_field(common_te1_phv.table_raw_table_size, 7);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, output_list_address+64);

    modify_field(p4plus2p4_hdr.table2_valid, 1);
    modify_field(common_te2_phv.table_pc, 0);
    modify_field(common_te2_phv.table_raw_table_size, 7);
    modify_field(common_te2_phv.table_lock_en, 0);
    modify_field(common_te2_phv.table_addr, input_list_address);
}

//stage 1
action ipsec_encap_txdma2_dummy()
{
    modify_field(ipsec_to_stage1_scratch.barco_desc_addr, ipsec_to_stage1.barco_desc_addr);

    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 0);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, txdma2_global.in_desc_addr);
}

//stage 0
action ipsec_encap_txdma2_initial_table(rsvd, cosA, cosB, cos_sel,
                                       eval_last, host, total, pid,
                                       rxdma_ring_pindex, rxdma_ring_cindex,
                                       barco_ring_pindex, barco_ring_cindex,
                                       key_index, iv_size, icv_size, spi,
                                       esn_lo, iv, esn_hi, barco_enc_cmd,
                                       ipsec_cb_index, block_size,
                                       cb_pindex, cb_cindex, barco_pindex, barco_cindex, 
                                       cb_ring_base_addr_hi, cb_ring_base_addr, 
                                       barco_ring_base_addr_hi, barco_ring_base_addr,  
                                       iv_salt, flags)
{
    IPSEC_CB_SCRATCH

    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);
    //modify_field(ipsec_to_stage4.ipsec_cb_addr, p4_txdma_intr.qstate_addr);

    modify_field(txdma2_global.ipsec_cb_index, ipsec_cb_index);
    modify_field(txdma2_global.iv_size, iv_size);
    modify_field(txdma2_global.icv_size, icv_size);
 
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 7);
    modify_field(common_te0_phv.table_lock_en, 0);
    // modify_field(common_te0_phv.table_addr, BRQ_REQ_RING_BASE_ADDR+(BRQ_REQ_RING_ENTRY_SIZE * barco_ring_cindex));
}
