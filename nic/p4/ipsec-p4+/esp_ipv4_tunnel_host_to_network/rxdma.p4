#include "../../../common-p4+/p4/common_rxdma.p4"

#define IPSEC_CB_SIZE 4096 
#define INDESC_SEMAPHORE_ADDR   0xa0a0a0a0
#define OUTDESC_SEMAPHORE_ADDR  0xa1a1a1a1
#define INPAGE_SEMAPHORE_ADDR   0xa2a2a2a2
#define OUTPAGE_SEMAPHORE_ADDR  0xa3a3a3a3

#define RING_INDEX_WIDTH 16

#define ESP_FIXED_HDR_SIZE 8 

header_type ipsec_int_header_t {
    fields {
    ipsec_int_in_desc           : ADDRESS_WIDTH;
    ipsec_int_out_desc          : ADDRESS_WIDTH;
    ipsec_int_barco_enc_cmd     : 32;
    ipsec_int_ipsec_cb_index    : 16;
    ipsec_int_headroom          : 8;
    ipsec_int_tailroom          : 8;
    ipsec_int_headroom_offset   : 16;
    ipsec_int_tailroom_offset   : 16;
    ipsec_int_pad_size          : 8;
    //in_desc_entry_addr          : ADDRESS_WIDTH;
    p42p4plus_hdr_payload_start : 16;
    ipsec_cb_key_index          : 32;
    //ipsec_cb_iv_addr            : ADDRESS_WIDTH; // directly write the computed value to brq_iv_addr ??
    //out_desc_entry_addr         : 30;
    ipsec_int_header_pad        : 2;
    ipsec_int_buf_size          : 16;
    }
}

// this is the scratch metadata used by ipsec-rxdma program
header_type rxdma_ipsec_scratch_t {
    fields {
        // will move into common phv later
        table0_pc   : RAW_TABLE_PC_WIDTH;
        table0_size : RAW_TABLE_SIZE_WIDTH;
        table0_lock_en : 1;
        table0_addr : RAW_TABLE_ADDR_WIDTH;

        in_desc_semaphore : 10;
        out_desc_semaphore : 10;
        in_page_semaphore : 10;
        out_page_semaphore : 10;
        barco_req_index    : 16;
        in_page_index      : RING_INDEX_WIDTH;
        out_page_index     : RING_INDEX_WIDTH;
        in_desc_index      : RING_INDEX_WIDTH;
        out_desc_index     : RING_INDEX_WIDTH;
        in_page_value      : 34;
        out_page_value     : 34;
        in_desc_value      : 34;
        out_desc_value     : 34;

        payload_size : 14;
    }
}

header_type ipsec_cb_metadata_t {
    fields {
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
        head_desc_addr : 64;
        tail_desc_addr : 64;
        ipsec_cb_pad : 120;
    }
}

// the below phv is going to be written to the descriptor memory by action func as phv2mem
// please refer to barco document - table3 for the latest format.
header_type barco_descriptor_t {
    fields {
        // ipsec_int gathered by RxDMA - stored inside descriptor - begin
        ipsec_int_in_desc           : ADDRESS_WIDTH;
        ipsec_int_out_desc          : ADDRESS_WIDTH;
        ipsec_int_barco_enc_cmd     : 32;
        ipsec_int_ipsec_cb_index    : 12;
        ipsec_int_headroom          : 8;
        ipsec_int_tailroom          : 8;
        ipsec_int_headroom_offset   : 16;
        ipsec_int_tailroom_offset   : 16;
        ipsec_int_pad_size          : 8;
        //in_desc_entry_addr          : ADDRESS_WIDTH;
        //out_desc_entry_addr         : ADDRESS_WIDTH;
        p42p4plus_hdr_payload_start : 16;
        //ipsec_cb_key_index          : 32;
        // ipsec_int gathered by RxDMA - stored inside descriptor - end (512 bits)

        //scratch : 512
        A0_addr : 64;
        O0      : 32;
        L0      : 32;
        A1_addr : 64;
        O1      : 32;
        L1      : 32;
        A2_addr : 64;
        O2      : 32;
        L2      : 32;
        // Barco linked list next descriptor entry addr
        NextAddr : 64;
        Reserved : 64;
    }
}


header_type ipsec_rxdma_cmds_t {
    fields {
        // pkt2mem - used for copying input packet to memory.
        dma_cmd0_type : 3;
        dma_cmd0_eop : 1;
        dma_cmd0_host_addr : 1;
        dma_cmd0_addr : 52;
        dma_cmd0_size : 14;
        dma_cmd0_cache : 1;
        dma_cmd0_use_override_lif : 1;
        dma_cmd0_override_lif : 11;
        dma_cmd0_pad : 44;

        // phv2mem - used for writing first 64 bytes of descriptor with ipsec_int header accumulated
        dma_cmd1_type : 3;
        dma_cmd1_eop : 1;
        dma_cmd1_host_addr : 1;
        dma_cmd1_addr : 52;
        dma_cmd1_phv_start_addr : 10;
        dma_cmd1_phv_end_addr : 10;
        dma_cmd1_wr_fence : 1;
        dma_cmd1_cache : 1;
        dma_cmd1_use_override_lif : 1;
        dma_cmd1_override_lif : 11;
        dma_cmd1_barrier : 1;
        dma_cmd1_pcie_msg : 1;
        dma_cmd1_pad : 35;

        // phv2mem - used for writing next 64 bytes of input-descriptor
        dma_cmd2_type : 3;
        dma_cmd2_eop : 1;
        dma_cmd2_host_addr : 1;
        dma_cmd2_addr : 52;
        dma_cmd2_phv_start_addr : 10;
        dma_cmd2_phv_end_addr : 10;
        dma_cmd2_wr_fence : 1;
        dma_cmd2_cache : 1;
        dma_cmd2_use_override_lif : 1;
        dma_cmd2_override_lif : 11;
        dma_cmd2_barrier : 1;
        dma_cmd2_pcie_msg : 1;
        dma_cmd2_pad : 35;

        // phv2mem - used for writing next 64 bytes of output-descriptor
        dma_cmd3_type : 3;
        dma_cmd3_eop : 1;
        dma_cmd3_host_addr : 1;
        dma_cmd3_addr : 52;
        dma_cmd3_phv_start_addr : 10;
        dma_cmd3_phv_end_addr : 10;
        dma_cmd3_wr_fence : 1;
        dma_cmd3_cache : 1;
        dma_cmd3_use_override_lif : 1;
        dma_cmd3_override_lif : 11;
        dma_cmd3_barrier : 1;
        dma_cmd3_pcie_msg : 1;
        dma_cmd3_pad : 35;

        //mem2mem-src - used for adding pad bytes
        dma_cmd4_type : 3;
        dma_cmd4_eop : 1;
        dma_cmd4_mem2mem_type : 2;
        dma_cmd4_host_addr : 1;
        dma_cmd4_cache : 1;
        dma_cmd4_addr : 52;
        dma_cmd4_size : 14;
        dma_cmd4_phv_start_addr : 10;
        dma_cmd4_phv_end_addr : 10;
        dma_cmd4_wr_fence : 1;
        dma_cmd4_use_override_lif : 1;
        dma_cmd4_override_lif : 11;
        dma_cmd4_barrier : 1;
        dma_cmd4_pcie_msg : 1;

        //mem2mem-dst - used for adding pad bytes
        dma_cmd5_type : 3;
        dma_cmd5_eop : 1;
        dma_cmd5_mem2mem_type : 2;
        dma_cmd5_host_addr : 1;
        dma_cmd5_cache : 1;
        dma_cmd5_addr : 52;
        dma_cmd5_size : 14;
        dma_cmd5_phv_start_addr : 10;
        dma_cmd5_phv_end_addr : 10;
        dma_cmd5_wr_fence : 1;
        dma_cmd5_use_override_lif : 1;
        dma_cmd5_override_lif : 11;
        dma_cmd5_barrier : 1;
        dma_cmd5_pcie_msg : 1;
    }
}

#if 0
// Metadata across - all IPSec programs
metadata cap_phv_intr_global_t capri_global_intrinsic;
metadata cap_phv_intr_p4_t capri_p4;
metadata cap_phv_intr_rxdma_t rxdma_intrinsic;
#endif

//Unionize the below with p4_2_p4plus_app_header_t
metadata p4_to_p4plus_ipsec_header_t p42p4plus_hdr;
#if 0
// Now the table-engine related - fixed
metadata p4plus_common_table_engine0_phv_t te0_phv;
metadata p4plus_common_table_engine_phv_t te_phv;
#endif


//RXDMA - IPsec feature specific scratch
metadata ipsec_int_header_t ipsec_int_header;
metadata ipsec_cb_metadata_t ipsec_cb;
metadata rxdma_ipsec_scratch_t rxdma_scratch;
metadata barco_descriptor_t barco_desc;

//dma commands
metadata ipsec_rxdma_cmds_t ipsec_rxdma_cmds;

action ipsec_rxdma_fill_ipsec_int_content_for_txdma()
{

}


@pragma stage 3
table ipsec_rxdma_incoming_packet {
    reads {
        rxdma_scratch.in_desc_value : exact;
    }
    actions {
        ipsec_rxdma_fill_ipsec_int_content_for_txdma;
    }
}


control ipsec_esp_v4_tun_h2n_do_pkt_dma {
    apply(ipsec_rxdma_incoming_packet);
}

action read_input_desc_contents (a0_addr, a0_offset, a0_len,
                                 a1_addr, a1_offset, a1_len,
                                 a2_addr, a2_offset, a2_len,
                                 next_ptr, desc_lock_en)
{
    modify_field(barco_desc.A0_addr, a0_addr);
    modify_field(barco_desc.O0, a0_offset);
    modify_field(barco_desc.L0, a0_len);
    modify_field(barco_desc.A1_addr, a1_addr);
    modify_field(barco_desc.O1, a1_offset);
    modify_field(barco_desc.L1, a1_len);
    modify_field(barco_desc.A2_addr, a2_addr);
    modify_field(barco_desc.O2, a2_offset);
    modify_field(barco_desc.L2, a2_len);


}

@pragma stage 3
@pragma raw_table common_te0_phv.table0_pc
table ipsec_read_input_desc_entry {
    reads {
        rxdma_scratch.in_desc_value : exact;
    }
    actions {
        read_input_desc_contents;
    }
}


control ipsec_esp_v4_tun_h2n_get_desc_and_page_entry {
   apply(ipsec_read_input_desc_entry);

}


action in_page_index_read(in_page_value)
{
    // modify_field(rxdma_scratch.in_page_value, in_page_value);
    modify_field(p42p4plus_hdr.table3_valid, 1);

    modify_field(te_phv.table3_addr, in_page_value);
    modify_field(te_phv.table3_pc, 0);
    modify_field(te_phv.table3_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te_phv.table3_lock_en, 0);
}

action out_page_index_read(out_page_value)
{
    // modify_field(rxdma_scratch.out_page_value, out_page_value);
    modify_field(p42p4plus_hdr.table2_valid, 1);

    modify_field(te_phv.table2_addr, out_page_value);
    modify_field(te_phv.table2_pc, 0);
    modify_field(te_phv.table2_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te_phv.table2_lock_en, 0);
}
action out_desc_index_read(out_desc_value)
{
    // modify_field(rxdma_scratch.out_desc_value, out_desc_value);
    modify_field(p42p4plus_hdr.table1_valid, 1);

    modify_field(te_phv.table1_addr, out_desc_value);
    modify_field(te_phv.table1_pc, 0);
    modify_field(te_phv.table1_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te_phv.table1_lock_en, 0);
}

action in_desc_index_read(in_desc_value)
{
    //modify_field(rxdma_scratch.in_desc_value, in_desc_value);

    modify_field(p42p4plus_hdr.table0_valid, 1);

    modify_field(te0_phv.table0_addr, in_desc_value);
    modify_field(te0_phv.table0_pc, 0);
    modify_field(te0_phv.table0_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te0_phv.table0_lock_en, 0);
}

@pragma stage 2
@pragma raw_table common_te_phv.table3_pc
table in_page_index {
    reads {
        rxdma_scratch.in_page_index: exact;
    }
    actions {
        in_page_index_read;
    }
    size : 1;
}

@pragma stage 2
@pragma raw_table common_te_phv.table2_pc
table out_page_index {
    reads {
        rxdma_scratch.out_page_index: exact;
    }
    actions {
        out_page_index_read;
    }
    size : 1;
}

@pragma stage 2
@pragma raw_table common_te_phv.table1_pc
table out_desc_index {
    reads {
        rxdma_scratch.out_desc_index: exact;
    }
    actions {
        out_desc_index_read;
    }
    size : 1;
}

@pragma stage 2
@pragma raw_table common_te0_phv.table0_pc
table in_desc_index {
    reads {
        rxdma_scratch.in_desc_index: exact;
    }
    actions {
        in_desc_index_read;
    }
    size : 1;
}


control ipsec_esp_v4_tun_h2n_get_desc_and_page_addrs {
    apply(in_page_index);
    apply(out_page_index);
    apply(out_desc_index);
    apply(in_desc_index);
}

action allocate_input_page_semaphore(in_page_index)
{
//  modify_field(rxdma_scratch.in_page_index, in_page_index);
    modify_field(p42p4plus_hdr.table3_valid, 1);

    modify_field(te_phv.table3_addr, in_page_index);
    modify_field(te_phv.table3_pc, 0);
    modify_field(te_phv.table3_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te_phv.table3_lock_en, 0);
}

action allocate_output_page_semaphore(out_page_index)
{
//  modify_field(rxdma_scratch.out_page_index, out_page_index);
    modify_field(p42p4plus_hdr.table2_valid, 1);

    modify_field(te_phv.table2_addr, out_page_index);
    modify_field(te_phv.table2_pc, 0);
    modify_field(te_phv.table2_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te_phv.table2_lock_en, 0);
}

action allocate_output_desc_semaphore(out_desc_index)
{
    //modify_field(rxdma_scratch.out_desc_index, out_desc_index);
    modify_field(p42p4plus_hdr.table1_valid, 1);

    modify_field(te_phv.table1_addr, out_desc_index);
    modify_field(te_phv.table1_pc, 0);
    modify_field(te_phv.table1_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te_phv.table1_lock_en, 0);
}

action allocate_input_desc_semaphore(in_desc_index)
{
    //modify_field(rxdma_scratch.in_desc_index, in_desc_index);
    modify_field(p42p4plus_hdr.table0_valid, 1);

    // its actually not in_desc_index - but address computed for index.
    // assembly will take care of that part.
    modify_field(te0_phv.table0_addr, in_desc_index);
    modify_field(te0_phv.table0_pc, 0);
    modify_field(te0_phv.table0_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te0_phv.table0_lock_en, 0);

}


@pragma stage 1
@pragma raw_table common_te_phv.table3_pc
table input_page_semaphore {
    reads {
        rxdma_scratch.in_page_semaphore : exact;
    }
    actions {
       allocate_input_page_semaphore;
    }
    size : 1;
}

@pragma stage 1
@pragma raw_table common_te_phv.table2_pc
table output_page_semaphore {
    reads {
        rxdma_scratch.out_page_semaphore : exact;
    }
    actions {
        allocate_output_page_semaphore;
    }
    size : 1;
}

@pragma stage 1
@pragma raw_table common_te_phv.table1_pc
table output_desc_semaphore {
    reads {
        rxdma_scratch.out_desc_semaphore : exact;
    }
    actions {
        allocate_output_desc_semaphore;
    }
    size : 1;
}

@pragma stage 1
@pragma raw_table common_te0_phv.table0_pc
table input_desc_semaphore {
    reads {
        rxdma_scratch.in_desc_semaphore : exact;
    }
    actions {
       allocate_input_desc_semaphore;
    }
    size : 1;
}

//stage 1
control ipsec_esp_v4_tun_h2n_read_semaphores {
    apply(input_desc_semaphore);
    apply(output_desc_semaphore);
    apply(input_page_semaphore);
    apply(output_page_semaphore);
}

action ipsec_encap_rxdma_initial_table(key_index, iv_size, icv_size,
                                 spi, esn_lo, iv, esn_hi,
                                 barco_enc_cmd, ipsec_cb_index, block_size)
{
    modify_field(ipsec_cb.key_index, key_index);
    modify_field(ipsec_cb.iv_size, iv_size);
    modify_field(ipsec_cb.icv_size, icv_size);
    modify_field(ipsec_cb.spi, spi);
    modify_field(ipsec_cb.esn_lo, esn_lo);
    modify_field(ipsec_cb.iv, iv);
    modify_field(ipsec_cb.esn_hi, esn_hi);
    modify_field(ipsec_cb.barco_enc_cmd, barco_enc_cmd);
    modify_field(ipsec_cb.ipsec_cb_index, ipsec_cb_index);
    modify_field(ipsec_cb.block_size, block_size);

    // based on ipsec_esp_v4_tun_h2n_encap_init
    modify_field(ipsec_int_header.ipsec_int_headroom, ESP_FIXED_HDR_SIZE+iv_size);
    modify_field(ipsec_int_header.ipsec_int_headroom_offset, p42p4plus_hdr.ipsec_payload_start);
    modify_field(ipsec_int_header.ipsec_int_tailroom_offset, p42p4plus_hdr.ipsec_payload_end+ESP_FIXED_HDR_SIZE+iv_size);

    modify_field(rxdma_scratch.payload_size, (p42p4plus_hdr.ipsec_payload_end - p42p4plus_hdr.ipsec_payload_start));
    modify_field(ipsec_int_header.ipsec_int_pad_size, block_size - ((p42p4plus_hdr.ipsec_payload_end - p42p4plus_hdr.ipsec_payload_start) & block_size));
    modify_field(ipsec_int_header.ipsec_int_tailroom, ipsec_int_header.ipsec_int_pad_size + 2 + icv_size);
    modify_field(ipsec_int_header.ipsec_int_buf_size, capri_p4.frame_size + ipsec_int_header.ipsec_int_headroom+ipsec_int_header.ipsec_int_tailroom);

    //set for stage1 - 4 tables for 4 semaphores

    modify_field(p42p4plus_hdr.table0_valid, 1);
    modify_field(p42p4plus_hdr.table1_valid, 1);
    modify_field(p42p4plus_hdr.table2_valid, 1);
    modify_field(p42p4plus_hdr.table3_valid, 1);

    // will K+D complaint issue come ??
    modify_field(te0_phv.table0_addr, INDESC_SEMAPHORE_ADDR);
    // put the right pc value here
    modify_field(te0_phv.table0_pc, 0);
    modify_field(te0_phv.table0_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te0_phv.table0_lock_en, 0);

    modify_field(te_phv.table1_addr, OUTDESC_SEMAPHORE_ADDR);
    modify_field(te_phv.table1_pc, 0); // put the right pc value here
    modify_field(te_phv.table1_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te_phv.table1_lock_en, 0);

    modify_field(te_phv.table2_addr, INPAGE_SEMAPHORE_ADDR);
    modify_field(te_phv.table2_pc, 0); // put the right pc value here
    modify_field(te_phv.table2_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te_phv.table2_lock_en, 0);

    modify_field(te_phv.table3_addr, OUTPAGE_SEMAPHORE_ADDR);
    modify_field(te_phv.table3_pc, 0); // put the right pc value here
    modify_field(te_phv.table3_raw_table_size, RAW_TABLE_SIZE_8);
    modify_field(te_phv.table3_lock_en, 0);
}

@pragma stage 0
table ipsec_encap_rxdma_initial_table {
    // this will change to 52 bytes of app-header.
    reads {
        capri_global_intrinsic.lif  : exact;
        rxdma_intrinsic.qtype : exact;
        rxdma_intrinsic.qid : exact;
    }
    actions {
        ipsec_encap_rxdma_initial_table;
    }
    size : IPSEC_CB_SIZE;
}

control ipsec_esp_v4_tun_h2n_encap_init {
    apply(ipsec_encap_rxdma_initial_table);
}

parser start {
    return ingress;
}

control ingress {
   ipsec_esp_v4_tun_h2n_encap_init();
   ipsec_esp_v4_tun_h2n_read_semaphores();
   ipsec_esp_v4_tun_h2n_get_desc_and_page_addrs();
   ipsec_esp_v4_tun_h2n_get_desc_and_page_entry();
   ipsec_esp_v4_tun_h2n_do_pkt_dma();

}

