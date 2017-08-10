/*****************************************************************************
 * target.h: Definitions for modeling the storage target P4+ pipeline of Capri
 *****************************************************************************/
 
#ifndef TARGET_H
#define TARGET_H

#include "defines.h"
#include "scratch.h"
#include "queue.h"



// Check if NVME backend has not got a command to process yet AND
//       if SSD's priority running counter is < priority weight AND
//       if the priority queue in the SQ can be popped
// If all match,
//       1. Load the queue address for the next stage 
//       2. Set the fact that a command to process was got
//       3. Increment priority running counter
#define SERVICE_NVME_BE_SSD_QUEUE(be, ssd, sq, pri_ctr, pri_weight, 	\
                                  CHK_FN, LOAD_FN, stage_entry)		\
  if ((be.got_cmd != 1) and (ssd.pri_ctr < ssd.pri_weight) and		\
      (not(CHK_FN(sq)))) {						\
    LOAD_FN(sq, common_te0_phv, NVME_BE_SQ_ENTRY_SIZE, R2N_NVME_HDR_SIZE,\
            stage_entry)						\
    modify_field(be.got_cmd, 1);					\
    modify_field(ssd.pri_ctr, ssd.pri_ctr + 1);				\
  }

// Macros to copy NVME command
#define COPY_NVME_CMD1(be_cmd)						\
  modify_field(be_cmd.src_queue_id, src_queue_id);			\
  modify_field(be_cmd.ssd_handle, ssd_handle);				\
  modify_field(be_cmd.io_priority, io_priority);			\
  modify_field(be_cmd.is_read, is_read);				\
  modify_field(be_cmd.cmd_handle, cmd_handle);				\
  modify_field(be_cmd.pad, 0);						\

#define COPY_NVME_CMD2(be_cmd)						\
  modify_field(be_cmd.nvme_cmd_w0, nvme_cmd_w0);			\
  modify_field(be_cmd.nvme_cmd_cid, nvme_cmd_cid);			\
  modify_field(be_cmd.nvme_cmd_hi, nvme_cmd_hi);			\

#define COPY_NVME_STATUS(be_sta)					\
  modify_field(be_sta.nvme_sta_lo, nvme_sta_lo);			\
  modify_field(be_sta.nvme_sta_cid, nvme_sta_cid);			\
  modify_field(be_sta.nvme_sta_w7, nvme_sta_w7);			\

// Macros to save the SSD information into PHV
#define COPY_SSD_INFO(ssd_info)						\
  modify_field(ssd_info.lo_weight, lo_weight);				\
  modify_field(ssd_info.med_weight, med_weight);			\
  modify_field(ssd_info.hi_weight, hi_weight);				\
  modify_field(ssd_info.lo_running, lo_running);			\
  modify_field(ssd_info.med_running, med_running);			\
  modify_field(ssd_info.hi_running, hi_running);			\
  modify_field(ssd_info.num_running, num_running);			\
  modify_field(ssd_info.max_cmds, max_cmds);


// ROCE Completion Queue entry
header_type rcq_ent_t {
  fields {
    comp_type    : 32;     // Completion type 
    rsvd         : 32;     // Padding for 64 bit alignment
    cmd_handle   : 64;     // Rx buffer entry pointer
    status       : 32;     // Status
    qp_index     : 32;     // queue pair index
  }
}

// ROCE buffer post entry
header_type rbuf_post_t {
  fields {
    cmd_handle   : 64;     // Buffer pointer
    local_addr   : 64;     // Local address pointer (for data)
    data_size    : 64;     // Buffer size (for data)
  }
}

// ROCE Send Queue entry 
header_type rsq_ent_t {
  fields {
    cmd_handle   : 64;     // Command handle
    cmd_type     : 64;     // Command type
    remote_addr  : 64;     // Remote address
    local_addr   : 64;     // Local address
    data_size    : 64;     // Buffer size
  }
}

// ROCE2NVME metadata
header_type r2n_t {
  fields {
    // ROCE buffer header address from which to derive the offsets
    rbuf_hdr_addr    : 64;     // Address of the ROCE buffer header

    // Bits set to alter control flow in P4
    send_pvm_error   : 1;      // Send error to PVM
    new_cmd     : 1;      // New command to be given to NVME backend
    status_xfer : 1;      // Status Xfer done => return buffer to ROCE


    // Intrinsic bits in PHV
    recirc           : 1;      // Set to 1 if the packet is recirculated
                               // back to the pipeline

  }
}

// NVME backend command header carried over ROCE
header_type nvme_be_cmd_t {
  fields {
    src_queue_id : 32;     // ROCE source queue id
    ssd_handle   : 16;     // SSD handle to select NVME backend
    io_priority  : 8;      // I/O priority to select queue
    is_read      : 8;      // If NVME command is a read
    cmd_handle   : 64;     // Back pointer to the ROCE command address
    pad          : 384;    // Flit alignment
    nvme_cmd_w0  : 16;     // First 16 bits of actual NVME command
    nvme_cmd_cid : 16;     // Next 16 bits of actual NVME command (command id)
    nvme_cmd_hi  : 480;    // Last 60 bytes of actual NVME command
  }
}


// NVME response read from backend carried over ROCE
header_type nvme_be_resp_t {
  fields {
    time_us      : 32;     // Timestamp in usec
    be_status    : 8;      // Backend status
    is_q0        : 8;      // Is queue 0 (admin queue)
    rsvd         : 16;     // Padding for 64 bit alignment
    cmd_handle   : 64;     // Pointer to the ROCE buffer
    nvme_sta_lo  : 96;     // First 12 bytes of NVME status
    nvme_sta_cid : 16;     // Next 16 bits of NVME status (command id)
    nvme_sta_w7  : 16;     // Final 16 bits of NVME status
  }
}

// NVME backend metadata
header_type nvme_be_t {
  fields {
    // The index to save/retrieve the NVME command passed to SSD
    cmd_index        : 8;
    // Bits set to alter control flow in P4
    got_cmd          : 1;      // Got a command to pop and run
  }
}

// SSD information metadata (used by NVME backend)
header_type ssd_info_t {
  fields {
    lo_weight   : 8;       // Weight of low pri queue
    med_weight  : 8;       // Weight of medium pri queue
    hi_weight   : 8;       // Weight of high pri queue
    lo_running  : 8;       // Number of commands running from low pri queue
    med_running : 8;       // Number of commands running from medium pri queue
    hi_running  : 8;       // Number of commands running from high pri queue
    num_running : 8;       // Total number of commands running
    max_cmds    : 8;       // Maximum number of commands than can be running
  }
}

// Bitmap of command ids pending to the SSD
header_type ssd_cmds_t {
  fields {
    bitmap      : 64;      // Bitmap of allocated (1) vs free (0) entries
  }
}

header_type nvme_tgt_kivec0_t {
  fields {
    idx          : 16;     // Index of the Queue context
    state        : 16;     // State of the Queue context
    c_ndx        : 16;     // Consumer index
    p_ndx        : 16;     // Producer index
    num_entries  : 16;     // Number of Queue entries
    sq_id        : 16;     // Submission Queue Index (valid for SQ)
    paired_q_idx : 16;     // Index of Paired Queue to send commands (if any)
    ssd_handle   : 16;     // SSD handle to select NVME backend
    io_priority  : 8;      // I/O priority to select queue
    cmd_index    : 8;      // Index to save/retrieve NVME command passed to SSD
    is_read      : 8;      // If NVME command is a read
  }
}

header_type nvme_tgt_kivec1_t {
  fields {
    base_addr     : 64;    // Base address of Queue entries
    rbuf_hdr_addr : 64;    // Address of the ROCE buffer header
  }
}

header_type nvme_tgt_kivec2_t {
  fields {
    qp_index      : 32;    // Address of the ROCE buffer header
  }
}

header_type nvme_tgt_kivec3_t {
  fields {
    c_ndx_lo     : 16;     // Consumer index (low priority)
    c_ndx_med    : 16;     // Consumer index (medium priority)
    c_ndx_hi     : 16;     // Consumer index (high priority)
    p_ndx_lo     : 16;     // Producer index (low priority)
    p_ndx_med    : 16;     // Producer index (medium priority)
    p_ndx_hi     : 16;     // Producer index (high priority)
  }
}

@pragma pa_header_union ingress common_t0_s2s
metadata nvme_tgt_kivec0_t nvme_tgt_kivec0;

@pragma pa_header_union ingress common_global
metadata nvme_tgt_kivec1_t nvme_tgt_kivec1;

@pragma pa_header_union ingress to_stage_2
metadata nvme_tgt_kivec2_t nvme_tgt_kivec2;

@pragma pa_header_union ingress to_stage_3
metadata nvme_tgt_kivec3_t nvme_tgt_kivec3;

// ROCE Completion Queue (CQ) Context
@pragma scratch_metadata
metadata q_ctx_t rcq_ctx;

// ROCE Rx buffer Queue (RQ) context 
@pragma scratch_metadata
metadata q_ctx_t rrq_ctx;

// ROCE Host buffer Queue (HQ) context 
@pragma scratch_metadata
metadata q_ctx_t rhq_ctx;

// ROCE Send Queue (SQ) context 
@pragma scratch_metadata
metadata q_ctx_t rsq_ctx;

// PVM Error Queue context 
@pragma scratch_metadata
metadata q_ctx_t peq_ctx;

// SSD Submission Queue (SQ) context 
@pragma scratch_metadata
metadata q_ctx_t ssq_ctx;

// SSD Completion Queue (CQ) context
@pragma scratch_metadata
metadata q_ctx_t scq_ctx;

// NVME backend Submission Queue (SQ) context
@pragma scratch_metadata
metadata pri_q_ctx_t nsq_ctx;

// NVME backend Completion Queue (CQ) context
@pragma scratch_metadata
metadata q_ctx_t ncq_ctx;

// ROCE Completion Queue entry
metadata rcq_ent_t rcq_ent;

// ROCE Send Queue entry 
metadata rsq_ent_t rsq_ent;

// ROCE Rx buffer post entry
metadata rbuf_post_t rbuf_post;

// The metadata for the PHV passed to the roce2nvme layer
@pragma scratch_metadata
metadata r2n_t r2n;

// NVME backend response scratch
@pragma scratch_metadata
metadata nvme_be_resp_t nvme_be_resp;

// NVME backend response in PHV 
metadata nvme_be_resp_t nvme_be_resp_p;

// NVME backend command
@pragma scratch_metadata
metadata nvme_be_cmd_t nvme_be_cmd;

// NVME backend command in PHV
metadata nvme_be_cmd_t nvme_be_cmd_p;

// The metadata for NVME backend SSD information
@pragma scratch_metadata
metadata ssd_info_t ssd_info;

// The metadata for NVME backend's list of commands outstanding to the SSD
@pragma scratch_metadata
metadata ssd_cmds_t ssd_cmds;

// The metadata for the PHV passed to the NVME backend layer
@pragma scratch_metadata
metadata nvme_be_t nvme_be;

// Placeholder for Capri DMA commands 
metadata capri_mem2mem_dma_t dma_cmd4;
metadata capri_mem2mem_dma_t dma_cmd5;
metadata capri_mem2mem_dma_t dma_cmd6;
metadata capri_mem2mem_dma_t dma_cmd7;

metadata capri_phv2mem_dma_t dma_cmd0;
metadata capri_phv2mem_dma_t dma_cmd1;
metadata capri_phv2mem_dma_t dma_cmd2;
metadata capri_phv2mem_dma_t dma_cmd3;

#define NVME_TGT_KIVEC0_USE(scratch, kivec)				\
  modify_field(scratch.idx, kivec.idx);					\
  modify_field(scratch.state, kivec.state);				\
  modify_field(scratch.c_ndx, kivec.c_ndx);				\
  modify_field(scratch.p_ndx, kivec.p_ndx);				\
  modify_field(scratch.num_entries, kivec.num_entries);			\
  modify_field(scratch.sq_id, kivec.sq_id);				\
  modify_field(scratch.paired_q_idx, kivec.paired_q_idx);		\
  modify_field(scratch.ssd_handle, kivec.ssd_handle);			\
  modify_field(scratch.io_priority, kivec.io_priority);			\
  modify_field(scratch.cmd_index, kivec.cmd_index);			\
  modify_field(scratch.is_read, kivec.is_read);				\

#define NVME_TGT_KIVEC1_USE(scratch, kivec)				\
  modify_field(scratch.base_addr, kivec.base_addr);			\
  modify_field(scratch.rbuf_hdr_addr, kivec.rbuf_hdr_addr);		\

#define NVME_TGT_KIVEC2_USE(scratch, kivec)				\
  modify_field(scratch.qp_index, kivec.qp_index);			\

#define NVME_TGT_KIVEC3_USE(scratch, kivec)				\
  modify_field(scratch.c_ndx_lo, kivec.c_ndx_lo);			\
  modify_field(scratch.c_ndx_med, kivec.c_ndx_med);			\
  modify_field(scratch.c_ndx_hi, kivec.c_ndx_hi);			\
  modify_field(scratch.p_ndx_lo, kivec.p_ndx_lo);			\
  modify_field(scratch.p_ndx_med, kivec.p_ndx_med);			\
  modify_field(scratch.p_ndx_hi, kivec.p_ndx_hi);			\

#endif    // TARGET_H
