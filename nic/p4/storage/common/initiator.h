/*****************************************************************************
 * initiator.h: Definitions for modeling the storage initiator P4+ pipeline of 
 *              Capri
 *****************************************************************************/
 
#ifndef INITIATOR_H
#define INITIATOR_H

#include "defines.h"
#include "scratch.h"
#include "queue.h"

// PRP size calculation
#define PRP_SIZE(p)				(PRP_SIZE_SUB - (p & PRP_SIZE_MASK))

// LB size calculation
#define LB_SIZE(nlb)				(nlb << LB_SIZE_SHIFT)


// NVME command sent to PVM
header_type pvm_cmd_t {
  fields {
    // NVME command Dword 0
    opc          : 8;    // Opcode
    fuse         : 2;    // Fusing 2 simple commands
    rsvd0        : 4; 
    psdt         : 2;    // PRP or SGL
    cid          : 16;   // Command identifier
  
    // NVME command Dword 1
    nsid         : 32;   // Namespace identifier

    // NVME command Dword 2
    rsvd2        : 32;

    // NVME command Dword 3
    rsvd3        : 32;

    // NVME command Dwords 4 and 5 
    mptr         : 64;   // Metadata pointer

    // NVME command Dwords 6,7,8 & 9 form the data pointer (PRP or SGL)
    dptr1        : 64;   // PRP1 or address of SGL
    dptr2        : 64;   // PRP2 or size/type/sub_type in SGL

    // NVME command Dwords 10 and 11 
    slba         : 64;   // Starting LBA (for Read/Write) commands

    // NVME command Dword 12
    nlb          : 16;   // Number of logical blocks
    rsvd12       : 10;   
    prinfo       : 4;    // Protection information field
    fua          : 1;    // Force unit access
    lr           : 1;    // Limited retry
 
    // NVME command Dword 13
    dsm          : 8;    // Dataset management
    rsvd13       : 24;

    // NVME command Dword 14
    dw14         : 32;

    // NVME command Dword 15
    dw15         : 32;

    // Information passed to PVM
    vf_id        : 16;   // VF number
    sq_id        : 16;   // Submission queue id
    sq_ctx_idx   : 16;   // Submission Queue Context Index (valid for SQ)
    cq_ctx_idx   : 16;   // Completion Queue Context Index (paired CQ for SQ)

    // PRP assist information
    num_prps     : 8;    // Number of additional PRPs
    rsvd         : 56;
    tickreg      : 64;   // Microsecond reg from NIC
    prp_list_lo  : 512;  // PRP list upto a certain limit
    prp_list_hi  : 512;  // PRP list upto a certain limit
  }
}

// NVME status obtained from PVM
header_type pvm_status_t {
  fields {
    // NVME status Dword 0
    cspec        : 32;   // Command specific

    // NVME status Dword 1
    rsvd0        : 32;

    // NVME status Dword 2
    sq_head      : 16;   // Submission queue head pointer
    sq_id        : 16;   // Submission queue identifier

    // NVME status Dword 3
    cid          : 16;   // Command identifier
    phase        : 1;    // Phase bit
    status       : 15;   // Status

    // Information passed back by PVM
    vf_id        : 16;   // NVME Cmd VF number
    cq_ctx_idx   : 16;   // NVME VF Completion Queue Context Index 
    rsvd         : 96;   // Align to 128 bit boundary
  }
}

header_type nvme_init_kivec0_t {
   fields {
    idx          : 16;     // Index of the Queue context
    state        : 16;     // State of the Queue context
    c_ndx        : 16;     // Consumer index
    p_ndx        : 16;     // Producer index
    num_entries  : 16;     // Number of Queue entries
    sq_id        : 16;     // Submission Queue Index (valid for SQ)
    paired_q_idx : 16;     // Index of Paired Queue to send commands (if any)
   }
}

header_type nvme_init_kivec1_t {
   fields {
    base_addr    : 64;     // Base address of Queue entries
    prp_assist   : 1;      // Download additional PRP entries (upto 16)
   }
}

// NVME VF Submission Queue (SQ)
@pragma scratch_metadata
metadata vf_q_ctx_t nvme_vf_sq_ctx;

@pragma pa_header_union ingress common_t0_s2s
metadata nvme_init_kivec0_t nvme_init_kivec0;

@pragma pa_header_union ingress common_global
metadata nvme_init_kivec1_t nvme_init_kivec1;

// NVME VF Completion Queue (CQ)
@pragma scratch_metadata
metadata vf_q_ctx_t nvme_vf_cq_ctx;

// PVM VF Submission Queue (SQ)
@pragma scratch_metadata
metadata vf_q_ctx_t pvm_vf_sq_ctx;

// PVM VF Completion Queue (CQ)
@pragma scratch_metadata
metadata vf_q_ctx_t pvm_vf_cq_ctx;

// PVM Command metadata
metadata pvm_cmd_t pvm_cmd;

// PVM Status metadata
metadata pvm_status_t pvm_status;

// Placeholder for Capri DMA commands (upto a max of 4 for now)
metadata capri_phv2mem_dma_t dma_cmd0;
metadata capri_phv2mem_dma_t dma_cmd1;
metadata capri_phv2mem_dma_t dma_cmd2;
metadata capri_phv2mem_dma_t dma_cmd3;

#define NVME_INIT_KIVEC0_USE(scratch, kivec)				\
  modify_field(scratch.idx, kivec.idx);					\
  modify_field(scratch.state, kivec.state);				\
  modify_field(scratch.c_ndx, kivec.c_ndx);				\
  modify_field(scratch.p_ndx, kivec.p_ndx);				\
  modify_field(scratch.num_entries, kivec.num_entries);			\
  modify_field(scratch.sq_id, kivec.sq_id);				\
  modify_field(scratch.paired_q_idx, kivec.paired_q_idx);		\

#define NVME_INIT_KIVEC1_USE(scratch, kivec)				\
  modify_field(scratch.base_addr, kivec.base_addr);			\
  modify_field(scratch.prp_assist, kivec.prp_assist);			\

#endif    // INITIATOR_H
