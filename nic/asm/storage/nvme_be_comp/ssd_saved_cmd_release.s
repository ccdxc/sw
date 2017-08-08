/*****************************************************************************
 *  Stage: Read and process the NVME backend command that was saved before
 *         sending to SSD. Form NVME backend status based on this. Load the
 *         address of the SSD info table to update the running counters in the
 *         next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct ssd_saved_cmds2_k k;
struct ssd_saved_cmds2_release_d d;
struct phv_ p;

%%

ssd_saved_cmd_release_start:
   // Save the entire d vector to PHV
   phvwr	p.{ssd_cmds_bitmap}, d.{bitmap}

   // Clear the bit corresponding to the cmd_index in the table
   add		r5, r0, k.nvme_be_cmd_index
   sllv		r4, 1, r5
   xor		r4, r4, -1
   tbland	d.bitmap, r4

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(NVME_BE_CQ_CTX_TABLE_BASE, k.scq_ctx_idx, Q_CTX_SIZE,
                        Q_CTX_SIZE, nvme_be_cq_entry_push)

exit:
   nop.e
   nop
