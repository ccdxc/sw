#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action esp_v4_tunnel_n2h_txdma1_initial_table 

#define tx_table_s1_t0_action esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex
#define tx_table_s1_t1_action esp_v4_tunnel_n2h_allocate_barco_req_pindex


#define tx_table_s2_t0_action esp_v4_tunnel_n2h_txdma1_get_barco_req_index_ptr 
#define tx_table_s2_t1_action esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header
#define tx_table_s2_t2_action esp_v4_tunnel_n2h_txdma1_dequeue_head_desc 

#define tx_table_s3_t0_action esp_v4_tunnel_n2h_txdma1_write_barco_req

#define tx_table_s4_t0_action esp_v4_tunnel_n2h_txdma1_update_cb


#include "../../common-p4+/common_txdma.p4"

#include "../ipsec_defines.h"

#include "esp_v4_tunnel_n2h_headers.p4"



header_type p4plus_to_p4_ipsec_header_t {
    fields {
        app_type : 4;
        table0_valid : 1;
        table1_valid : 1;
        table2_valid : 1;
        table3_valid : 1;
        ipsec_pad : 256;
        ipsec_pad1 : 248;
    }
}


header_type ipsec_txdma1_global_t {
    fields {
        ipsec_cb_addr  : ADDRESS_WIDTH;
        in_desc_addr   : ADDRESS_WIDTH;
    }
}

header_type ipsec_table0_s2s {
    fields {
        in_desc_addr : ADDRESS_WIDTH;
        in_page_addr : ADDRESS_WIDTH;
        tailroom_offset : 16;
        icv_size     : 8;
        s2s0_pad     : 8;
    }
}

header_type ipsec_table1_s2s {
    fields {
        out_desc_addr : ADDRESS_WIDTH;
        out_page_addr : ADDRESS_WIDTH;
        s2s1_pad     : 32;
    }
}

header_type ipsec_table2_s2s {
    fields {
        in_desc_addr : ADDRESS_WIDTH;
        in_page_addr : ADDRESS_WIDTH;
        s2s2_pad     : 32;
    }
}

header_type ipsec_table3_s2s {
    fields {
        out_desc_addr : ADDRESS_WIDTH;
        out_page_addr : ADDRESS_WIDTH;
        s2s3_pad     : 32;
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
        barco_req_addr   : ADDRESS_WIDTH;
        stage3_pad1     : ADDRESS_WIDTH;
    }
}

header_type doorbell_data_pad_t {                                                                                                                                                                                
    fields {
        db_data_pad : 64;
    }
}


@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_ipsec_header_t p4plus2p4_hdr;

//unionize
@pragma pa_header_union ingress common_global 
metadata ipsec_txdma1_global_t txdma1_global;

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
metadata barco_descriptor_t barco_desc;
@pragma dont_trim
metadata barco_request_t barco_req;
@pragma dont_trim
metadata doorbell_data_t db_data;
@pragma dont_trim
metadata doorbell_data_pad_t db_data_pad;


@pragma dont_trim
metadata dma_cmd_phv2mem_t brq_req_write;

@pragma dont_trim
metadata dma_cmd_phv2mem_t ipsec_int_update_pad_l4_proto;

@pragma dont_trim
metadata dma_cmd_phv2mem_t doorbell_cmd; 


@pragma scratch_metadata
metadata ipsec_txdma1_global_t txdma1_global_scratch;

@pragma scratch_metadata
metadata ipsec_to_stage1_t scratch_to_s1;
@pragma scratch_metadata
metadata ipsec_to_stage2_t scratch_to_s2;
@pragma scratch_metadata
metadata ipsec_to_stage3_t scratch_to_s3;

@pragma scratch_metadata
metadata ipsec_table0_s2s scratch_t0_s2s;
@pragma scratch_metadata
metadata ipsec_table1_s2s scratch_t1_s2s;
@pragma scratch_metadata
metadata ipsec_table2_s2s scratch_t2_s2s;
@pragma scratch_metadata
metadata ipsec_table3_s2s scratch_t3_s2s;

@pragma scratch_metadata
metadata ipsec_cb_metadata_t ipsec_cb_scratch;

#define IPSEC_TXDMA1_GLOBAL_SCRATCH_INIT \
    modify_field(txdma1_global_scratch.ipsec_cb_index, txdma1_global.ipsec_cb_index); \
    modify_field(txdma1_global_scratch.in_desc_addr, txdma1_global.in_desc_addr); \
    modify_field(txdma1_global_scratch.ipsec_global_pad, txdma1_global.ipsec_global_pad); 

#define IPSEC_TXDMA1_S2S0_SCRATCH_INIT \
    modify_field(scratch_t0_s2s.in_desc_addr, t0_s2s.in_desc_addr); \
    modify_field(scratch_t0_s2s.in_page_addr, t0_s2s.in_page_addr); \
    modify_field(scratch_t0_s2s.s2s0_pad, t0_s2s.s2s0_pad);

#define IPSEC_TXDMA1_S2S1_SCRATCH_INIT \
    modify_field(scratch_t1_s2s.out_desc_addr, t1_s2s.out_desc_addr); \
    modify_field(scratch_t1_s2s.out_page_addr, t1_s2s.out_page_addr); \
    modify_field(scratch_t1_s2s.s2s1_pad, t1_s2s.s2s1_pad);

#define IPSEC_TXDMA1_S2S2_SCRATCH_INIT \
    modify_field(scratch_t2_s2s.in_desc_addr, t2_s2s.in_desc_addr); \
    modify_field(scratch_t2_s2s.in_page_addr, t2_s2s.in_page_addr); \
    modify_field(scratch_t2_s2s.s2s2_pad, t2_s2s.s2s2_pad);

#define IPSEC_TXDMA1_S2S3_SCRATCH_INIT \
    modify_field(scratch_t3_s2s.out_desc_addr, t3_s2s.out_desc_addr); \
    modify_field(scratch_t3_s2s.out_page_addr, t3_s2s.out_page_addr); \
    modify_field(scratch_t3_s2s.s2s0_pad, t3_s2s.s2s3_pad);

#define IPSEC_TXDMA1_TO_STAGE1_INIT \
    modify_field(scratch_to_s1.head_desc_addr, ipsec_to_stage1.head_desc_addr); \
    modify_field(scratch_to_s1.stage1_pad, ipsec_to_stage1.stage1_pad);

#define IPSEC_TXDMA1_TO_STAGE2_INIT \
    modify_field(scratch_to_s2.barco_req_addr, ipsec_to_stage2.barco_req_addr); \
    modify_field(scratch_to_s2.stage2_pad, ipsec_to_stage2.stage2_pad);

#define IPSEC_TXDMA1_TO_STAGE3_INIT \
    modify_field(scratch_to_s3.barco_req_addr, ipsec_to_stage2.barco_req_addr); \
    modify_field(scratch_to_s3.stage3_pad1, ipsec_to_stage2.stage3_pad1);


//stage 4 table 0
action esp_v4_tunnel_n2h_txdma1_update_cb(pc, rsvd, cosA, cosB,
                                       cos_sel, eval_last, host, total, pid,
                                       rxdma_ring_pindex, rxdma_ring_cindex,
                                       barco_ring_pindex, barco_ring_cindex,
                                       key_index, iv_size, icv_size,
                                       expected_seq_no, last_replay_seq_no,
                                       replay_seq_no_bmp, barco_enc_cmd,
                                       ipsec_cb_index, block_size,
                                       cb_pindex, cb_cindex, cb_ring_base_addr, ipsec_cb_pad)
{
    modify_field(p4plus2p4_hdr.table0_valid, 0);
    IPSEC_CB_SCRATCH
    //tblwr cb_cindex , rxdma_ring_cindex 
}

//stage 3 table 0
action esp_v4_tunnel_n2h_txdma1_write_barco_req(l4_protocol, pad_size)
{

    DMA_COMMAND_PHV2MEM_FILL(brq_req_write, ipsec_to_stage3.barco_req_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END, 0, 0, 0, 0) 
    modify_field(p4_txdma_intr.dma_cmd_ptr, TXDMA1_DMA_COMMANDS_OFFSET);
   
    //table writes for these 2 bytes here
    // should this be made DMA by doing phv2mem ?? Anyway these are needed only by txdma2. 
    modify_field(ipsec_int_header.l4_protocol, l4_protocol);
    modify_field(ipsec_int_header.pad_size, pad_size);

    // RING Barco-doorbell
    
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, txdma1_global.ipsec_cb_addr);
}

//stage 2 table 0
action esp_v4_tunnel_n2h_txdma1_get_barco_req_index_ptr(barco_req_index_address)
{
    modify_field(ipsec_to_stage3.barco_req_addr, barco_req_index_address);

    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 1);
    modify_field(common_te0_phv.table_lock_en, 0);
    // read 2 bytes at tailroom-offset so that we know l4proto and pad_size 
    // and update ipsec_int_header with that(which is in descriptor beginning)
    modify_field(common_te0_phv.table_addr, t0_s2s.in_page_addr+t0_s2s.tailroom_offset);
    modify_field(p4plus2p4_hdr.table2_valid, 0);
}


//stage 2 - table1
action esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header(in_desc, out_desc,
                                                   ipsec_cb_index, headroom, 
                                                   tailroom, headroom_offset,
                                                   tailroom_offset,
                                                   payload_start, buf_size,
                                                   payload_size, l4_protocol, pad_size)
{
    modify_field(barco_req.brq_in_addr, in_desc);
    modify_field(barco_req.brq_out_addr, out_desc);
    modify_field(barco_req.brq_auth_tag_addr, out_desc+tailroom_offset+pad_size+2);
    modify_field(barco_req.brq_hdr_size, payload_start);
    modify_field(t0_s2s.tailroom_offset, tailroom_offset);
}

//stage 2 - table2
action esp_v4_tunnel_n2h_txdma1_dequeue_head_desc(addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{

    modify_field(p4plus2p4_hdr.table1_valid, 0);
    modify_field(t0_s2s.in_page_addr, addr0);
     
}

//stage 1 - table1
action esp_v4_tunnel_n2h_allocate_barco_req_pindex (barco_pindex)
{
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, BRQ_REQ_RING_BASE_ADDR + (BRQ_REQ_RING_ENTRY_SIZE * barco_pindex));
    modify_field(p4plus2p4_hdr.table2_valid, 0);
}

//stage 1 - table0
action esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex(in_desc_addr)
{
    modify_field(p4plus2p4_hdr.table2_valid, 1);
    modify_field(common_te2_phv.table_pc, 0);
    modify_field(common_te2_phv.table_raw_table_size, 6);
    modify_field(common_te2_phv.table_lock_en, 0);
    modify_field(common_te2_phv.table_addr, in_desc_addr+64);
   
    modify_field(p4plus2p4_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0);
    modify_field(common_te1_phv.table_raw_table_size, 6);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, in_desc_addr);

}


//stage 0
action esp_v4_tunnel_n2h_txdma1_initial_table(pc, rsvd, cosA, cosB,
                                       cos_sel, eval_last, host, total, pid,
                                       rxdma_ring_pindex, rxdma_ring_cindex,
                                       barco_ring_pindex, barco_ring_cindex,
                                       key_index, iv_size, icv_size,
                                       expected_seq_no, last_replay_seq_no,
                                       replay_seq_no_bmp, barco_enc_cmd,
                                       ipsec_cb_index, block_size,
                                       cb_pindex, cb_cindex, cb_ring_base_addr, ipsec_cb_pad)
{
    //IPSEC_CB_SCRATCH 

    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    modify_field(txdma1_global.ipsec_cb_addr, IPSEC_CB_BASE + (IPSEC_CB_SIZE * ipsec_cb_index));

    modify_field(barco_req.brq_barco_enc_cmd, barco_enc_cmd);
    modify_field(barco_req.brq_iv_addr, IPSEC_CB_BASE + (IPSEC_CB_SIZE * ipsec_cb_index) + IPSEC_CB_IV_OFFSET);
    modify_field(barco_req.brq_key_index, key_index);

    modify_field(t0_s2s.icv_size, icv_size);
 
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, cb_ring_base_addr+cb_cindex*8);
    
    modify_field(p4plus2p4_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0);
    modify_field(common_te1_phv.table_raw_table_size, 2);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, BRQ_REQ_SEMAPHORE_ADDR);
}
