/*****************************************************************************
 *  Stage: Save the PVM status in CQ entry to PHV. Increment consumer index
 *         in PVM VF's CQ context to pop the entry. Check to see if we can do
 *         PRP assist and load the address for the next stage based on that.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct pvm_vf_cq_entry_k k;
struct pvm_vf_cq_entry_qpop_d d;
struct phv_ p;

%%
   .param	nvme_vf_cq_entry_push_start
pvm_vf_cq_entry_pop_start:
   // Save the entire d vector to PHV
   phvwr	p.{pvm_status_cspec...pvm_status_rsvd}, d.{cspec...rsvd}                                           

   // Pop the entry
   QUEUE_POP(k.nvme_init_kivec0_c_ndx, k.nvme_init_kivec0_num_entries,
             k.nvme_init_kivec0_idx, PVM_VF_CQ_CTX_TABLE_BASE,
             Q_CTX_STATE_OFFSET, Q_CTX_C_NDX_OFFSET)

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(NVME_VF_CQ_CTX_TABLE_BASE, 
                        d.cq_ctx_idx,
                        Q_CTX_SIZE, Q_CTX_SIZE, nvme_vf_cq_entry_push_start)

exit:
   nop.e
   nop
