#define rx_table_s0_t0_action esp_v4_tunnel_n2h_rxdma_initial_table 
#define common_p4plus_stage0_app_header_table_action_dummy esp_v4_tunnel_n2h_rxdma_initial_table
#define common_p4plus_stage0_app_header_table_action_dummy1     ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy2     ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy3     ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy4     ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy5     ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy6     ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy7     ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy8     ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy9     ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy10    ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy11    ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy12    ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy13    ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy14    ipsec_rxdma_dummy
#define common_p4plus_stage0_app_header_table_action_dummy15    ipsec_rxdma_dummy

#define rx_table_s0_t1_action ipsec_rxdma_dummy 
#define rx_table_s0_t2_action ipsec_rxdma_dummy 
#define rx_table_s0_t3_action ipsec_rxdma_dummy 

#define rx_table_s1_t0_action esp_v4_tunnel_n2h_allocate_input_desc_semaphore 
#define rx_table_s1_t1_action esp_v4_tunnel_n2h_allocate_output_desc_semaphore 
#define rx_table_s1_t2_action esp_v4_tunnel_n2h_allocate_input_page_semaphore 
#define rx_table_s1_t3_action esp_v4_tunnel_n2h_allocate_output_page_semaphore 

#define rx_table_s2_t0_action esp_v4_tunnel_n2h_allocate_input_desc_index 
#define rx_table_s2_t1_action esp_v4_tunnel_n2h_allocate_output_desc_index 
#define rx_table_s2_t2_action esp_v4_tunnel_n2h_allocate_input_page_index 
#define rx_table_s2_t3_action esp_v4_tunnel_n2h_allocate_output_page_index 

#define rx_table_s3_t0_action esp_v4_tunnel_n2h_update_input_desc_aol 
#define rx_table_s3_t1_action esp_v4_tunnel_n2h_update_output_desc_aol 
#define rx_table_s3_t2_action ipsec_rxdma_dummy
#define rx_table_s3_t3_action ipsec_rxdma_dummy

#define rx_table_s4_t0_action esp_v4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc 
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
#include "esp_v4_tunnel_n2h_headers.p4"

#include "../ipsec_defines.h"

header_type ipsec_to_stage3_t {
    fields {
        tail_desc_addr : ADDRESS_WIDTH; 
        stage2_pad     : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage4_t {
    fields {
        out_desc_addr : ADDRESS_WIDTH; 
        pad_size : 8;
        pad_addr : 40;
        to_stage_3_pad : 16;
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
        lif            : 11;
        qtype          : 3;
        rxdma_pad1     : 2;
        qid            : 24;
        frame_size     : 16; 
        ipsec_cb_index : 16;
        rxdma_pad22    : 56; 
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

//to_stage
@pragma pa_header_union ingress to_stage_3
metadata ipsec_to_stage3_t ipsec_to_stage3;

@pragma pa_header_union ingress to_stage_4
metadata ipsec_to_stage4_t ipsec_to_stage4;

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

@pragma scratch_metadata
metadata ipsec_cb_metadata_t ipsec_cb_scratch;

@pragma scratch_metadata
metadata p4_to_p4plus_ipsec_header_t p42p4plus_hdr_scratch;


#define IPSEC_SCRATCH_T0_S2S \
    modify_field(scratch_t0_s2s.in_desc_addr, t0_s2s.in_desc_addr); \
    modify_field(scratch_t0_s2s.in_page_addr, t0_s2s.in_page_addr); \
    modify_field(scratch_t0_s2s.s2s0_pad, t0_s2s.s2s0_pad);

#define IPSEC_SCRATCH_T1_S2S \
    modify_field(scratch_t1_s2s.out_desc_addr, t1_s2s.out_desc_addr); \
    modify_field(scratch_t1_s2s.out_page_addr, t1_s2s.out_page_addr); \
    modify_field(scratch_t1_s2s.s2s1_pad, t1_s2s.s2s1_pad);

#define IPSEC_SCRATCH_T2_S2S \
    modify_field(scratch_t2_s2s.in_desc_addr, t2_s2s.in_desc_addr); \
    modify_field(scratch_t2_s2s.in_page_addr, t2_s2s.in_page_addr); \
    modify_field(scratch_t2_s2s.s2s2_pad, t2_s2s.s2s2_pad);

#define IPSEC_SCRATCH_T3_S2S \
    modify_field(scratch_t3_s2s.out_desc_addr, t3_s2s.out_desc_addr); \
    modify_field(scratch_t3_s2s.out_page_addr, t3_s2s.out_page_addr); \
    modify_field(scratch_t3_s2s.s2s3_pad, t3_s2s.s2s3_pad);

#define IPSEC_SCRATCH_GLOBAL \
    modify_field(ipsec_global_scratch.lif, ipsec_global.lif); \
    modify_field(ipsec_global_scratch.qtype, ipsec_global.qtype); \
    modify_field(ipsec_global_scratch.qid, ipsec_global.qid); \
    modify_field(ipsec_global_scratch.ipsec_cb_index, ipsec_global.ipsec_cb_index); \
//modify_field(ipsec_global_scratch.frame_size, ipsec_global.frame_size);


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
action esp_v4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc (addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    //table write in_desc into rsvd value of tail_desc - write this part in assembly.
    // pass the in_desc value in s2s data or global data. 
    //modify_field(t0_s2s.in_desc_addr, ipsec_global.in_desc_addr);

    // do not try to understand the below line - just for K+I generation to get ipsec_cb_index
    modify_field(t0_s2s.in_page_addr, ipsec_global.ipsec_cb_index); 
    // DMA Commands
    // 1. Original pkt to input descriptor pkt2mem 
    DMA_COMMAND_PKT2MEM_FILL(dma_cmd_pkt2mem, addr0, length0, 0, 0)

    // 2. ipsec_int_header to input-descriptor scratch phv2mem
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_phv2mem_ipsec_int, t0_s2s.in_desc_addr, IPSEC_INT_START_OFFSET, IPSEC_INT_END_OFFSET, 0, 0, 0, 0)

    // 3. input-descriptor and output descriptor AOLs phv2mem - can I do table write instead ?? else I need 1K phv bits.
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_in_desc_aol, t0_s2s.in_desc_addr+64, IPSEC_IN_DESC_AOL_START, IPSEC_IN_DESC_AOL_END, 0, 0, 0, 0)

    // Need to change to out_desc_addr
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_out_desc_aol, ipsec_to_stage4.out_desc_addr+64, IPSEC_OUT_DESC_AOL_START, IPSEC_OUT_DESC_AOL_END, 0, 0, 0, 0)

    // 5. DMA Command to write tail_desc_addr in ipsec_cb
    DMA_COMMAND_PHV2MEM_FILL(dma_cmd_tail_desc_addr,  (ipsec_global.ipsec_cb_index * IPSEC_CB_SIZE) + IPSEC_CB_BASE + IPSEC_CB_TAIL_DESC_ADDR_OFFSET,  IPSEC_TAIL_DESC_ADDR_PHV_OFFSET_START, IPSEC_TAIL_DESC_ADDR_PHV_OFFSET_END, 0, 0, 0, 0)   


    modify_field(p4_rxdma_intr.dma_cmd_ptr, RXDMA_IPSEC_DMA_COMMANDS_OFFSET);
    // Ring Doorbell for IPSec-CB (svc_lif, type, ipsec-cb-index(qid))
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T0_S2S
}

//stage 3
action esp_v4_tunnel_n2h_update_output_desc_aol(addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    // pass output page in s2s data K+I - change it later
    modify_field(barco_desc_out.A0_addr, t1_s2s.out_page_addr);
    modify_field(barco_desc_out.O0, 0);
    modify_field(barco_desc_out.L0, 0); 

    modify_field(barco_desc_out.A1_addr, 0);
    modify_field(barco_desc_out.O1, 0);
    modify_field(barco_desc_out.L1, 0);

    modify_field(barco_desc_out.A2_addr, 0);
    modify_field(barco_desc_out.O2, 0);
    modify_field(barco_desc_out.L2, 0);
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T1_S2S

}

//stage 3
action esp_v4_tunnel_n2h_update_input_desc_aol (addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    // pass input page in s2s data K+I - change it later
    //modify_field(barco_desc_in.A0_addr, t0_s2s.in_page_addr);
    modify_field(barco_desc_in.O0, 0);
    //modify_field(barco_desc_in.L0, ipsec_global.frame_size - IPSEC_RXDMA_HW_SW_INTRINSIC_SIZE); 

    modify_field(barco_desc_in.A1_addr, 0);
    modify_field(barco_desc_in.O1, 0);
    modify_field(barco_desc_in.L1, 0);

    modify_field(barco_desc_in.A2_addr, 0);
    modify_field(barco_desc_in.O2, 0);
    modify_field(barco_desc_in.L2, 0);

    // Load tail-descriptor
    modify_field(p42p4plus_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0); 
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, ipsec_to_stage3.tail_desc_addr+64);
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T0_S2S
}

//stage 2 
action esp_v4_tunnel_n2h_allocate_output_page_index(out_page_index)
{
    modify_field(p42p4plus_hdr.table3_valid, 1);
    modify_field(common_te3_phv.table_pc, 0); 
    modify_field(common_te3_phv.table_raw_table_size, 3);
    modify_field(common_te3_phv.table_lock_en, 0);
    modify_field(common_te3_phv.table_addr, OUT_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * out_page_index));

    modify_field(t1_s2s.out_page_addr, OUT_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * out_page_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T3_S2S
}

//stage 2 
action esp_v4_tunnel_n2h_allocate_input_page_index(in_page_index)
{
    modify_field(p42p4plus_hdr.table2_valid, 1);
    modify_field(common_te2_phv.table_pc, 0); 
    modify_field(common_te2_phv.table_raw_table_size, 3);
    modify_field(common_te2_phv.table_lock_en, 0);
    modify_field(common_te2_phv.table_addr, IN_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * in_page_index));
  
    modify_field(t0_s2s.in_page_addr, IN_PAGE_ADDR_BASE+(PAGE_ENTRY_SIZE * in_page_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T2_S2S
}

//stage 2 
action esp_v4_tunnel_n2h_allocate_output_desc_index(out_desc_index)
{
    modify_field(p42p4plus_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0); 
    modify_field(common_te1_phv.table_raw_table_size, 3);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, OUT_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * out_desc_index));

    modify_field(t1_s2s.out_desc_addr, OUT_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * out_desc_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T1_S2S
}

//stage 2 
action esp_v4_tunnel_n2h_allocate_input_desc_index(in_desc_index)
{
    modify_field(p42p4plus_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0); 
    modify_field(common_te0_phv.table_raw_table_size, 3);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, IN_DESC_ADDR_BASE+(DESC_ENTRY_SIZE * in_desc_index));
    
    modify_field(t0_s2s.in_desc_addr, IN_DESC_ADDR_BASE++(DESC_ENTRY_SIZE* in_desc_index));
    IPSEC_SCRATCH_GLOBAL
    IPSEC_SCRATCH_T0_S2S
}

//stage 1
action esp_v4_tunnel_n2h_allocate_output_page_semaphore(out_page_ring_index)
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
action esp_v4_tunnel_n2h_allocate_input_page_semaphore(in_page_ring_index)
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
action esp_v4_tunnel_n2h_allocate_output_desc_semaphore(out_desc_ring_index)
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
action esp_v4_tunnel_n2h_allocate_input_desc_semaphore(in_desc_ring_index)
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
action esp_v4_tunnel_n2h_rxdma_initial_table(pc, rsvd, cosA, cosB,
                                       cos_sel, eval_last, host, total, pid,
                                       rxdma_ring_pindex, rxdma_ring_cindex, 
                                       barco_ring_pindex, barco_ring_cindex,
                                       key_index, iv_size, icv_size,
                                       expected_seq_no, last_replay_seq_no,
                                       replay_seq_no_bmp, barco_enc_cmd,
                                       ipsec_cb_index, block_size, 
                                       head_desc_addr, tail_desc_addr)
{

    //Set all variables to scratch so that they are not removed
    modify_field(ipsec_cb_scratch.pid, pid);
    modify_field(ipsec_cb_scratch.total, total);
    modify_field(ipsec_cb_scratch.host, host);
    modify_field(ipsec_cb_scratch.eval_last, eval_last);
    modify_field(ipsec_cb_scratch.cos_sel, cos_sel);
    modify_field(ipsec_cb_scratch.cosB, cosB);
    modify_field(ipsec_cb_scratch.cosA, cosA);
    modify_field(ipsec_cb_scratch.rsvd, rsvd);
    modify_field(ipsec_cb_scratch.pc, pc);

    modify_field(ipsec_cb_scratch.rxdma_ring_pindex, rxdma_ring_pindex);
    modify_field(ipsec_cb_scratch.rxdma_ring_cindex, rxdma_ring_cindex);
    modify_field(ipsec_cb_scratch.barco_ring_pindex, barco_ring_pindex);
    modify_field(ipsec_cb_scratch.barco_ring_cindex, barco_ring_cindex);
 
    modify_field(ipsec_cb_scratch.key_index, key_index);
    modify_field(ipsec_cb_scratch.iv_size, iv_size);
    modify_field(ipsec_cb_scratch.icv_size, icv_size);
    modify_field(ipsec_cb_scratch.last_replay_seq_no, last_replay_seq_no);
    modify_field(ipsec_cb_scratch.expected_seq_no, expected_seq_no);
    modify_field(ipsec_cb_scratch.replay_seq_no_bmp, replay_seq_no_bmp);
    modify_field(ipsec_cb_scratch.barco_enc_cmd,barco_enc_cmd);
    modify_field(ipsec_cb_scratch.block_size, block_size);
    modify_field(ipsec_cb_scratch.ipsec_cb_index, ipsec_cb_index);
    modify_field(ipsec_cb_scratch.head_desc_addr, head_desc_addr);
    modify_field(ipsec_cb_scratch.tail_desc_addr, tail_desc_addr);

    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);


    modify_field(ipsec_int_header.ipsec_cb_index, ipsec_cb_index);
    modify_field(ipsec_to_stage3.tail_desc_addr, tail_desc_addr);
 

    modify_field(ipsec_int_header.payload_start, p42p4plus_hdr.ipsec_payload_start+iv_size);
    modify_field(ipsec_int_header.tailroom_offset,  p42p4plus_hdr.ipsec_payload_end-icv_size-2);
    modify_field(ipsec_int_header.headroom, p42p4plus_hdr.ipsec_payload_start - p42p4plus_hdr.ip_hdr_size - 8 );
    modify_field(ipsec_int_header.headroom_offset, (p42p4plus_hdr.ipsec_payload_start - p42p4plus_hdr.ip_hdr_size - 8));
    modify_field(ipsec_int_header.payload_size, (p42p4plus_hdr.ipsec_payload_end-icv_size-2) - (p42p4plus_hdr.ipsec_payload_start+iv_size));
   
    modify_field(p42p4plus_hdr_scratch.seqNo,  p42p4plus_hdr.seqNo);

    //Sequence number checking logic

    // Lif, type, qid
    modify_field(ipsec_global.lif, p4_intr_global.lif);
    modify_field(ipsec_global.qtype, p4_rxdma_intr.qtype);
    modify_field(ipsec_global.qid, p4_rxdma_intr.qid);

  
    modify_field(p42p4plus_scratch_hdr.app_type, p42p4plus_hdr.app_type);
    modify_field(p42p4plus_scratch_hdr.table0_valid, p42p4plus_hdr.table0_valid);
    modify_field(p42p4plus_scratch_hdr.table1_valid, p42p4plus_hdr.table1_valid);
    modify_field(p42p4plus_scratch_hdr.table2_valid, p42p4plus_hdr.table2_valid);
    modify_field(p42p4plus_scratch_hdr.table3_valid, p42p4plus_hdr.table3_valid);

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
