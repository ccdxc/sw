#include "../../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0_action ipsec_encap_txdma_initial_table 

#define tx_table_s1_t0_action ipsec_stage1_dummy 
#define tx_table_s1_t1_action ipsec_stage1_drops

#define tx_table_s2_t0_action ipsec_get_in_desc_from_cb_cindex 

#define tx_table_s3_t0_action ipsec_encap_txdma_load_head_desc_int_header 
#define tx_table_s3_t1_action ipsec_encap_txdma_load_head_desc_int_header2 
#define tx_table_s3_t2_action allocate_barco_req_pindex 

#define tx_table_s4_t0_action ipsec_write_barco_req

#define tx_table_s5_t0_action ipsec_release_resources

#include "../../common-p4+/common_txdma.p4"
#include "esp_ipv4_tunnel_h2n_headers.p4"
#include "../ipsec_defines.h"


header_type ipsec_txdma1_global_t {
    fields {
        in_desc_addr   : ADDRESS_WIDTH;
        ipsec_cb_addr  : 40;
        flags          : 8;
        pad            : 16;
    }
}

header_type ipsec_table0_s2s {
    fields {
        in_desc_addr : ADDRESS_WIDTH;
        in_page_addr : ADDRESS_WIDTH;
        iv_size : 8;
        s2s0_pad     : 24;
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
        cb_ring_slot_addr : ADDRESS_WIDTH;
        stage1_pad    : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage2_t {
    fields {
        barco_cb_ring_base_addr : ADDRESS_WIDTH;
        barco_pindex : 16;
        ipsec_cb_index : 16;
        stage2_pad2    : 32;
        
    }
}

header_type ipsec_to_stage3_t {
    fields {
        barco_req_addr   : ADDRESS_WIDTH;
        stage3_pad     : ADDRESS_WIDTH;
    }
}

header_type ipsec_to_stage4_t {
    fields {
        barco_req_addr   : ADDRESS_WIDTH;
        barco_pindex : 16;
        sem_cindex   : 32;
        cb_cindex    : 16;
    }
}
header_type ipsec_to_stage5_t {
    fields {
        in_desc_addr : ADDRESS_WIDTH;
        out_desc_addr : ADDRESS_WIDTH;
    }
}

header_type doorbell_data_pad_t {
    fields {
        db_data_pad : 96;
    }
}

header_type barco_dbell_t {                                                                                                                                                     
    fields {
        pi : 32;
    }
}


@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t p4plus2p4_hdr;

//unionize
@pragma pa_header_union ingress common_global 
metadata ipsec_txdma1_global_t txdma1_global;

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
metadata barco_dbell_t barco_dbell; 
@pragma dont_trim
metadata doorbell_data_pad_t db_pad;
 
@pragma dont_trim
metadata barco_request_t barco_req;
@pragma dont_trim
metadata barco_zero_content_t barco_zero;

@pragma dont_trim
metadata doorbell_data_t db_data;

@pragma dont_trim
metadata dma_cmd_phv2mem_t brq_in_desc_zero;
@pragma dont_trim
metadata dma_cmd_phv2mem_t brq_out_desc_zero;

@pragma dont_trim
metadata dma_cmd_phv2mem_t brq_req_write;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_post_barco_ring;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_incr_pindex;

@pragma dont_trim
metadata dma_cmd_phv2mem_t rnmdr;
@pragma dont_trim
metadata dma_cmd_phv2mem_t tnmdr;
@pragma dont_trim
metadata dma_cmd_phv2mem_t doorbell_cmd;
@pragma dont_trim
metadata dma_cmd_phv2mem_t sem_cindex;


@pragma scratch_metadata
metadata ipsec_txdma1_global_t txdma1_global_scratch;

@pragma scratch_metadata
metadata ipsec_to_stage4_t ipsec_to_stage4_scratch;

@pragma scratch_metadata
metadata ipsec_to_stage5_t ipsec_to_stage5_scratch;

@pragma scratch_metadata
metadata ipsec_to_stage1_t scratch_to_s1;
@pragma scratch_metadata
metadata ipsec_to_stage2_t scratch_to_s2;
@pragma scratch_metadata
metadata ipsec_to_stage3_t scratch_to_s3;
@pragma scratch_metadata
metadata ipsec_to_stage4_t scratch_to_s4;

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

@pragma scratch_metadata
metadata ipsec_int_header_t ipsec_int_hdr_scratch;

@pragma scratch_metadata
metadata barco_shadow_params_d_t barco_shadow_params_d;

@pragma scratch_metadata
metadata h2n_stats_header_t ipsec_stats_scratch;


#define IPSEC_TXDMA1_GLOBAL_SCRATCH_INIT \
    modify_field(txdma1_global_scratch.ipsec_cb_addr, txdma1_global.ipsec_cb_addr); \
    modify_field(txdma1_global_scratch.in_desc_addr, txdma1_global.in_desc_addr); \
    modify_field(txdma1_global_scratch.flags, txdma1_global.flags); \
    modify_field(txdma1_global_scratch.pad, txdma1_global.pad); \

#define IPSEC_TXDMA1_S2S0_SCRATCH_INIT \
    modify_field(scratch_t0_s2s.in_desc_addr, t0_s2s.in_desc_addr); \
    modify_field(scratch_t0_s2s.in_page_addr, t0_s2s.in_page_addr); \
    modify_field(scratch_t0_s2s.iv_size, t0_s2s.iv_size); \
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
    modify_field(scratch_to_s1.cb_ring_slot_addr, ipsec_to_stage1.cb_ring_slot_addr); \
    modify_field(scratch_to_s1.stage1_pad, ipsec_to_stage1.stage1_pad);

#define IPSEC_TXDMA1_TO_STAGE2_INIT \
    modify_field(scratch_to_s2.stage2_pad, ipsec_to_stage2.stage2_pad);

#define IPSEC_TXDMA1_TO_STAGE3_INIT \
    modify_field(scratch_to_s3.barco_req_addr, ipsec_to_stage3.barco_req_addr); \
    modify_field(scratch_to_s3.stage3_pad1, ipsec_to_stage3.stage3_pad1);

#define IPSEC_TXDMA1_TO_STAGE4_INIT \
    modify_field(scratch_to_s4.barco_req_addr, ipsec_to_stage4.barco_req_addr); \
    modify_field(scratch_to_s4.barco_pindex, ipsec_to_stage4.barco_pindex); \
    modify_field(scratch_to_s4.sem_cindex, ipsec_to_stage4.sem_cindex); \
    modify_field(scratch_to_s4.cb_cindex, ipsec_to_stage4.cb_cindex);

//stage 5
action ipsec_release_resources(sem_cindex)
{
    IPSEC_TXDMA1_GLOBAL_SCRATCH_INIT
     modify_field(ipsec_to_stage5_scratch.in_desc_addr, ipsec_to_stage5.in_desc_addr);
     modify_field(ipsec_to_stage5_scratch.out_desc_addr, ipsec_to_stage5.out_desc_addr);
     modify_field(ipsec_to_stage4_scratch.sem_cindex, sem_cindex);
}

//stage 4 table 0
action ipsec_write_barco_req(pc, rsvd, cosA, cosB, cos_sel,
                             eval_last, host, total, pid,
                             rxdma_ring_pindex, rxdma_ring_cindex,
                             barco_ring_pindex, barco_ring_cindex,
                             key_index, iv_size, icv_size, spi,
                             esn_lo, iv, barco_enc_cmd,
                             ipsec_cb_index,
                             cb_pindex, cb_cindex, barco_pindex, barco_cindex, 
                             cb_ring_base_addr_hi, cb_ring_base_addr,
                             barco_ring_base_addr_hi, barco_ring_base_addr, 
                             iv_salt, barco_full_count, flags)
{
    IPSEC_TXDMA1_GLOBAL_SCRATCH_INIT
    IPSEC_CB_SCRATCH_WITH_PC
    // memwr to increment hw-cindex (cb_base + offset)
    IPSEC_TXDMA1_S2S0_SCRATCH_INIT
    IPSEC_TXDMA1_TO_STAGE4_INIT
    DMA_COMMAND_PHV2MEM_FILL(brq_req_write, ipsec_to_stage4.barco_req_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END, 0, 0, 0, 0) 
    modify_field(p4_txdma_intr.dma_cmd_ptr, TXDMA1_DMA_COMMANDS_OFFSET);
}

//stage 2 table 1
action ipsec_get_barco_req_index_ptr(barco_req_index_address)
{
    IPSEC_TXDMA1_GLOBAL_SCRATCH_INIT
    modify_field(ipsec_to_stage3.barco_req_addr, barco_req_index_address);
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, txdma1_global.ipsec_cb_addr);
    modify_field(p4plus2p4_hdr.table2_valid, 0);
}

//stage 3 - table1
action ipsec_encap_txdma_load_head_desc_int_header2(in_desc, out_desc, in_page, out_page,
                                                   ipsec_cb_index, headroom, 
                                                   tailroom, headroom_offset,
                                                   tailroom_offset, payload_start,
                                                   buf_size, payload_size, pad_size, l4_protocol)
{
    IPSEC_INT_HDR_SCRATCH
    IPSEC_TXDMA1_S2S1_SCRATCH_INIT
    modify_field(barco_req.input_list_address, in_desc);
    modify_field(barco_req.output_list_address, out_desc);
    modify_field(barco_req.iv_address, in_page);

    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 3);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, txdma1_global.ipsec_cb_addr);

    modify_field(scratch_to_s3.barco_req_addr, ipsec_to_stage3.barco_req_addr);
}

//stage 2 - table0
action ipsec_encap_txdma_load_head_desc_int_header(in_desc, out_desc, in_page, out_page,
                                                   ipsec_cb_index, headroom, 
                                                   tailroom, headroom_offset,
                                                   tailroom_offset, payload_start,
                                                   buf_size, payload_size, pad_size, l4_protocol)
{
    IPSEC_INT_HDR_SCRATCH
    IPSEC_TXDMA1_S2S0_SCRATCH_INIT
    modify_field(barco_req.input_list_address, in_desc);
    modify_field(barco_req.output_list_address, out_desc);
    //modify_field(barco_req.auth_tag_addr, out_page+tailroom_offset+pad_size+2);
   // modify_field(barco_req.header_size, payload_start);
    modify_field(barco_req.iv_address, in_page);

    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 3);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, txdma1_global.ipsec_cb_addr);

    modify_field(scratch_to_s3.barco_req_addr, ipsec_to_stage3.barco_req_addr);
}

//stage 2 - table1
action allocate_barco_req_pindex (BARCO_SHADOW_PARAMS)
{
    modify_field(p4plus2p4_hdr.table1_valid, 1);
    modify_field(p4plus2p4_hdr.table2_valid, 0);
    GENERATE_BARCO_SHADOW_PARAMS_D
}

//stage 2 - table0
action ipsec_get_in_desc_from_cb_cindex(in_desc_addr)
{
    modify_field(txdma1_global.in_desc_addr, in_desc_addr);

    modify_field(scratch_to_s2.barco_cb_ring_base_addr, ipsec_to_stage2.barco_cb_ring_base_addr);
    modify_field(scratch_to_s2.barco_pindex, ipsec_to_stage2.barco_pindex);
    modify_field(scratch_to_s2.ipsec_cb_index, ipsec_to_stage2.ipsec_cb_index);
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, in_desc_addr);
}
//stage 1
action ipsec_stage1_dummy()
{
    IPSEC_TXDMA1_GLOBAL_SCRATCH_INIT
    IPSEC_TXDMA1_S2S0_SCRATCH_INIT
    IPSEC_TXDMA1_TO_STAGE1_INIT
}

//stage 1
action ipsec_stage1_drops(H2N_STATS_UPDATE_PARAMS)
{
    H2N_STATS_UPDATE_SET
}

//stage 0
action ipsec_encap_txdma_initial_table(rsvd, cosA, cosB, cos_sel,
                                       eval_last, host, total, pid,
                                       rxdma_ring_pindex, rxdma_ring_cindex,
                                       barco_ring_pindex, barco_ring_cindex,
                                       key_index, iv_size, icv_size, spi,
                                       esn_lo, iv, barco_enc_cmd,
                                       ipsec_cb_index,
                                       cb_pindex, cb_cindex, barco_pindex, barco_cindex, 
                                       cb_ring_base_addr_hi, cb_ring_base_addr,
                                       barco_ring_base_addr_hi,  barco_ring_base_addr,
                                       iv_salt, barco_full_count, flags)
{
    IPSEC_CB_SCRATCH
  
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    modify_field(barco_req.command, barco_enc_cmd);
    //modify_field(barco_req.brq_iv_addr, IPSEC_CB_BASE + (IPSEC_CB_SIZE * ipsec_cb_index) + IPSEC_CB_IV_OFFSET);
    //modify_field(barco_req.key_desc_index, key_index);
    modify_field(t0_s2s.iv_size, iv_size);
 
    modify_field(p4plus2p4_hdr.table1_valid, 1);
    modify_field(common_te0_phv.table_pc, 0); 
    modify_field(common_te0_phv.table_raw_table_size, 3);
    modify_field(common_te0_phv.table_lock_en, 0);
    //modify_field(common_te0_phv.table_addr, cb_ring_base_addr+cb_cindex*64);

    modify_field(p4plus2p4_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0);
    modify_field(common_te1_phv.table_raw_table_size, 2);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, BRQ_REQ_SEMAPHORE_ADDR);
}
