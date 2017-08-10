/*****************************************************************************
 *  Stage: Read and process the NVME backend command that was saved before
 *         sending to SSD. Form NVME backend status based on this. Load the
 *         address of the SSD info table to update the running counters in the
 *         next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct ssd_cmds_k k;
struct ssd_cmds_release_d d;
struct phv_ p;

%%
   .param	nvme_be_cq_entry_push_start

ssd_saved_cmd_release_start:

   // Clear the bit corresponding to the cmd_index in the table
   add		r5, r0, k.nvme_tgt_kivec0_cmd_index
   sllv		r4, 1, r5
   xor		r4, r4, -1
   tbland	d.bitmap, r4

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(NVME_BE_CQ_CTX_TABLE_BASE, k.nvme_tgt_kivec0_paired_q_idx, Q_CTX_SIZE,
                        Q_CTX_SIZE, nvme_be_cq_entry_push_start)

exit:
   nop.e
   nop
