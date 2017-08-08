/*****************************************************************************
 * capri.h: Definitions for intrinsic fields in P4+ pipeline of Capri
 *****************************************************************************/
 
#ifndef CAPRI_H
#define CAPRI_H

// Intrinsic field definitions in Capri PHV
header_type capri_intrinsic_t {
  fields {
    table_mpu_entry_raw  : 32;
    table_addr_raw       : 64;
    lif                  : 11;
    bypass               : 1;
    debug_trace          : 1;
    drop                 : 1;
    hw_error             : 1;
    error_bits           : 4;
    frame_size           : 14;
    no_data              : 1;
    timestamp            : 48;
    tm_span_session      : 8;
    tm_iport             : 3;
    tm_oport             : 8;
    tm_oq                : 5;
    tm_iq                : 5;
    tm_replicate_en_rep  : 1;
    tm_replicate_ptr_id  : 12;
    tm_q_depth           : 16;
    recirc               : 1;
    recirc_count         : 3;
    table_sel            : 5;
    table_size_raw       : 7;
    table_key_phv        : 8;
    qid                  : 24;
    rx_splitter_offset   : 10;
    dma_cmd_ptr          : 8;
    dma_cmd              : 128;
    pad                  : 82;
  }
}

// Phv2Mem DMA command format
// TODO: Check the ordering of fields (big endian vs little endian)
header_type capri_phv2mem_dma_t {
  fields {
    rsvd                 : 37;
    override_lif         : 11;
    use_override_lif     : 1;
    phv_end              : 10;
    phv_start            : 10;
    wr_fence             : 1;
    addr                 : 52;
    cache                : 1;
    host_addr            : 1;
    cmdeop               : 1;  
    cmdtype              : 3;
  }
}

// Mem2Mem DMA command format
// TODO: Check the ordering of fields (big endian vs little endian)
header_type capri_mem2mem_dma_t {
  fields {
    rsvd                 : 21;
    override_lif         : 11;
    use_override_lif     : 1;
    phv_end              : 10;
    phv_start            : 10;
    wr_fence             : 1;
    data_size            : 14;
    addr                 : 52;
    cache                : 1;
    host_addr            : 1;
    mem2mem_type         : 2;
    cmdeop               : 1;  
    cmdtype              : 3;
  }
}

#define CAPRI_INTRINSIC_INIT(i)			\
  modify_field(i.lif, 0);			\
  modify_field(i.bypass, 0);			\
  modify_field(i.debug_trace, 0);		\
  modify_field(i.drop, 0);			\
  modify_field(i.hw_error, 0);			\
  modify_field(i.error_bits, 0);		\
  modify_field(i.frame_size, 0);		\
  modify_field(i.no_data, 0);			\
  modify_field(i.timestamp, 0);			\
  modify_field(i.tm_span_session, 0);		\
  modify_field(i.tm_iport, 0);			\
  modify_field(i.tm_oport, 0);			\
  modify_field(i.tm_oq, 0);			\
  modify_field(i.tm_iq, 0);			\
  modify_field(i.tm_replicate_en_rep, 0);	\
  modify_field(i.tm_replicate_ptr_id, 0);	\
  modify_field(i.tm_q_depth, 0);		\
  modify_field(i.recirc, 0);			\
  modify_field(i.recirc_count, 0);		\
  modify_field(i.table_sel, 0);			\
  modify_field(i.table_addr_raw, 0);		\
  modify_field(i.table_size_raw, 0);		\
  modify_field(i.table_key_phv, 0);		\
  modify_field(i.table_mpu_entry_raw, 0);	\
  modify_field(i.qid, 0);			\
  modify_field(i.rx_splitter_offset, 0);	\
  modify_field(i.dma_cmd_ptr, 0);		\
  modify_field(i.dma_cmd, 0);			\
  modify_field(i.pad, 0);			\

#define CAPRI_PHV2MEM_DMA_INIT(d)		\
  modify_field(d.rsvd, 0);			\
  modify_field(d.override_lif, 0);		\
  modify_field(d.use_override_lif, 0);		\
  modify_field(d.phv_end, 0);			\
  modify_field(d.phv_start, 0);			\
  modify_field(d.wr_fence, 0);			\
  modify_field(d.addr, 0);			\
  modify_field(d.cache, 0);			\
  modify_field(d.host_addr, 0);			\
  modify_field(d.cmdeop, 0);			\
  modify_field(d.cmdtype, 0);			\

#define CAPRI_MEM2MEM_DMA_INIT(d)		\
  modify_field(d.rsvd, 0);			\
  modify_field(d.override_lif, 0);		\
  modify_field(d.use_override_lif, 0);		\
  modify_field(d.phv_end, 0);			\
  modify_field(d.phv_start, 0);			\
  modify_field(d.wr_fence, 0);			\
  modify_field(d.data_size, 0);			\
  modify_field(d.addr, 0);			\
  modify_field(d.cache, 0);			\
  modify_field(d.host_addr, 0);			\
  modify_field(d.mem2mem_type, 0);		\
  modify_field(d.cmdeop, 0);			\
  modify_field(d.cmdtype, 0);			\

// Load a table based on fixed base, index and fixed size
// Size * index is used to determine offset beyond table base
#define CAPRI_LOAD_TABLE_IDX(i, _table_base, _idx,			\
                             _table_size, _load_size, _stage_entry)	\
  modify_field(i.table_pc, _stage_entry);				\
  modify_field(i.table_addr, _table_base + (_idx * _table_size));	\
  modify_field(i.table_raw_table_size, _table_size);			\

// Load a table based on calculated base, fixed offset and fixed size
// Offset is used as is beyond table base
#define CAPRI_LOAD_TABLE_OFFSET(i, _table_base, _table_offset,		\
                                _load_size, _stage_entry)		\
  modify_field(i.table_pc, _stage_entry);				\
  modify_field(i.table_addr, _table_base + _table_offset);		\
  modify_field(i.table_raw_table_size, _load_size);			\

// Load no table, but call an action handler
#define CAPRI_LOAD_NO_TABLE(i, _stage_entry)				\
  modify_field(i.table_pc, _stage_entry);				\
  modify_field(i.table_addr, 0);					\
  modify_field(i.table_raw_table_size, 0);				\

// ASM Function pointers
#define nvme_be_sq_ctx_check			0x00800000
#define nvme_be_sq_skip_stage			0x00801000
#define nvme_be_sq_ssd_tbl_addr_load		0x00802000
#define nvme_be_sq_pri_wrr			0x00803000
#define nvme_be_sq_entry_copy 			0x00804000
#define nvme_be_sq_entry_pop 			0x00805000
#define ssd_cmd_save 				0x00806000
#define ssd_sq_entry_push 			0x00807000

#define ssd_cq_ctx_check			0x00810000
#define ssd_cq_entry_pop			0x00811000
#define ssd_saved_cmd_handle			0x00812000
#define ssd_info_update				0x00813000
#define ssd_saved_cmd_skip_stage		0x00814000
#define ssd_saved_cmd_tbl_addr_load		0x00815000
#define ssd_saved_cmd_release			0x00816000
#define nvme_be_cq_entry_push			0x00817000

#define nvme_vf_sq_ctx_check			0x00820000
#define nvme_vf_sq_entry_pop			0x00821000
#define pvm_vf_sq_entry_push			0x00822000

#define pvm_vf_cq_ctx_check			0x00830000
#define pvm_vf_cq_entry_pop			0x00831000
#define nvme_vf_cq_entry_push			0x00832000

#define r2n_cq_ctx_check			0x00840000
#define r2n_cq_entry_pop			0x00841000
#define r2n_nvme_cmd_handle			0x00842000
#define nvme_be_sq_entry_push			0x00843000
#define pvm_errq_entry_push			0x00844000
#define r2n_cq_rx_buf_get			0x00845000
// r2n_rq_entry_push defined subsequently

#define r2n_hq_ctx_check			0x00850000
#define r2n_hq_entry_pop			0x00851000
#define r2n_rq_tbl_addr_load			0x00852000
#define r2n_rq_entry_push			0x00853000

#define nvme_be_cq_ctx_check			0x00860000
#define nvme_be_cq_entry_pop			0x00861000
#define nvme_be_r2n_sq_derive			0x00862000
#define r2n_sq_entry_push			0x00863000

#define seq_sq_ctx_check			0x00870000
#define seq_sq_entry_pop			0x00871000
#define seq_desc_entry_addr_load		0x00872000
#define seq_desc_entry_check			0x00873000
#define seq_next_sq_entry_push			0x00874000

#endif     // CAPRI_H
