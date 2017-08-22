#define tx_table_s0_t0_action ipsec_encap_txdma_initial_table 
#define tx_table_s0_t1_action ipsec_txdma_dummy 
#define tx_table_s0_t2_action ipsec_txdma_dummy 
#define tx_table_s0_t3_action ipsec_txdma_dummy 

#define tx_table_s1_t0_action ipsec_encap_txdma_load_head_desc_int_header
#define tx_table_s1_t1_action ipsec_encap_txdma_deque_head_desc 
#define tx_table_s1_t2_action allocate_barco_req_pindex 
#define tx_table_s1_t3_action ipsec_txdma_dummy 

#define tx_table_s2_t0_action ipsec_get_barco_req_index_ptr 
#define tx_table_s2_t1_action ipsec_txdma_dummy
#define tx_table_s2_t2_action ipsec_txdma_dummy
#define tx_table_s2_t3_action ipsec_txdma_dummy

#define tx_table_s3_t0_action ipsec_write_barco_req
#define tx_table_s3_t1_action ipsec_txdma_dummy
#define tx_table_s3_t2_action ipsec_txdma_dummy
#define tx_table_s3_t3_action ipsec_txdma_dummy

#define tx_table_s4_t0_action ipsec_txdma_dummy
#define tx_table_s4_t1_action ipsec_txdma_dummy
#define tx_table_s4_t2_action ipsec_txdma_dummy
#define tx_table_s4_t3_action ipsec_txdma_dummy

#define tx_table_s5_t0_action ipsec_txdma_dummy
#define tx_table_s5_t1_action ipsec_txdma_dummy
#define tx_table_s5_t2_action ipsec_txdma_dummy
#define tx_table_s5_t3_action ipsec_txdma_dummy

#define tx_table_s6_t0_action ipsec_txdma_dummy
#define tx_table_s6_t1_action ipsec_txdma_dummy
#define tx_table_s6_t2_action ipsec_txdma_dummy
#define tx_table_s6_t3_action ipsec_txdma_dummy

#define tx_table_s7_t0_action ipsec_txdma_dummy
#define tx_table_s7_t1_action ipsec_txdma_dummy
#define tx_table_s7_t2_action ipsec_txdma_dummy
#define tx_table_s7_t3_action ipsec_txdma_dummy

#include "../../common-p4+/common_txdma.p4"

#include "../ipsec_defines.h"


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
        l4_protocol       : 8;
        pad_size          : 8;
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
        //scratch : 512

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


// The below will be filled up by TxDMA by doing a phv2mem and writing the request contents to the ring
// please refer to barco document - table1 for the latest format.
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
        s2s0_pad     : 32;
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
metadata dma_cmd_phv2mem_t brq_req_write;
@pragma dont_trim
metadata dma_cmd_phv2mem_t head_desc_addr_update;

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




action ipsec_txdma_dummy ()
{
}

//stage 3 table 0
action ipsec_write_barco_req(pid, cosb, cosa, pc_offset,
                             ipsec_cb_pindex, ipsec_cb_cindex,
                             barco_cb_pindex, barco_cb_cindex,
                             key_index, iv_size, icv_size,
                             spi, esn_lo, iv, esn_hi,
                             barco_enc_cmd, ipsec_cb_index,
                             block_size, head_desc_addr, tail_desc_addr)
{

    DMA_COMMAND_PHV2MEM_FILL(brq_req_write, ipsec_to_stage3.barco_req_addr, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START, IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END, 0, 0, 0, 0) 
    modify_field(p4_txdma_intr.dma_cmd_ptr, TXDMA1_DMA_COMMANDS_OFFSET);
}

//stage 2 table 0
action ipsec_get_barco_req_index_ptr(barco_req_index_address)
{
    modify_field(ipsec_to_stage3.barco_req_addr, barco_req_index_address);

    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, txdma1_global.ipsec_cb_addr);
    modify_field(p4plus2p4_hdr.table2_valid, 0);
}


//stage 1 - table0
action ipsec_encap_txdma_load_head_desc_int_header(in_desc, out_desc,
                                                   ipsec_cb_index, headroom, 
                                                   tailroom, headroom_offset,
                                                   tailroom_offset, pad_size,
                                                   payload_start, buf_size,
                                                   payload_size)
{
    modify_field(barco_req.brq_in_addr, in_desc);
    modify_field(barco_req.brq_out_addr, out_desc);
    modify_field(barco_req.brq_auth_tag_addr, out_desc+tailroom_offset+pad_size+2);
    modify_field(barco_req.brq_hdr_size, payload_start);

    //modify_field(txdma1_global.out_desc_addr, out_desc);
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 3);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, txdma1_global.ipsec_cb_addr);
}

//stage 1 - table1
action ipsec_encap_txdma_deque_head_desc (addr0, offset0, length0,
                              addr1, offset1, length1,
                              addr2, offset2, length2,
                              nextptr, rsvd)
{
    // DMA write to ipsec_cb's head_desc_addr = rsvd
    modify_field(ipsec_to_stage1.head_desc_addr, rsvd);
    DMA_COMMAND_PHV2MEM_FILL(head_desc_addr_update, txdma1_global.ipsec_cb_addr+IPSEC_CB_HEAD_DESC_ADDR_OFFSET, 
          IPSEC_TXDMA1_HEAD_DESC_PHV_OFFSET_START, IPSEC_TXDMA1_HEAD_DESC_PHV_OFFSET_END, 0, 0, 0, 0) 
    modify_field(p4plus2p4_hdr.table1_valid, 0);
     
}

//stage 1 - table2
action allocate_barco_req_pindex (barco_pindex)
{
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, BRQ_REQ_RING_BASE_ADDR + (BRQ_REQ_RING_ENTRY_SIZE * barco_pindex));
    modify_field(p4plus2p4_hdr.table2_valid, 0);
}


//stage 0
action ipsec_encap_txdma_initial_table(pid, cosb, cosa, pc_offset,
                                       ipsec_cb_pindex, ipsec_cb_cindex,
                                       barco_cb_pindex, barco_cb_cindex,
                                       key_index, iv_size, icv_size,
                                       spi, esn_lo, iv, esn_hi,
                                       barco_enc_cmd, ipsec_cb_index,
                                       block_size, head_desc_addr, tail_desc_addr)
{

    //IPSEC_TXDMA1_GLOBAL_SCRATCH_INIT
    //IPSEC_TXDMA1_S2S0_SCRATCH_INIT
   
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);
 

    modify_field(txdma1_global.ipsec_cb_addr, IPSEC_CB_BASE + (IPSEC_CB_SIZE * ipsec_cb_index));
    modify_field(txdma1_global.in_desc_addr, head_desc_addr);

    modify_field(barco_req.brq_barco_enc_cmd, barco_enc_cmd);
    modify_field(barco_req.brq_iv_addr, IPSEC_CB_BASE + (IPSEC_CB_SIZE * ipsec_cb_index) + IPSEC_CB_IV_OFFSET);
    modify_field(barco_req.brq_key_index, key_index);

 
    modify_field(p4plus2p4_hdr.table0_valid, 1);
    modify_field(common_te0_phv.table_pc, 0);
    modify_field(common_te0_phv.table_raw_table_size, 6);
    modify_field(common_te0_phv.table_lock_en, 0);
    modify_field(common_te0_phv.table_addr, head_desc_addr+64);
   
    modify_field(p4plus2p4_hdr.table1_valid, 1);
    modify_field(common_te1_phv.table_pc, 0);
    modify_field(common_te1_phv.table_raw_table_size, 6);
    modify_field(common_te1_phv.table_lock_en, 0);
    modify_field(common_te1_phv.table_addr, head_desc_addr);
    
    modify_field(p4plus2p4_hdr.table1_valid, 1);
    modify_field(common_te2_phv.table_pc, 0);
    modify_field(common_te2_phv.table_raw_table_size, 2);
    modify_field(common_te2_phv.table_lock_en, 0);
    modify_field(common_te2_phv.table_addr, BRQ_REQ_SEMAPHORE_ADDR);
}

