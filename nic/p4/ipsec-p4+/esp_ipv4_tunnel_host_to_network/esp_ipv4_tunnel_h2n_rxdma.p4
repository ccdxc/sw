#include "../../common-p4+/common_rxdma_dummy.p4"

#define rx_table_s0_t0_action ipsec_encap_rxdma_initial_table 
#define common_p4plus_stage0_app_header_table_action_dummy      ipsec_encap_rxdma_initial_table 
#define rx_table_s0_t1_action ipsec_encap_rxdma_initial_table

#define rx_table_s1_t0_action allocate_input_desc_semaphore 
#define rx_table_s1_t1_action allocate_output_desc_semaphore 
#define rx_table_s1_t2_action allocate_input_page_semaphore 
#define rx_table_s1_t3_action allocate_output_page_semaphore 

#define rx_table_s2_t0_action allocate_input_desc_index 
#define rx_table_s2_t1_action allocate_output_desc_index 
#define rx_table_s2_t2_action allocate_input_page_index 
#define rx_table_s2_t3_action allocate_output_page_index 

#define rx_table_s3_t0_action update_input_desc_aol 
#define rx_table_s3_t1_action update_output_desc_aol 
#define rx_table_s3_t2_action read_random_number_from_barco 
#define rx_table_s3_t3_action update_input_desc_aol2 

#define rx_table_s4_t0_action ipsec_cb_tail_enqueue_input_desc 
#define rx_table_s4_t2_action ipsec_cb_tail_enqueue_input_desc2 

#include "../../common-p4+/common_rxdma.p4"
#include "esp_ipv4_tunnel_h2n_headers.p4"
#include "../ipsec_defines.h"

header_type ipsec_random_number_t {
    fields {
        random_number1 : 64;
        random_number2 : 64;
    }
}

header_type ipsec_to_stage2_t {
    fields {
        is_random : 8;
        stage2_pad1 : 56;
        stage2_pad2 : 64;
    }
}

header_type ipsec_to_stage3_t {
    fields {
        pad_addr : ADDRESS_WIDTH;
        pad_size : 8;
        iv_size  : 8;
        packet_len : 16;
        iv_salt  : 32;
    }
}

header_type ipsec_to_stage4_t {
    fields {
        out_desc_addr : ADDRESS_WIDTH; 
        to_stage_4_pad : 64;
    }
}

header_type ipsec_table0_s2s {
    fields {
        in_desc_addr : ADDRESS_WIDTH;
        in_page_addr : ADDRESS_WIDTH;
        payload_size : 16;
        payload_start : 16;
    }
}

header_type ipsec_table1_s2s {
    fields {
        out_desc_addr : ADDRESS_WIDTH;
        out_page_addr : ADDRESS_WIDTH; 
        payload_size : 16;
        payload_start : 16;
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
        in_page_addr : ADDRESS_WIDTH;
        out_page_addr : ADDRESS_WIDTH; 
        s2s3_pad : 32;
    }
}

header_type ipsec_rxdma_global_t {
    fields {
        lif            : 11;
        qtype          : 3;
        rxdma_pad1     : 2;
        qid            : 24;
        frame_size     : 16; 
        ipsec_cb_index : 16;
        ipsec_cb_pindex : 8;
        ipsec_cb_addr : 34;
        ipsec_global_pad : 14;
    }
}

header_type doorbell_data_pad_t {
    fields {
        db_data_pad : 64;
    }
}

//Unionize the below with p4_2_p4plus_app_header_t
@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_ipsec_header_t p42p4plus_hdr;

@pragma scratch_metadata
metadata p4_to_p4plus_ipsec_header_t p42p4plus_scratch_hdr;

//Unionize the below
@pragma pa_header_union ingress common_t0_s2s
metadata ipsec_table0_s2s t0_s2s;
@pragma pa_header_union ingress common_t1_s2s
metadata ipsec_table1_s2s t1_s2s;
@pragma pa_header_union ingress common_t2_s2s
metadata ipsec_table2_s2s t2_s2s;
@pragma pa_header_union ingress common_t3_s2s
metadata ipsec_table3_s2s t3_s2s;

@pragma scratch_metadata
metadata ipsec_table0_s2s scratch_t0_s2s;
@pragma scratch_metadata
metadata ipsec_table1_s2s scratch_t1_s2s;
@pragma scratch_metadata
metadata ipsec_table2_s2s scratch_t2_s2s;
@pragma scratch_metadata
metadata ipsec_table3_s2s scratch_t3_s2s;

//Global
@pragma pa_header_union ingress common_global
metadata ipsec_rxdma_global_t ipsec_global;

@pragma scratch_metadata
metadata ipsec_rxdma_global_t ipsec_global_scratch;

@pragma scratch_metadata
metadata ipsec_random_number_t ipsec_random_number_scratch;

//to_stage
@pragma pa_header_union ingress to_stage_2
metadata ipsec_to_stage2_t ipsec_to_stage2;
@pragma scratch_metadata
metadata ipsec_to_stage2_t ipsec_to_stage2_scratch;

@pragma pa_header_union ingress to_stage_3
metadata ipsec_to_stage3_t ipsec_to_stage3;
@pragma scratch_metadata
metadata ipsec_to_stage3_t ipsec_to_stage3_scratch;

@pragma pa_header_union ingress to_stage_4
metadata ipsec_to_stage4_t ipsec_to_stage4;

//RXDMA - IPsec feature specific scratch
@pragma dont_trim
metadata ipsec_int_header_t ipsec_int_header;
@pragma dont_trim
metadata esp_header_t esp_header;

@pragma dont_trim
metadata barco_descriptor_t barco_desc_in;
@pragma dont_trim
metadata barco_descriptor_t barco_desc_out;


@pragma dont_trim
metadata doorbell_data_t db_data;
//@pragma dont_trim
//metadata doorbell_data_pad_t db_data_pad;

//dma commands
@pragma dont_trim
metadata dma_cmd_pkt2mem_t dma_cmd_pkt2mem;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_phv2mem_ipsec_int;

@pragma dont_trim
metadata dma_cmd_phv2mem_t tail_2_bytes;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_fill_esp_hdr;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_in_desc_aol; 
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_out_desc_aol; 
@pragma dont_trim
metadata dma_cmd_mem2mem_t dma_cmd_pad_byte_src;
@pragma dont_trim
metadata dma_cmd_mem2mem_t dma_cmd_pad_byte_dst;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_iv_salt; 

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_iv; 

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_post_cb_ring;

@pragma dont_trim
metadata dma_cmd_phv2mem_t doorbell_cmd; 

@pragma scratch_metadata
metadata ipsec_cb_metadata_t ipsec_cb_scratch;


#define IPSEC_SCRATCH_T0_S2S \
    modify_field(scratch_t0_s2s.in_desc_addr, t0_s2s.in_desc_addr); \
    modify_field(scratch_t0_s2s.in_page_addr, t0_s2s.in_page_addr); \
    modify_field(scratch_t0_s2s.payload_size, t0_s2s.payload_size); \
    modify_field(scratch_t0_s2s.payload_start, t0_s2s.payload_start); 

#define IPSEC_SCRATCH_T1_S2S \
    modify_field(scratch_t1_s2s.out_desc_addr, t1_s2s.out_desc_addr); \
    modify_field(scratch_t1_s2s.out_page_addr, t1_s2s.out_page_addr); \
    modify_field(scratch_t1_s2s.payload_size, t1_s2s.payload_size); \
    modify_field(scratch_t1_s2s.payload_start, t1_s2s.payload_start); 

#define IPSEC_SCRATCH_T2_S2S \
    modify_field(scratch_t2_s2s.in_desc_addr, t2_s2s.in_desc_addr); \
    modify_field(scratch_t2_s2s.in_page_addr, t2_s2s.in_page_addr); \
    modify_field(scratch_t2_s2s.s2s2_pad, t2_s2s.s2s2_pad);

#define IPSEC_SCRATCH_T3_S2S \
    modify_field(scratch_t3_s2s.in_page_addr, t3_s2s.in_page_addr); \
    modify_field(scratch_t3_s2s.out_page_addr, t3_s2s.out_page_addr); \
    modify_field(scratch_t3_s2s.s2s3_pad, t3_s2s.s2s3_pad);

#define IPSEC_SCRATCH_GLOBAL \
    modify_field(ipsec_global_scratch.lif, ipsec_global.lif); \
    modify_field(ipsec_global_scratch.qtype, ipsec_global.qtype); \
    modify_field(ipsec_global_scratch.qid, ipsec_global.qid); \
    modify_field(ipsec_global_scratch.ipsec_cb_index, ipsec_global.ipsec_cb_index); \
    modify_field(ipsec_global_scratch.ipsec_cb_addr, ipsec_global.ipsec_cb_addr); \
    modify_field(ipsec_global_scratch.ipsec_cb_pindex, ipsec_global.ipsec_cb_pindex); \

//stage 4 - table 0
action ipsec_cb_tail_enqueue_input_desc2(pc, rsvd, cosA, cosB, cos_sel,
                                       eval_last, host, total, pid,
                                       rxdma_ring_pindex, rxdma_ring_cindex,
                                       barco_ring_pindex, barco_ring_cindex,
                                       key_index, iv_size, icv_size, spi,
                                       esn_lo, iv, barco_enc_cmd,
                                       ipsec_cb_index, block_size,
                                       cb_pindex, cb_cindex, barco_pindex, barco_cindex,
                                       cb_ring_base_addr_hi, cb_ring_base_addr,
                                       barco_ring_base_addr_hi, barco_ring_base_addr, 
                                       iv_salt, flags)
{
    IPSEC_CB_SCRATCH_WITH_PC
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T2_S2S
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_out_desc_aol, ipsec_to_stage4.out_desc_addr+64, IPSEC_OUT_DESC_AOL_START, IPSEC_OUT_DESC_AOL_END, 0, 0, 0, 0)
}

//stage 4 - table 0
action ipsec_cb_tail_enqueue_input_desc(pc, rsvd, cosA, cosB, cos_sel,
                                       eval_last, host, total, pid,
                                       rxdma_ring_pindex, rxdma_ring_cindex,
                                       barco_ring_pindex, barco_ring_cindex,
                                       key_index, iv_size, icv_size, spi,
                                       esn_lo, iv, barco_enc_cmd,
                                       ipsec_cb_index, block_size,
                                       cb_pindex, cb_cindex, barco_pindex, barco_cindex,
                                       cb_ring_base_addr_hi, cb_ring_base_addr,
                                       barco_ring_base_addr_hi, barco_ring_base_addr, 
                                       iv_salt, flags)
{
    IPSEC_CB_SCRATCH_WITH_PC

    // 2. ipsec_int_header to input-descriptor scratch phv2mem
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_phv2mem_ipsec_int, t0_s2s.in_desc_addr, IPSEC_INT_START_OFFSET, IPSEC_INT_END_OFFSET, 0, 0, 0, 0)

    // 3. input-descriptor and output descriptor AOLs phv2mem - can I do table write instead ?? else I need 1K phv bits.
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_in_desc_aol, t0_s2s.in_desc_addr+64, IPSEC_IN_DESC_AOL_START, IPSEC_IN_DESC_AOL_END, 0, 0, 0, 0)

    // Need to change to out_desc_addr
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_out_desc_aol, ipsec_to_stage4.out_desc_addr+64, IPSEC_OUT_DESC_AOL_START, IPSEC_OUT_DESC_AOL_END, 0, 0, 0, 0)

    //DMA_COMMAND_PHV2MEM_FILL(dma_cmd_post_cb_ring, cb_ring_base_addr + (cb_pindex * 8), IPSEC_CB_RING_IN_DESC_START, IPSEC_CB_RING_IN_DESC_END, 0, 0, 0, 0)

    //DMA_COMMAND_PHV2MEM_FILL(dma_cmd_fill_esp_hdr, t0_s2s.in_page_addr+IPSEC_SALT_HEADROOM+iv_size, IPSEC_ESP_HEADER_PAGE_START, IPSEC_ESP_HEADER_PAGE_END, 0, 0, 0 0);
    modify_field(p4_rxdma_intr.dma_cmd_ptr, RXDMA_IPSEC_DMA_COMMANDS_OFFSET);
    // Ring Doorbell for IPSec-CB (svc_lif, type, ipsec-cb-index(qid))
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T0_S2S
}

//stage 3
action read_random_number_from_barco(random_number1, random_number2)
{
    modify_field( ipsec_random_number_scratch.random_number1, random_number1);
    modify_field( ipsec_random_number_scratch.random_number2, random_number2);
}

//stage 3
action update_output_desc_aol(addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    modify_field(barco_desc_out.A0_addr, t1_s2s.out_page_addr);
    modify_field(barco_desc_out.O0, 0);
    modify_field(barco_desc_out.L0, 0); 
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T1_S2S
    modify_field(ipsec_to_stage3_scratch.iv_salt, ipsec_to_stage3.iv_salt); 
    modify_field(ipsec_to_stage3_scratch.iv_size, ipsec_to_stage3.iv_size); 
    modify_field(ipsec_to_stage3_scratch.packet_len, ipsec_to_stage3.packet_len); 
    modify_field(ipsec_to_stage3_scratch.pad_addr, ipsec_to_stage3.pad_addr); 
    modify_field(ipsec_to_stage3_scratch.pad_size, ipsec_to_stage3.pad_size); 
}

//stage 3 
action update_input_desc_aol2 (addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T3_S2S
    modify_field(ipsec_to_stage3_scratch.iv_size, ipsec_to_stage3.iv_size);
    // Original pkt to input descriptor pkt2mem 
    DMA_COMMAND_PKT2MEM_FILL(dma_cmd_pkt2mem, addr0+4+ipsec_to_stage3.iv_size, ipsec_to_stage3.packet_len, 0, 0)
    // Pad bytes from HBM - mem2mem 
    DMA_COMMAND_MEM2MEM_FILL(dma_cmd_pad_byte_src, dma_cmd_pad_byte_dst, ipsec_to_stage3.pad_addr, 0, addr0+length0, 0, ipsec_to_stage3.pad_size, 0, 0, 0)
    // DMA_COMMAND SALT
    modify_field(ipsec_to_stage3_scratch.iv_salt, ipsec_to_stage3.iv_salt); 
    modify_field(ipsec_to_stage3_scratch.iv_size, ipsec_to_stage3.iv_size); 
    modify_field(ipsec_to_stage3_scratch.packet_len, ipsec_to_stage3.packet_len); 
    modify_field(ipsec_to_stage3_scratch.pad_addr, ipsec_to_stage3.pad_addr); 
    modify_field(ipsec_to_stage3_scratch.pad_size, ipsec_to_stage3.pad_size); 

}

//stage 3
action update_input_desc_aol (addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    //modify_field(barco_desc_in.A0_addr, t0_s2s.in_page_addr);
    modify_field(barco_desc_in.O0, 0);
    //modify_field(barco_desc_in.L0, ipsec_global.frame_size - IPSEC_RXDMA_HW_SW_INTRINSIC_SIZE); 

    // Load ipsec-cb-again
    modify_field(p42p4plus_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0); 
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, (ipsec_global.ipsec_cb_index * IPSEC_CB_SIZE) + IPSEC_CB_BASE);

    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T0_S2S
    // Original pkt to input descriptor pkt2mem 
    DMA_COMMAND_PKT2MEM_FILL(dma_cmd_pkt2mem, addr0+4+ipsec_to_stage3.iv_size, ipsec_to_stage3.packet_len, 0, 0)
    // Pad bytes from HBM - mem2mem 
    DMA_COMMAND_MEM2MEM_FILL(dma_cmd_pad_byte_src, dma_cmd_pad_byte_dst, ipsec_to_stage3.pad_addr, 0, addr0+length0, 0, ipsec_to_stage3.pad_size, 0, 0, 0)
    // DMA_COMMAND SALT
    modify_field(ipsec_to_stage3_scratch.iv_salt, ipsec_to_stage3.iv_salt); 
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_iv_salt, addr0, IPSEC_IN_DESC_IV_SALT_START, IPSEC_IN_DESC_IV_SALT_END, 0, 0, 0, 0) 
    // DMA IV to beginning of INPUT DESC 
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_iv, addr0+IPSEC_SALT_HEADROOM, IPSEC_IN_DESC_IV_START, IPSEC_IN_DESC_IV_END, 0, 0, 0, 0)
    modify_field(ipsec_to_stage3_scratch.iv_salt, ipsec_to_stage3.iv_salt); 
    modify_field(ipsec_to_stage3_scratch.iv_size, ipsec_to_stage3.iv_size); 
    modify_field(ipsec_to_stage3_scratch.packet_len, ipsec_to_stage3.packet_len); 
    modify_field(ipsec_to_stage3_scratch.pad_addr, ipsec_to_stage3.pad_addr); 
    modify_field(ipsec_to_stage3_scratch.pad_size, ipsec_to_stage3.pad_size); 
}

//stage 2 
action allocate_output_page_index(out_page_index)
{
    modify_field(p42p4plus_hdr.table3_valid, 1);
    modify_field(common_te3_phv.table_pc, 0); 
    modify_field(common_te3_phv.table_raw_table_size, 3);
    modify_field(common_te3_phv.table_lock_en, 0);
    modify_field(common_te3_phv.table_addr, OUT_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * out_page_index));
    modify_field(ipsec_int_header.out_page, OUT_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * out_page_index));
    modify_field(t1_s2s.out_page_addr, OUT_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * out_page_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T3_S2S
}

//stage 2 
action allocate_input_page_index(in_page_index)
{
    modify_field(p42p4plus_hdr.table2_valid, 1);
    modify_field(common_te2_phv.table_pc, 0); 
    modify_field(common_te2_phv.table_raw_table_size, 3);
    modify_field(common_te2_phv.table_lock_en, 0);
    modify_field(common_te2_phv.table_addr, IN_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * in_page_index));
    modify_field(ipsec_int_header.in_page,  IN_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * in_page_index)); 
    modify_field(t0_s2s.in_page_addr, IN_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * in_page_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T2_S2S
    if (ipsec_to_stage2.is_random == 1) {
        modify_field(p42p4plus_hdr.table2_valid, 1);
        modify_field(common_te2_phv.table_pc, 0);  //random-number read PC
        modify_field(common_te2_phv.table_raw_table_size, 3); // 8 bytes or 16 bytes 
        modify_field(common_te2_phv.table_addr, 0); // some hash define of barco crypt mem for now
    }
}

//stage 2 
action allocate_output_desc_index(out_desc_index)
{
    modify_field(p42p4plus_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0); 
    modify_field(common_te1_phv.table_raw_table_size, 3);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, OUT_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * out_desc_index));
    modify_field(ipsec_int_header.out_desc, OUT_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * out_desc_index));
    modify_field(t1_s2s.out_desc_addr, OUT_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * out_desc_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T1_S2S
}

//stage 2 
action allocate_input_desc_index(in_desc_index)
{
    modify_field(p42p4plus_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0); 
    modify_field(common_te0_phv.table_raw_table_size, 3);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, IN_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * in_desc_index));
    modify_field(ipsec_int_header.in_desc, IN_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * in_desc_index));
    modify_field(t0_s2s.in_desc_addr, IN_DESC_ADDR_BASE++(DESC_ENTRY_SIZE* in_desc_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T0_S2S
}

//stage 1
action allocate_output_page_semaphore(out_page_ring_index)
{
    modify_field(p42p4plus_hdr.table3_valid, 1);
    modify_field(common_te3_phv.table_pc, 0); 
    modify_field(common_te3_phv.table_raw_table_size, 3);
    modify_field(common_te3_phv.table_lock_en, 0);
    modify_field(common_te3_phv.table_addr, OUT_PAGE_RING_BASE+(PAGE_PTR_SIZE * out_page_ring_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T3_S2S
}

//stage 1
action allocate_input_page_semaphore(in_page_ring_index)
{
    modify_field(p42p4plus_hdr.table2_valid, 1);
    modify_field(common_te2_phv.table_pc, 0); 
    modify_field(common_te2_phv.table_raw_table_size, 3);
    modify_field(common_te2_phv.table_lock_en, 0);
    modify_field(common_te2_phv.table_addr, IN_PAGE_RING_BASE+(PAGE_PTR_SIZE * in_page_ring_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T2_S2S
}

//stage 1
action allocate_output_desc_semaphore(out_desc_ring_index)
{
    modify_field(p42p4plus_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0); 
    modify_field(common_te1_phv.table_raw_table_size, 3);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, OUT_DESC_RING_BASE+(DESC_PTR_SIZE * out_desc_ring_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T1_S2S
}

//stage 1
action allocate_input_desc_semaphore(in_desc_ring_index)
{
    modify_field(p42p4plus_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0); 
    modify_field(common_te0_phv.table_raw_table_size, 3);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, IN_DESC_RING_BASE+(DESC_PTR_SIZE * in_desc_ring_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T0_S2S
}

//stage 0
action ipsec_encap_rxdma_initial_table(rsvd, cosA, cosB, cos_sel, 
                                       eval_last, host, total, pid,
                                       rxdma_ring_pindex, rxdma_ring_cindex, 
                                       barco_ring_pindex, barco_ring_cindex,
                                       key_index, iv_size, icv_size, spi,
                                       esn_lo, iv, barco_enc_cmd,
                                       ipsec_cb_index, block_size, 
                                       cb_pindex, cb_cindex, barco_pindex, 
                                       barco_cindex, cb_ring_base_addr_hi, cb_ring_base_addr, 
                                       barco_ring_base_addr_hi, barco_ring_base_addr, iv_salt, flags)
{
    IPSEC_CB_SCRATCH

    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    modify_field(ipsec_global.ipsec_cb_addr, p4_rxdma_intr.qstate_addr);
    // we do not need the other IPSec-CB content right now - we need it in txdma1
    modify_field(ipsec_int_header.ipsec_cb_index, ipsec_cb_index);

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
    modify_field(ipsec_to_stage3.packet_len, p42p4plus_hdr.ipsec_payload_end);

    //  modify_field(ipsec_int_header.tailroom, ipsec_int_header.pad_size + 2 + icv_size);
    // modify_field(ipsec_int_header.buf_size, p4_intr.frame_size + ipsec_int_header.headroom+ipsec_int_header.tailroom);
    modify_field(ipsec_int_header.l4_protocol, p42p4plus_hdr.l4_protocol);

    // Lif, type, qid
    modify_field(ipsec_global.lif, p4_intr_global.lif);
    modify_field(ipsec_global.qtype, p4_rxdma_intr.qtype);
    modify_field(ipsec_global.qid, p4_rxdma_intr.qid);

    modify_field(p42p4plus_scratch_hdr.p4plus_app_id, p42p4plus_hdr.p4plus_app_id);
    modify_field(p42p4plus_scratch_hdr.table0_valid, p42p4plus_hdr.table0_valid);
    modify_field(p42p4plus_scratch_hdr.table1_valid, p42p4plus_hdr.table1_valid);
    modify_field(p42p4plus_scratch_hdr.table2_valid, p42p4plus_hdr.table2_valid);
    modify_field(p42p4plus_scratch_hdr.table3_valid, p42p4plus_hdr.table3_valid);

    modify_field(ipsec_to_stage3.iv_size, iv_size);
    modify_field(ipsec_to_stage3.iv_salt, iv_salt);

    if (flags & IPSEC_FLAGS_RANDOM_MASK == IPSEC_FLAGS_RANDOM_MASK) {
       modify_field(ipsec_to_stage2.is_random, 1);
    }

    modify_field(p42p4plus_hdr.table0_valid, 1);
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
