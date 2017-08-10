/*****************************************************************************
 *  Stage: Save the NVME command in SQ entry to PHV. Increment consumer index
 *         in NVME VF's SQ context to pop the entry. Check to see if we can do
 *         PRP assist and load the address for the next stage based on that.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_vf_sq_entry_k k;
struct nvme_vf_sq_entry_qpop_d d;
struct phv_ p;

%%
   .param	pvm_vf_sq_entry_push_start
nvme_vf_sq_entry_pop_start:
   // Save the entire d vector to PHV
   phvwr	p.{pvm_cmd_opc...pvm_cmd_dw15}, d.{opc...dw15}                                           

   // Initialize PRP assist parameters in PHV
   phvwri	p.nvme_init_kivec1_prp_assist, 0
   phvwri	p.pvm_cmd_num_prps, 0

   // PRP assist check. Output of PRP_SIZE(pvm_cmd.dptr2) stored in r7.
   PRP_ASSIST_CHECK(k.nvme_init_kivec0_sq_id, d.opc, d.psdt, d.nlb, 
                    d.dptr1, d.dptr2, queue_pop)

   // PRP assist is to be done
   phvwri	p.nvme_init_kivec1_prp_assist, 1

   // Calculate assist size in terms of number of PRPS. GPR r7 has the PRP 
   // size from PRP_ASSIST_CHECK, shift by 3 to get num PRPS
   srl		r7, r7, 3

   // Use the minimum value of <num PRPs, NVME_MAX_XTRA_PRPS>
   addi		r2, r0, NVME_MAX_XTRA_PRPS
   sle		c1, r2, r7
   bcf		[!c1], max_prps
   phvwr	p.pvm_cmd_num_prps, r7
   b 		queue_pop

max_prps:
   // MAX PRPs as bound by upper limit
   phvwri	p.pvm_cmd_num_prps, NVME_MAX_XTRA_PRPS
     
queue_pop:
   // Pop the entry
   QUEUE_POP(k.nvme_init_kivec0_c_ndx, k.nvme_init_kivec0_num_entries,
             k.nvme_init_kivec0_idx, NVME_VF_SQ_CTX_TABLE_BASE,
             Q_CTX_STATE_OFFSET, Q_CTX_C_NDX_OFFSET)

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(PVM_VF_SQ_CTX_TABLE_BASE, k.nvme_init_kivec0_paired_q_idx,
                        Q_CTX_SIZE, Q_CTX_SIZE, pvm_vf_sq_entry_push_start)

exit:
   nop.e
   nop
