/*****************************************************************************
 *  Stage: Check NVME VF SQ context. If busy bit set, yield control. Else
 *         set the busy bit, save SQ context to PHV and load the SQ entry.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_qcheck_d d;
struct phv_ p;

%%
   .param	nvme_vf_sq_entry_pop_start
nvme_vf_sq_ctx_check_start:
   // Save relevant parts of the d vector to PHV
   phvwr	p.nvme_init_kivec0_idx, d.idx
   phvwr	p.nvme_init_kivec0_state, d.state
   phvwr	p.nvme_init_kivec0_c_ndx, d.c_ndx
   phvwr	p.nvme_init_kivec1_base_addr, d.base_addr
   phvwr	p.nvme_init_kivec0_num_entries, d.num_entries
   phvwr	p.nvme_init_kivec0_sq_id, d.sq_id
   phvwr	p.nvme_init_kivec0_paired_q_idx, d.paired_q_idx
   
   // If queue context is locked (state is busy), exit
   QUEUE_CTX_LOCKED(d.state, exit)

   // If queue is empty, exit
   QUEUE_EMPTY(d.p_ndx, d.c_ndx, exit)

   // Lock the queue context
   QUEUE_CTX_LOCK(d.state)

   // Set the table and program address 
   LOAD_TABLE_CBASE_IDX(d.base_addr, d.c_ndx, NVME_VF_SQ_ENTRY_SIZE, 
                        NVME_VF_SQ_ENTRY_SIZE, nvme_vf_sq_entry_pop_start)
exit:
   nop.e
   nop
