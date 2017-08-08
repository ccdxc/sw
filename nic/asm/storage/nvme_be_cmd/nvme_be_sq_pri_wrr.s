/*****************************************************************************
 *  Stage: Perform weighted round robin between the 3 priority queues and pick
 *         a queue to pop the command from. Load that queue's address for the
 *         next stage. If no command can be popped, exit the pipeline.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_be_ssd_info_k k;
struct nvme_be_ssd_info_wrr_d d;
struct phv_ p;

%%
   .param	nvme_be_sq_entry_copy_start
nvme_be_sq_pri_wrr_start:
   // Service high priority queue 
   NVME_BE_SERVICE_PRI_QUEUE(d.num_running, d.hi_running, d.hi_weight,
                             NVME_BE_PRIORITY_HI,
                             k.nvme_tgt_kivec3_p_ndx_hi,
                             k.nvme_tgt_kivec3_c_ndx_hi,
                             k.nvme_tgt_kivec1_base_addr,
                             k.nvme_tgt_kivec0_num_entries, 
                             NVME_BE_SQ_ENTRY_SIZE, R2N_NVME_HDR_SIZE,
                             check_med)

   // Save the priority queue's c_ndx in the key for the "pop" stage
   phvwr	p.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec3_c_ndx_hi

check_med:
   // Service medium priority queue next 
   NVME_BE_SERVICE_PRI_QUEUE(d.num_running, d.med_running, d.med_weight,
                             NVME_BE_PRIORITY_MED,
                             k.nvme_tgt_kivec3_p_ndx_med,
                             k.nvme_tgt_kivec3_c_ndx_med,
                             k.nvme_tgt_kivec1_base_addr,
                             k.nvme_tgt_kivec0_num_entries, 
                             NVME_BE_SQ_ENTRY_SIZE, R2N_NVME_HDR_SIZE,
                             check_med)

   // Save the priority queue's c_ndx in the key for the "pop" stage
   phvwr	p.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec3_c_ndx_med

check_lo:
   // Service low priority queue last 
   NVME_BE_SERVICE_PRI_QUEUE(d.num_running, d.lo_running, d.lo_weight,
                             NVME_BE_PRIORITY_LO,
                             k.nvme_tgt_kivec3_p_ndx_lo,
                             k.nvme_tgt_kivec3_c_ndx_lo,
                             k.nvme_tgt_kivec1_base_addr,
                             k.nvme_tgt_kivec0_num_entries, 
                             NVME_BE_SQ_ENTRY_SIZE, R2N_NVME_HDR_SIZE,
                             exit)

   // Save the priority queue's c_ndx in the key for the "pop" stage
   phvwr	p.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec3_c_ndx_lo

exit:
   nop.e
   nop
