/*****************************************************************************
 * sequencer.h: Definitions for the sequencer block in the P4+ pipeline of 
 *              Capri for sequencing the operations between different blocks 
 *              such as encryption, erasure coding etc
 *****************************************************************************/

#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "defines.h"
#include "scratch.h"
#include "queue.h"

// Sequencer SQ entry metadata
header_type seq_sq_ent_t {
  fields {
    // Sequencer PDMA parameters
    pdma_src_addr       : 64;  // PDMA source address
    pdma_dst_addr       : 64;  // PDMA destination address
    pdma_size           : 64;  // PDMA size

    // Sequencer table entry parameters
    tbl_entry_db        : 64;  // doorbell address for the scheduler bit
    tbl_entry_db_val    : 64;  // value to write to doorbell

    rsvd                : 192; // Pad to 512 bit boundary
  }
}

// Sequencer metadata
header_type seq_t {
  fields {
    // Sequencer table entry address (for this scheduler bit)
    tbl_entry_addr      : 64;

    // Sequencer table entry doorbell (for this scheduler bit)
    tbl_entry_db        : 64;

    // Current descriptor entry address
    desc_entry_addr     : 64;

    // Descriptor metadata
    is_valid            : 1;     // Valid entry
    is_last             : 1;     // Last entry in this sequence.
    is_done             : 1;     // Sequence processed the entry.
    is_error            : 1;     // Error in processing seq entry
    desc_size           : 16;    // The size of desc. at the end.
    desc_base_addr      : 64;    // Address of the descriptor
    next_sq_ctx_addr    : 64;    // Base address of next submission Q
    rsvd                : 172;   // Pad to 512 bit boundary
  }
}

// Capri intrinsic PHV
metadata capri_intrinsic_t intrinsic;

// Sequencer SQ context
metadata q_ctx_t seq_sq_ctx;

// Sequencer SQ entry
metadata seq_sq_ent_t seq_sq_ent;

// Sequencer metadata
metadata seq_t seq;

// Next SQ context
metadata q_ctx_t next_sq_ctx;

// Placeholder for Capri DMA commands 
metadata capri_mem2mem_dma_t dma_cmd0;
metadata capri_mem2mem_dma_t dma_cmd1;
metadata capri_phv2mem_dma_t dma_cmd2;
metadata capri_phv2mem_dma_t dma_cmd3;

#endif    // SEQUENCER_H
