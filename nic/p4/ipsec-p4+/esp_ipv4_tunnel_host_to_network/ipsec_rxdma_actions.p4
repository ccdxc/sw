#define rx_table_s0_t0_action ipsec_encap_rxdma_initial_table 
#define common_p4plus_stage0_app_header_table_action ipsec_encap_rxdma_initial_table 
#define rx_table_s0_t1_action ipsec_rxdma_dummy 
#define rx_table_s0_t2_action ipsec_rxdma_dummy 
#define rx_table_s0_t3_action ipsec_rxdma_dummy 

#define rx_table_s1_t0_action allocate_input_desc_semaphore 
#define rx_table_s1_t1_action allocate_output_desc_semaphore 
#define rx_table_s1_t2_action allocate_input_page_semaphore 
#define rx_table_s1_t3_action allocate_output_page_semaphore 

#define rx_table_s2_t0_action update_input_desc_aol 
#define rx_table_s2_t1_action update_output_desc_aol 
#define rx_table_s2_t2_action ipsec_rxdma_dummy
#define rx_table_s2_t3_action ipsec_rxdma_dummy

#define rx_table_s3_t0_action ipsec_cb_tail_enqueue_input_desc 
#define rx_table_s3_t1_action ipsec_rxdma_dummy
#define rx_table_s3_t2_action ipsec_rxdma_dummy
#define rx_table_s3_t3_action ipsec_rxdma_dummy

#define rx_table_s4_t0_action ipsec_rxdma_dummy
#define rx_table_s4_t1_action ipsec_rxdma_dummy
#define rx_table_s4_t2_action ipsec_rxdma_dummy
#define rx_table_s4_t3_action ipsec_rxdma_dummy

#define rx_table_s5_t0_action ipsec_rxdma_dummy
#define rx_table_s5_t1_action ipsec_rxdma_dummy
#define rx_table_s5_t2_action ipsec_rxdma_dummy
#define rx_table_s5_t3_action ipsec_rxdma_dummy

#define rx_table_s6_t0_action ipsec_rxdma_dummy
#define rx_table_s6_t1_action ipsec_rxdma_dummy
#define rx_table_s6_t2_action ipsec_rxdma_dummy
#define rx_table_s6_t3_action ipsec_rxdma_dummy

#define rx_table_s7_t0_action ipsec_rxdma_dummy
#define rx_table_s7_t1_action ipsec_rxdma_dummy
#define rx_table_s7_t2_action ipsec_rxdma_dummy
#define rx_table_s7_t3_action ipsec_rxdma_dummy

#include "../../common-p4+/common_rxdma.p4"

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
	pad_size          : 8;
	payload_start     : 16;
	buf_size          : 16;
	payload_size      : 16;
        l4_protocol       : 8;
        ipsec_int_pad0    : 256;
    }
}

header_type ipsec_cb_metadata_t {
    fields {
        pid : 16;
        cos_b : 4;
        cos_a : 4;
        pc_offset : 8; 
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
        head_desc_addr : ADDRESS_WIDTH;
        tail_desc_addr : ADDRESS_WIDTH;
        ipsec_cb_pad : 40;
    }
}

// the below phv is going to be written to the descriptor memory by action func as phv2mem
// please refer to barco document - table3 for the latest format.
header_type barco_descriptor_t {
    fields {
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

header_type ipsec_to_stage2_t {
    fields {
        tail_desc_addr : ADDRESS_WIDTH; 
        stage2_pad     : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage3_t {
    fields {
        out_desc_addr : ADDRESS_WIDTH; 
        pad_addr : 34;
        pad_size : 8;
        to_stage_3_pad : 22;
    }
}

header_type ipsec_table0_s2s {
    fields {
        in_desc_addr : ADDRESS_WIDTH;
        in_page_addr : ADDRESS_WIDTH;
        s2s0_pad : 32;
    }
}

header_type ipsec_table1_s2s {
    fields {
        out_desc_addr : ADDRESS_WIDTH;
        out_page_addr : ADDRESS_WIDTH; 
        s2s1_pad : 32;
    }
}

header_type ipsec_table2_s2s {
    fields {
        in_desc_addr : ADDRESS_WIDTH;
        in_page_addr : ADDRESS_WIDTH; 
        s2s2_pad : 32;
    }
}

header_type ipsec_table3_s2s {
    fields {
        out_desc_addr : ADDRESS_WIDTH;
        out_page_addr : ADDRESS_WIDTH; 
        s2s3_pad : 32;
    }
}

header_type ipsec_rxdma_global_t {
    fields {
        in_desc_addr : 64;
        ipsec_cb_index : 12;
        lif            : 11;
        qtype          : 3;
        qid            : 24;
        frame_size     : 14; 
    }
}



//Unionize the below with p4_2_p4plus_app_header_t
@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_ipsec_header_t p42p4plus_hdr;

//Unionize the below
@pragma pa_header_union ingress common_t0_s2s
metadata ipsec_table0_s2s t0_s2s;
@pragma pa_header_union ingress common_t1_s2s
metadata ipsec_table1_s2s t1_s2s;
@pragma pa_header_union ingress common_t2_s2s
metadata ipsec_table2_s2s t2_s2s;
@pragma pa_header_union ingress common_t3_s2s
metadata ipsec_table3_s2s t3_s2s;

//Global
@pragma pa_header_union ingress common_global
metadata ipsec_rxdma_global_t ipsec_global;

//to_stage
@pragma pa_header_union ingress to_stage_2
metadata ipsec_to_stage2_t ipsec_to_stage2;

@pragma pa_header_union ingress to_stage_3
metadata ipsec_to_stage3_t ipsec_to_stage3;

//RXDMA - IPsec feature specific scratch
@pragma dont_trim
metadata ipsec_int_header_t ipsec_int_header;
@pragma dont_trim
metadata barco_descriptor_t barco_desc_in;
@pragma dont_trim
metadata barco_descriptor_t barco_desc_out;


//dma commands
@pragma dont_trim
metadata dma_cmd_pkt2mem_t dma_cmd_pkt2mem;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_phv2mem_ipsec_int;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_in_desc_aol; 
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_out_desc_aol; 
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_tail_desc_addr; 
@pragma dont_trim
metadata dma_cmd_mem2mem_t dma_cmd_pad_byte_src;
@pragma dont_trim
metadata dma_cmd_mem2mem_t dma_cmd_pad_byte_dst;

@pragma scratch_metadata
metadata ipsec_cb_metadata_t ipsec_cb_scratch;


//action ipsec_rxdma_dummy (data0, data1) 
action ipsec_rxdma_dummy () 
{
}


// Enqueue the input-descriptor at the tail of ipsec-cb
// 1. Read the tail descriptor table from tail_desc_addr+64 bytes.
// 2. rsvd field should be zero there. 
// 3. Now table write the rsvd with newly allocated input-descriptor
// 4. Update the input-descriptor rsvd to zero (it would be anyway be zero - but just to make sure)
//stage 3 - table 0
action ipsec_cb_tail_enqueue_input_desc (addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    //table write in_desc into rsvd value of tail_desc - write this part in assembly.
    // pass the in_desc value in s2s data or global data. 
    modify_field(t0_s2s.in_desc_addr, ipsec_global.in_desc_addr);

    // do not try to understand the below line - just for K+I generation to get ipsec_cb_index
    modify_field(t0_s2s.in_page_addr, ipsec_global.ipsec_cb_index); 
    // DMA Commands
    // 1. Original pkt to input descriptor pkt2mem 
    DMA_COMMAND_PKT2MEM_FILL(dma_cmd_pkt2mem, addr0, length0, 0, 0)

    // 2. ipsec_int_header to input-descriptor scratch phv2mem
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_phv2mem_ipsec_int, ipsec_global.in_desc_addr, IPSEC_INT_START_OFFSET, IPSEC_INT_END_OFFSET, 0, 0, 0, 0)

    // 3. input-descriptor and output descriptor AOLs phv2mem - can I do table write instead ?? else I need 1K phv bits.
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_in_desc_aol, ipsec_global.in_desc_addr+64, IPSEC_IN_DESC_AOL_START, IPSEC_IN_DESC_AOL_END, 0, 0, 0, 0)

    // Need to change to out_desc_addr
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_out_desc_aol, ipsec_to_stage3.out_desc_addr+64, IPSEC_OUT_DESC_AOL_START, IPSEC_OUT_DESC_AOL_END, 0, 0, 0, 0)

    // 4. Pad bytes from HBM - mem2mem 
    DMA_COMMAND_MEM2MEM_FILL(dma_cmd_pad_byte_src, dma_cmd_pad_byte_dst, ipsec_to_stage3.pad_addr, 0, addr0+length0, 0, ipsec_to_stage3.pad_size, 0, 0, 0)
    // 5. DMA Command to write tail_desc_addr in ipsec_cb
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_tail_desc_addr,  (ipsec_global.ipsec_cb_index * IPSEC_CB_SIZE) + IPSEC_CB_BASE + IPSEC_CB_TAIL_DESC_ADDR_OFFSET,  IPSEC_TAIL_DESC_ADDR_PHV_OFFSET_START, IPSEC_TAIL_DESC_ADDR_PHV_OFFSET_END, 0, 0, 0, 0)   


    modify_field(p4_rxdma_intr.dma_cmd_ptr, RXDMA_IPSEC_DMA_COMMANDS_OFFSET);
    // Ring Doorbell for IPSec-CB (svc_lif, type, ipsec-cb-index(qid))
}

//stage 2
action update_output_desc_aol(addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    // pass output page in s2s data K+I - change it later
    modify_field(barco_desc_in.A0_addr, t1_s2s.out_page_addr);
    modify_field(barco_desc_in.O0, 0);
    modify_field(barco_desc_in.L0, 0); 

    modify_field(barco_desc_in.A1_addr, 0);
    modify_field(barco_desc_in.O1, 0);
    modify_field(barco_desc_in.L1, 0);

    modify_field(barco_desc_in.A2_addr, 0);
    modify_field(barco_desc_in.O2, 0);
    modify_field(barco_desc_in.L2, 0);

}

//stage 2
action update_input_desc_aol (addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    // pass input page in s2s data K+I - change it later
    modify_field(barco_desc_out.A0_addr, t0_s2s.in_page_addr);
    modify_field(barco_desc_out.O0, 0);
    modify_field(barco_desc_out.L0, ipsec_global.frame_size - IPSEC_RXDMA_HW_SW_INTRINSIC_SIZE); 

    modify_field(barco_desc_out.A1_addr, 0);
    modify_field(barco_desc_out.O1, 0);
    modify_field(barco_desc_out.L1, 0);

    modify_field(barco_desc_out.A2_addr, 0);
    modify_field(barco_desc_out.O2, 0);
    modify_field(barco_desc_out.L2, 0);

    // Load tail-descriptor
    modify_field(p42p4plus_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0); 
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, ipsec_to_stage2.tail_desc_addr+64);
}

//stage 1
action allocate_output_page_semaphore(out_page_index)
{
    modify_field(p42p4plus_hdr.table3_valid, 1);
    modify_field(common_te3_phv.table_pc, 0); 
    modify_field(common_te3_phv.table_raw_table_size, 2);
    modify_field(common_te3_phv.table_lock_en, 0);
    modify_field(common_te3_phv.table_addr, OUT_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * out_page_index));
  
    modify_field(t1_s2s.out_page_addr, OUT_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * out_page_index));
}

//stage 1
action allocate_input_page_semaphore(in_page_index)
{
    modify_field(p42p4plus_hdr.table2_valid, 1);
    modify_field(common_te2_phv.table_pc, 0); 
    modify_field(common_te2_phv.table_raw_table_size, 2);
    modify_field(common_te2_phv.table_lock_en, 0);
    modify_field(common_te2_phv.table_addr, IN_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * in_page_index));

    modify_field(t0_s2s.in_page_addr, IN_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * in_page_index));
}


//stage 1
action allocate_output_desc_semaphore(out_desc_index)
{
    modify_field(p42p4plus_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0); 
    modify_field(common_te1_phv.table_raw_table_size, 2);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, OUT_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * out_desc_index));

    modify_field(t1_s2s.out_desc_addr, OUT_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * out_desc_index));
    modify_field(ipsec_to_stage3.out_desc_addr, OUT_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * out_desc_index));
}

//stage 1
action allocate_input_desc_semaphore(in_desc_index)
{
    modify_field(p42p4plus_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0); 
    modify_field(common_te0_phv.table_raw_table_size, 2);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, IN_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * in_desc_index));

    modify_field(t0_s2s.in_desc_addr, IN_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * in_desc_index));
    modify_field(ipsec_global.in_desc_addr, IN_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * in_desc_index));
}


//stage 0
action ipsec_encap_rxdma_initial_table(key_index, iv_size, icv_size,
                                       spi, esn_lo, iv, esn_hi,
                                       barco_enc_cmd, ipsec_cb_index, 
                                       block_size, head_desc_addr, tail_desc_addr)
{

    //Set all variables to scratch so that they are not removed 
    modify_field(ipsec_cb_scratch.key_index, key_index);
    modify_field(ipsec_cb_scratch.iv_size, iv_size);
    modify_field(ipsec_cb_scratch.icv_size, icv_size);
    modify_field(ipsec_cb_scratch.spi,spi);
    modify_field(ipsec_cb_scratch.esn_lo,esn_lo);
    modify_field(ipsec_cb_scratch.iv,iv);
    modify_field(ipsec_cb_scratch.esn_hi,esn_hi);
    modify_field(ipsec_cb_scratch.barco_enc_cmd,barco_enc_cmd);
    modify_field(ipsec_cb_scratch.block_size, block_size);
    modify_field(ipsec_cb_scratch.head_desc_addr, head_desc_addr);


    // we do not need the other IPSec-CB content right now - we need it in txdma1
    modify_field(ipsec_int_header.ipsec_cb_index, ipsec_cb_index);
    modify_field(ipsec_to_stage2.tail_desc_addr, tail_desc_addr);
 

    // based on ipsec_esp_v4_tun_h2n_encap_init
    modify_field(ipsec_int_header.headroom_offset, p42p4plus_hdr.ipsec_payload_start);
    //HLIR promoting the below to 16 bits un-necessarily
    //modify_field(ipsec_int_header.tailroom_offset, p42p4plus_hdr.ipsec_payload_end+ESP_FIXED_HDR_SIZE+iv_size);
    modify_field(ipsec_int_header.headroom, ESP_FIXED_HDR_SIZE+iv_size);

    modify_field(ipsec_int_header.payload_size, (p42p4plus_hdr.ipsec_payload_end - p42p4plus_hdr.ipsec_payload_start));
    modify_field(ipsec_int_header.pad_size, block_size - ((p42p4plus_hdr.ipsec_payload_end - p42p4plus_hdr.ipsec_payload_start) & block_size));


    // mem2mem HBM based ipsec pad-table
    //modify_field(ipsec_to_stage3.pad_addr, (block_size - (((p42p4plus_hdr.ipsec_payload_end - p42p4plus_hdr.ipsec_payload_start) & block_size)) * 256) + IPSEC_PAD_BYTES_TABLE_BASE );
    //modify_field(ipsec_to_stage3.pad_size, block_size - ((p42p4plus_hdr.ipsec_payload_end - p42p4plus_hdr.ipsec_payload_start) & block_size));
    
    //HLIR promoting block_size to 34 bits - do not want that
    modify_field(ipsec_to_stage3.pad_addr, 0);
    modify_field(ipsec_to_stage3.pad_size, 0);


    modify_field(ipsec_int_header.tailroom, ipsec_int_header.pad_size + 2 + icv_size);
    modify_field(ipsec_int_header.buf_size, p4_intr.frame_size + ipsec_int_header.headroom+ipsec_int_header.tailroom);
    modify_field(ipsec_int_header.l4_protocol, p42p4plus_hdr.l4_protocol);

    // Lif, type, qid
    modify_field(ipsec_global.lif, p4_intr_global.lif);
    modify_field(ipsec_global.qtype, p4_rxdma_intr.qtype);
    modify_field(ipsec_global.qid, p4_rxdma_intr.qid);
    modify_field(ipsec_global.frame_size, p4_intr.frame_size);

    
    // prepare tables for next stages

    modify_field(p42p4plus_hdr.table0_valid, 1);
    // need to fill pc address here
    modify_field(common_te0_phv.table_pc, 0); 
    modify_field(common_te0_phv.table_raw_table_size, 2);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, INDESC_SEMAPHORE_ADDR);

    modify_field(p42p4plus_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0); 
    modify_field(common_te1_phv.table_raw_table_size, 2);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, OUTDESC_SEMAPHORE_ADDR);

    modify_field(p42p4plus_hdr.table2_valid, 1);
    modify_field(common_te2_phv.table_pc, 0); 
    modify_field(common_te2_phv.table_raw_table_size, 2);
    modify_field(common_te2_phv.table_lock_en, 0);
    modify_field(common_te2_phv.table_addr, INPAGE_SEMAPHORE_ADDR);

    modify_field(p42p4plus_hdr.table3_valid, 1);
    modify_field(common_te3_phv.table_pc, 0); 
    modify_field(common_te3_phv.table_raw_table_size, 2);
    modify_field(common_te3_phv.table_lock_en, 0);
    modify_field(common_te3_phv.table_addr, OUTPAGE_SEMAPHORE_ADDR);

}


