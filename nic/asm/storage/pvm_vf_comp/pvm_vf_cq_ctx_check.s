/*****************************************************************************
 *  Stage: Check PVM VF CQ context. If busy bit set, yield control. Else
 *         set the busy bit, save CQ context to PHV and load CQ entry.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct tx_table_s0_t1_k k;
struct tx_table_s0_t0_qcheck_d d;
struct phv_ p;

%%
   .param	pvm_vf_cq_entry_pop_start
pvm_vf_cq_ctx_check_start:
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
   LOAD_TABLE_CBASE_IDX(d.base_addr, d.c_ndx, PVM_VF_CQ_ENTRY_SIZE, 
                        PVM_VF_CQ_ENTRY_SIZE, pvm_vf_cq_entry_pop_start)
exit:
   nop.e
   nop
