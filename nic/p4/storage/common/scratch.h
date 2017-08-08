/*****************************************************************************
 * scratch.h: Definitions for scratch metadata fields
 *****************************************************************************/
 
#ifndef SCRATCH_H
#define SCRATCH_H

header_type storage_scratch_metadata_t {
  fields {
    dummy_field     : 512;    // Dummy action handler
    key             : 96;     // K part of K+I vector
    idx             : 16;     // Index of the Queue context
    state           : 16;     // State of the Queue context
    c_ndx           : 16;     // Consumer index
    p_ndx           : 16;     // Producer index
    p_ndx_db        : 64;     // Doorbell for Producer index
    c_ndx_db        : 64;     // Doorbell for Consumer index
    c_ndx_lo        : 16;     // Consumer index (low priority)
    c_ndx_med       : 16;     // Consumer index (medium priority)
    c_ndx_hi        : 16;     // Consumer index (high priority)
    p_ndx_lo        : 16;     // Producer index (low priority)
    p_ndx_med       : 16;     // Producer index (medium priority)
    p_ndx_hi        : 16;     // Producer index (high priority)
    sq_id           : 16;     // Submission Queue Index (valid for SQ)
    p_ndx_lo_db     : 64;     // Doorbell for Producer index (low priority)
    p_ndx_med_db    : 64;     // Doorbell for Producer index (medium priority)
    p_ndx_hi_db     : 64;     // Doorbell for Producer index (high priority)
    base_addr       : 64;     // Base address of Queue entries
    num_entries     : 16;     // Number of Queue entries
    paired_q_idx    : 16;     // Index of Paired Queue to send commands (if any)
    ssd_handle      : 16;     // SSD handle to select NVME backend
    io_priority     : 8;      // I/O priority to select queue
    cmd_index       : 8;      // Saved command index
    qp_index        : 32;     // queue pair index
    nvme_cmd_w0  : 16;     // First 16 bits of actual NVME command
    nvme_cmd_cid : 16;     // Next 16 bits of actual NVME command (command id)
    rbuf_hdr_addr   : 64;     // Address of the ROCE buffer header
    is_read         : 8;      // If NVME command is a read
    desc_size       : 16;     // The size of desc. at the end.
    desc_base_addr  : 64;     // Address of the descriptor
    desc_entry_addr : 64;     // Current descriptor entry address
    tbl_entry_addr  : 64;     // Table entry address
    tbl_entry_db    : 64;     // Table entry address doorbell
    is_last         : 1;      // Last entry in this sequence.
    prp_assist   : 1;      // Download additional PRP entries (upto 16)
  }
}

@pragma scratch_metadata
metadata storage_scratch_metadata_t storage_scratch;

#endif     // SCRATCH_H
