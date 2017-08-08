/*****************************************************************************
 *  Stage: Check NVME backend SQ context. If busy bit set, yield control. Else
 *         set the busy bit, save SQ context to PHV and load SSD info table.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_be_sq_ctx_k k;
struct nvme_be_sq_ctx_qcheck_d d;
struct phv_ p;

%%
   .param	nvme_be_sq_skip_stage_start
nvme_be_sq_ctx_check_start:
   // Save relevant parts of the d vector to PHV (for K+I)
   phvwr	p.nvme_tgt_kivec0_idx, d.idx
   phvwr	p.nvme_tgt_kivec0_state, d.state
   phvwr	p.nvme_tgt_kivec1_base_addr, d.base_addr
   phvwr	p.nvme_tgt_kivec0_num_entries, d.num_entries
   phvwr	p.nvme_tgt_kivec0_paired_q_idx, d.paired_q_idx
   phvwr	p.nvme_tgt_kivec3_c_ndx_lo, d.c_ndx_lo
   phvwr	p.nvme_tgt_kivec3_c_ndx_med, d.c_ndx_med
   phvwr	p.nvme_tgt_kivec3_c_ndx_hi, d.c_ndx_hi

   // If queue context is locked (state is busy), exit
   QUEUE_CTX_LOCKED(d.state, exit)

   // If all priority queues are empty, exit
   PRI_QUEUE_ALL_EMPTY(d.p_ndx_lo, d.c_ndx_lo, d.p_ndx_med, d.c_ndx_med, 
                       d.p_ndx_hi, d.c_ndx_hi, exit)

   // Lock the queue context
   QUEUE_CTX_LOCK(d.state)

   // Move the next (skip) stage without loading a table. This is done to
   // line up the stages.
   LOAD_NO_TABLE(nvme_be_sq_skip_stage_start)

exit:
   nop.e
   nop
