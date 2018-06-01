/*****************************************************************************
 * storage_seq_p4_hdr.h: Defintions needed for storage sequencer P4+ program
 *****************************************************************************/
 
#ifndef STORAGE_SEQ_P4_HDR_H
#define STORAGE_SEQ_P4_HDR_H

#include "storage_p4_hdr.h"


// Generic Sequencer Queue State. Total size can be 64 bytes at most.
header_type seq_q_state_t {
  fields {
    //pc_offset     : 8;    // Program counter (relative offset)
    rsvd            : 8;    // Hardware reserved field
    cosA            : 4;    // Cos value A
    cosB            : 4;    // Cos value B
    cos_sel         : 8;    // Cos selector
    eval_last       : 8;    // Evaluator of "work ready" for ring
    total_rings     : 4;    // Total number of rings used by this qstate
    host_rings      : 4;    // Number of host facing rings used by this qstate
    pid             : 16;   // PID value to be compared with that from host
    p_ndx           : 16;   // Producer Index
    c_ndx           : 16;   // Consumer Index
    w_ndx           : 16;   // Working consumer index
    num_entries     : 16;   // Number of queue entries (power of 2 of this value)
    base_addr       : 64;   // Base address of queue entries
    entry_size      : 16;   // Size of each queue entry
    next_pc         : 28;   // Next program's PC
    dst_lif         : 11;   // Destination LIF number
    dst_qtype       : 3;    // Destination LIF type (within the LIF)
    dst_qid         : 24;   // Destination queue number (within the LIF)
    dst_qaddr       : 34;   // Destination queue state address
    desc1_next_pc_valid: 1;
    desc1_next_pc   : 28;   // desc bytes 64-127 next program's PC
    pad             : 135;
                            // 
    // When canceling a doorbell push DMA command that is also the last (EOP)
    // in the DMA command set, NOP can't be used due to the EOP. The
    // workaround is to convert the doorbell push into a PHV2MEM into
    // the location below.
    eop_p2m_rsvd    : 32;   // reserved for PHV2MEM eop cmd cancel write
  }
}

// Barco ring 
header_type barco_ring_t {
  fields {
    p_ndx           : 32;   // Producer Index
  }
}


// Sequencer metadata for pushing Barco descriptor
header_type seq_barco_entry_t {
  fields {
    barco_desc_addr : 64;   // Address of the descriptor to push
    barco_pndx_addr : 34;   // 64 bit address of the doorbell to ring
    barco_pndx_shadow_addr: 34;
    barco_desc_size : 4;    // Size of the descriptor to push
    barco_pndx_size : 3;    // Size of the ring state to be loaded
    barco_ring_size : 5;    // log2(ring_size)
    barco_ring_addr : 34;   // Address of the ring

    barco_batch_size: 16;   // barco_desc_addr is a vector of this many descriptors
    barco_batch_mode: 1;    // when barco_batch_mode is set.
  }
}

// Sequencer metadata compression status entry
header_type seq_comp_status_desc0_t {
  fields {
    next_db_addr    : 64;   // 64 bit address of the next doorbell to ring
    next_db_data    : 64;   // 64 bit data of the next doorbell to ring,
                            // also represents barco_desc_addr when next_db_action_barco_push is set
    barco_pndx_addr : 34;   // producer index address
    barco_pndx_shadow_addr: 34;
    barco_desc_size : 4;    // log2(descriptor size)
    barco_pndx_size : 3;    // log2(producer index size)
    barco_ring_size : 5;    // log2(ring_size)
    barco_num_descs : 6;    // initial number of descriptors to xfer to Barco
    status_addr0    : 64;   // Address where compression status will be placed
    status_addr1    : 64;   // 2nd address where compression status will be placed
    intr_addr       : 64;   // Address where interrupt needs to be written
    intr_data       : 32;   // Data that needs to be written for interrupt
    status_len      : 16;   // Length of the compression status
    status_offset0  : 7;    // Add this to status_addr0 before DMA into status_addr1 
    status_dma_en   : 1;    // 1 => DMA status, 0 => don't DMA 
    next_db_en      : 1;    // 1 => Ring next sequencer doorbell, 0 => don't ring
    intr_en         : 1;    // 1 => Fire the MSI-X interrupt, 0 => don't fire
                            // NOTE: Don't enable intr_en and next_db_en together
                            //       as only one will be serviced
                            // Order of evaluation: 1. next_db_en 2. intr_en
    next_db_action_barco_push: 1; // 1 => next_db action is actually Barco push
  }
}

header_type seq_comp_status_desc1_t {
  fields {
    rsvd            : 64;
    comp_buf_addr   : 64;  // used as source (of compressed output) data for SGL PDMA transfer,
                            // only works when output buffer is flat (all compressed blocks
                            // are contiguous)
    aol_src_vec_addr: 64;   // for compress-pad-encrypt: source AOL vector for encrypt
    aol_dst_vec_addr: 64;   // for compress-pad-encrypt: destination AOL vector for encrypt;
                            // otherwise, aol_dst_vec_addr is used as SGL destination
                            // for SGL PDMA transfer (if sgl_pdma_en is set) 
    sgl_vec_addr    : 64;   // SGL vector for padding operation
    pad_buf_addr    : 64;   // pad buffer address
    data_len        : 16;   // Length of the compression data
    pad_boundary_shift  : 5;// log2(padding boundray)
    stop_chain_on_error : 1; // 1 => don't ring next DB on error
    data_len_from_desc  : 1; // 1 => Use data_len in the descriptor, 
                             // 0 => Use the comp_output_data_len
    aol_pad_en      : 1;
    sgl_pad_en      : 1;
    sgl_pdma_en     : 1;
    sgl_pdma_pad_only:1;
    desc_vec_push_en:1;
    copy_src_dst_on_error: 1; // not yet supported
  }
}

// SGL PDMA metadata
header_type chain_sgl_pdma_t {
  fields {
    addr0           : 64;   // SGL data buffer 0 address
    len0            : 32;   // SGL data buffer 0 length
    addr1           : 64;   // SGL data buffer 1 address
    len1            : 32;   // SGL data buffer 1 length
    addr2           : 64;   // SGL data buffer 2 address
    len2            : 32;   // SGL data buffer 2 length
    addr3           : 64;   // SGL data buffer 3 address
    len3            : 32;   // SGL data buffer 3 length
  }
}

// Compression status metadata
header_type seq_comp_status_t {
  fields {
    status          : 16;   // Valid bit and error status
    output_data_len : 16;   // Output bits
    rsvd            : 32;   // Reserved
  }
}

// Compression header metadata
header_type seq_comp_hdr_t {
  fields {
    cksum          : 32;
    data_len       : 16;
    version        : 16;
  }
}

// Sequencer metadata XTS status entry
header_type seq_xts_status_desc0_t {
  fields {
    next_db_addr    : 64;   // 64 bit address of the next doorbell to ring
    next_db_data    : 64;   // 64 bit data of the next doorbell to ring
                            // also represents barco_desc_addr when next_db_action_barco_push is set
    barco_pndx_addr : 34;   // producer index address
    barco_pndx_shadow_addr: 34;
    barco_desc_size : 4;    // log2(descriptor size)
    barco_pndx_size : 3;    // log2(producer index size)
    barco_ring_size : 5;    // log2(ring_size)
    barco_num_descs : 6;    // log2(descriptor set size)
    status_addr0    : 64;   // Address where HW crypto status was placed
    status_addr1    : 64;   // 2nd address where a copy of above status can be made
    intr_addr       : 64;   // Address where interrupt needs to be written
    intr_data       : 32;   // Data that needs to be written for interrupt
    status_len      : 16;   // Length of the crypto status
    status_offset0  : 7;    // Add this to status_addr0 before DMA into status_addr1 
    status_dma_en   : 1;    // 1 => DMA status, 0 => don't DMA 
    next_db_en      : 1;    // 1 => Ring next sequencer doorbell, 0 => don't ring
    intr_en         : 1;    // 1 => Fire the MSI-X interrupt, 0 => don't fire
                            // NOTE: Don't enable intr_en and next_db_en together
                            //       as only one will be serviced
                            // Order of evaluation: 1. next_db_en 2. intr_en
    next_db_action_barco_push: 1; // 1 => next_db action is actually Barco push
  }
}

header_type seq_xts_status_desc1_t {
  fields {
    comp_sgl_src_addr   : 64;// comp SGL source address
    sgl_pdma_dst_addr   : 64;// points to chain_sgl_pdma_t when sgl_pdma_en is set
    decr_buf_addr       : 64;// decrypt buffer address
    data_len            : 16;// valid PDMA data length if sgl_pdma_len_from_desc is set
    blk_boundary_shift  : 5; // log2(block boundary)
    stop_chain_on_error : 1; // 1 => don't ring next DB on error
    comp_len_update_en  : 1; // 1 => read length from cp_hdr and update comp desc/SGL
    comp_sgl_src_en     : 1; // 1 => comp_sgl_src_addr is valid
    comp_sgl_src_vec_en : 1; // 1 => comp_sgl_src_addr is a vector address
    sgl_pdma_en         : 1; // 1 => do PDMA from decr_buf_addr to sgl_pdma_dst_addr
    sgl_pdma_len_from_desc: 1; // 1 => PDMA length is data_len above
    desc_vec_push_en    : 1; // 1 => barco_desc_addr is a vector address
  }
}

// XTS status metadata
header_type seq_xts_status_t {
  fields {
    err             : 64;   // Error status (0: success: >0: failure)
  }
}

// Compression/decompression descriptor
header_type cp_desc_t {
  fields {
    src             : 64;
    dst             : 64;
    cmd             : 16;
    datain_len      : 16;
    extended_len    : 16;
    threshold_len   : 16;
    status_addr     : 64;
    doorbell_addr   : 64;
    doorbell_data   : 64;
    opaque_tag_addr : 64;
    opaque_tag_data : 32;
    status_data     : 32;
  }
}

// Barco AOL (used by XTS)
header_type barco_aol_t {
  fields {
    A0              : 64;
    O0              : 32;
    L0              : 32;
    A1              : 64;
    O1              : 32;
    L1              : 32;
    A2              : 64;
    O2              : 32;
    L2              : 32;
    next_addr       : 64;
    rsvd            : 64;
  }
}

// Barco SGL (used by comp/decomp engines)
header_type barco_sgl_t {
  fields {
    addr0           : 64;   // SGL data buffer 0 address
    len0            : 32;   // SGL data buffer 0 length
    rsvd0           : 32;
    addr1           : 64;   // SGL data buffer 1 address
    len1            : 32;   // SGL data buffer 1 length
    rsvd1           : 32;
    addr2           : 64;   // SGL data buffer 2 address
    len2            : 32;   // SGL data buffer 2 length
    rsvd2           : 32;
    link            : 64;
    rsvd            : 64;
  }
}

// Pad structures

header_type storage_seq_pad192_t {
  fields {
      pad           : 192;
  }
}

header_type storage_seq_pad176_t {
  fields {
      pad           : 176;
  }
}
#define BARCO_SGL_DESC_SIZE         64
#define BARCO_SGL_DESC_SIZE_SHIFT   6

// Storage K+I vectors

// kivec0: header union with stage_2_stage for table 0,
header_type seq_kivec0_t {
  fields {
    w_ndx           : 16;   // Working consumer index
    dst_lif         : 11;   // Destination LIF number
    dst_qtype       : 3;    // Destination LIF type (within the LIF)
    dst_qid         : 24;   // Destination queue number (within the LIF)
    dst_qaddr       : 34;   // Destination queue state address
  }
}

// kivec1: header union with global (128 bits max)
header_type seq_kivec1_t {
  fields {
    src_lif         : 11;   // Source LIF number
    src_qtype       : 3;    // Source LIF type (within the LIF)
    src_qid         : 24;   // Source queue number (within the LIF)
    src_qaddr       : 34;   // Source queue state address
    barco_ring_addr : 34;   // Barco ring address
  }
}

// kivec2: header union with to_stage_2 (128 bits max)
header_type seq_kivec2_t {
  fields {
    sgl_pdma_dst_addr: 64;  // SGL address where data will be placed for PDMA
    sgl_vec_addr     : 64;  // address of SGL vector for hash after compression
  }
}

// kivec2xts: header union with to_stage_2 (128 bits max)
// used by XTS status handler
header_type seq_kivec2xts_t {
  fields {
    sgl_pdma_dst_addr: 64;  // SGL address where data will be placed for PDMA
    decr_buf_addr    : 64;
  }
}

// kivec3: header union with to_stage_3 (128 bits max)
header_type seq_kivec3_t {
  fields {
    comp_buf_addr       : 64;
    pad_len             : 16;
    num_blks            : 5;
    pad_boundary_shift  : 5;
  }
}

// kivec3xts: header union with to_stage_3 (128 bits max)
// used by XTS status handler
header_type seq_kivec3xts_t {
  fields {
    decr_buf_addr    : 64;
  }
}

// kivec4: header union with stage_2_stage for table 0 (160 bits max)
header_type seq_kivec4_t {
  fields {
    barco_desc_addr         : 64;
    barco_ring_addr         : 34;
    barco_pndx_shadow_addr  : 34;
    barco_desc_size         : 4;
    barco_pndx_size         : 3;
    barco_ring_size         : 5;
    barco_num_descs         : 6;
    pad_boundary_shift      : 5;
  }
}

// kivec5: header union with global (128 bits max)
header_type seq_kivec5_t {
  fields {
    intr_addr           : 64;
    pad_buf_addr        : 34;   // pad buffer in HBM
    data_len            : 16;   // Length of compression data (either from descriptor or 
                                // from the compression status)
    status_dma_en       : 1;    // 1 => DMA status, 0 => don't DMA status
    next_db_en          : 1;
    intr_en             : 1;
    next_db_action_barco_push: 1; // next_db action is actually Barco push
    stop_chain_on_error : 1;
    data_len_from_desc  : 1;    // 1 => Use the data length in the descriptor, 
                                // 0 => Use the data lenghth in the status
    aol_pad_en          : 1;
    sgl_pad_en          : 1;
    sgl_pdma_en         : 1;
    sgl_pdma_pad_only   : 1;
    desc_vec_push_en    : 1;
    copy_src_dst_on_error: 1;
  }
}

// kivec5xts: header union with global (128 bits max)
// used by XTS status handler
header_type seq_kivec5xts_t {
  fields {
    intr_addr           : 64;
    data_len            : 16;
    blk_boundary_shift  : 5;
    status_dma_en       : 1;
    next_db_en          : 1;
    intr_en             : 1;
    next_db_action_barco_push: 1;
    stop_chain_on_error : 1;
    comp_len_update_en  : 1;
    comp_sgl_src_en     : 1;
    comp_sgl_src_vec_en : 1;
    sgl_pdma_en         : 1;
    sgl_pdma_len_from_desc: 1;
    desc_vec_push_en    : 1;
  }
}

// kivec6: header union with stage_2_stage for table 3 (160 bits max)
header_type seq_kivec6_t {
  fields {
    aol_src_vec_addr   : 64;
    aol_dst_vec_addr   : 64;
  }
}

// kivec7xts: header union with stage_2_stage for table 2 (160 bits max)
// used by XTS status handler
header_type seq_kivec7xts_t {
  fields {
      comp_desc_addr   : 64;
      comp_sgl_src_addr: 64;
  }
}

// kivec8: header union with to_stage_4 (128 bits max)
header_type seq_kivec8_t {
  fields {
    pad_len             : 16;
    last_blk_len        : 16;
  }
}

// kivec8: header union with to_stage_4 (128 bits max)
// used by XTS status handler
header_type seq_kivec8xts_t {
  fields {
    datain_len          : 16;
  }
}

#define SEQ_Q_STATE_COPY_INTRINSIC(q_state)                             \
  modify_field(q_state.rsvd, rsvd);                                     \
  modify_field(q_state.cosA, cosA);                                     \
  modify_field(q_state.cosB, cosB);                                     \
  modify_field(q_state.cos_sel, cos_sel);                               \
  modify_field(q_state.eval_last, eval_last);                           \
  modify_field(q_state.total_rings, total_rings);                       \
  modify_field(q_state.host_rings, host_rings);                         \
  modify_field(q_state.pid, pid);                                       \

#define SEQ_Q_STATE_COPY_STAGE0(q_state)                                \
  SEQ_Q_STATE_COPY_INTRINSIC(q_state)                                   \
  modify_field(q_state.p_ndx, p_ndx);                                   \
  modify_field(q_state.c_ndx, c_ndx);                                   \
  modify_field(q_state.w_ndx, w_ndx);                                   \
  modify_field(q_state.num_entries, num_entries);                       \
  modify_field(q_state.base_addr, base_addr);                           \
  modify_field(q_state.entry_size, entry_size);                         \
  modify_field(q_state.next_pc, next_pc);                               \
  modify_field(q_state.dst_lif, dst_lif);                               \
  modify_field(q_state.dst_qtype, dst_qtype);                           \
  modify_field(q_state.dst_qid, dst_qid);                               \
  modify_field(q_state.dst_qaddr, dst_qaddr);                           \
  modify_field(q_state.desc1_next_pc_valid, desc1_next_pc_valid);       \
  modify_field(q_state.desc1_next_pc, desc1_next_pc);                   \
  
#define SEQ_Q_STATE_COPY(q_state)                                       \
  SEQ_Q_STATE_COPY_STAGE0(q_state)                                      \
  modify_field(q_state.pad, pad);                                       \

#define SEQ_KIVEC0_USE(scratch, kivec)                                  \
  modify_field(scratch.w_ndx, kivec.w_ndx);                             \
  modify_field(scratch.dst_lif, kivec.dst_lif);                         \
  modify_field(scratch.dst_qtype, kivec.dst_qtype);                     \
  modify_field(scratch.dst_qid, kivec.dst_qid);                         \
  modify_field(scratch.dst_qaddr, kivec.dst_qaddr);                     \

#define SEQ_KIVEC1_USE(scratch, kivec)                                  \
  modify_field(scratch.src_lif, kivec.src_lif);                         \
  modify_field(scratch.src_qtype, kivec.src_qtype);                     \
  modify_field(scratch.src_qid, kivec.src_qid);                         \
  modify_field(scratch.src_qaddr, kivec.src_qaddr);                     \
  modify_field(scratch.barco_ring_addr, kivec.barco_ring_addr);         \

#define SEQ_KIVEC2_USE(scratch, kivec)                                  \
  modify_field(scratch.sgl_pdma_dst_addr, kivec.sgl_pdma_dst_addr);     \
  modify_field(scratch.sgl_vec_addr, kivec.sgl_vec_addr);               \

#define SEQ_KIVEC2XTS_USE(scratch, kivec)                               \
  modify_field(scratch.sgl_pdma_dst_addr, kivec.sgl_pdma_dst_addr);     \
  modify_field(scratch.decr_buf_addr, kivec.decr_buf_addr);             \

#define SEQ_KIVEC3_USE(scratch, kivec)                                  \
  modify_field(scratch.comp_buf_addr, kivec.comp_buf_addr);             \
  modify_field(scratch.pad_len, kivec.pad_len);                         \
  modify_field(scratch.num_blks, kivec.num_blks);                       \
  modify_field(scratch.pad_boundary_shift, kivec.pad_boundary_shift);   \

#define SEQ_KIVEC3XTS_USE(scratch, kivec)                               \
  modify_field(scratch.decr_buf_addr, kivec.decr_buf_addr);             \

#define SEQ_KIVEC4_USE(scratch, kivec)                                  \
  modify_field(scratch.barco_desc_addr, kivec.barco_desc_addr);         \
  modify_field(scratch.barco_pndx_shadow_addr, kivec.barco_pndx_shadow_addr);\
  modify_field(scratch.barco_desc_size, kivec.barco_desc_size);         \
  modify_field(scratch.barco_pndx_size, kivec.barco_pndx_size);         \
  modify_field(scratch.barco_ring_size, kivec.barco_ring_size);         \
  modify_field(scratch.barco_ring_addr, kivec.barco_ring_addr);         \
  modify_field(scratch.barco_num_descs, kivec.barco_num_descs);         \
  modify_field(scratch.pad_boundary_shift, kivec.pad_boundary_shift);   \

#define SEQ_KIVEC5_USE(scratch, kivec)                                  \
  modify_field(scratch.intr_addr, kivec.intr_addr);                     \
  modify_field(scratch.pad_buf_addr, kivec.pad_buf_addr);               \
  modify_field(scratch.data_len, kivec.data_len);                       \
  modify_field(scratch.status_dma_en, kivec.status_dma_en);             \
  modify_field(scratch.next_db_en, kivec.next_db_en);                   \
  modify_field(scratch.intr_en, kivec.intr_en);                         \
  modify_field(scratch.next_db_action_barco_push, kivec.next_db_action_barco_push);\
  modify_field(scratch.stop_chain_on_error, kivec.stop_chain_on_error); \
  modify_field(scratch.data_len_from_desc, kivec.data_len_from_desc);   \
  modify_field(scratch.aol_pad_en, kivec.aol_pad_en);                   \
  modify_field(scratch.sgl_pad_en, kivec.sgl_pad_en);                   \
  modify_field(scratch.sgl_pdma_en, kivec.sgl_pdma_en);                 \
  modify_field(scratch.copy_src_dst_on_error, kivec.copy_src_dst_on_error);\
  modify_field(scratch.sgl_pdma_pad_only, kivec.sgl_pdma_pad_only);     \
  modify_field(scratch.desc_vec_push_en, kivec.desc_vec_push_en);       \

#define SEQ_KIVEC5XTS_USE(scratch, kivec)                               \
  modify_field(scratch.intr_addr, kivec.intr_addr);                     \
  modify_field(scratch.data_len, kivec.data_len);                       \
  modify_field(scratch.blk_boundary_shift, kivec.blk_boundary_shift);   \
  modify_field(scratch.status_dma_en, kivec.status_dma_en);             \
  modify_field(scratch.next_db_en, kivec.next_db_en);                   \
  modify_field(scratch.intr_en, kivec.intr_en);                         \
  modify_field(scratch.next_db_action_barco_push, kivec.next_db_action_barco_push);\
  modify_field(scratch.stop_chain_on_error, kivec.stop_chain_on_error); \
  modify_field(scratch.comp_len_update_en, kivec.comp_len_update_en);   \
  modify_field(scratch.comp_sgl_src_en, kivec.comp_sgl_src_en);         \
  modify_field(scratch.comp_sgl_src_vec_en, kivec.comp_sgl_src_vec_en); \
  modify_field(scratch.sgl_pdma_en, kivec.sgl_pdma_en);                 \
  modify_field(scratch.sgl_pdma_len_from_desc, kivec.sgl_pdma_len_from_desc);\
  modify_field(scratch.desc_vec_push_en, kivec.desc_vec_push_en);       \

#define SEQ_KIVEC6_USE(scratch, kivec)                                  \
  modify_field(scratch.aol_src_vec_addr, kivec.aol_src_vec_addr);       \
  modify_field(scratch.aol_dst_vec_addr, kivec.aol_dst_vec_addr);       \

#define SEQ_KIVEC7XTS_USE(scratch, kivec)                               \
  modify_field(scratch.comp_desc_addr, kivec.comp_desc_addr);           \
  modify_field(scratch.comp_sgl_src_addr, kivec.comp_sgl_src_addr);     \

#define SEQ_KIVEC8_USE(scratch, kivec)                                  \
  modify_field(scratch.pad_len, kivec.pad_len);                         \
  modify_field(scratch.last_blk_len, kivec.last_blk_len);               \

#define SEQ_KIVEC8XTS_USE(scratch, kivec)                               \
  modify_field(scratch.datain_len, kivec.datain_len);                   \

// Macros for ASM param addresses (hardcoded in P4)
#define seq_barco_chain_action_start	    0x82000000
#define seq_comp_status_handler_start       0x82010000
#define seq_comp_sgl_pdma_xfer_start        0x82020000
#define seq_xts_status_handler_start        0x82030000
#define seq_barco_ring_pndx_pre_read0_start 0x82040000
#define seq_barco_ring_pndx_read_start      0x82050000
#define seq_comp_aol_src_pad_start          0x82060000
#define seq_comp_aol_dst_pad_start          0x82070000


#endif     // STORAGE_SEQ_P4_HDR_H
