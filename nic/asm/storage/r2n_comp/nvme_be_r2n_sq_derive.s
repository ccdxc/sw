/*****************************************************************************
 *  Stage: Derive the R2N SQ from the saved R2N command
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_be_r2n_sq_k k;
struct nvme_be_r2n_sq_derive_d d;
struct phv_ p;

%%
   .param	r2n_sq_entry_push_start

nvme_be_r2n_sq_derive_start:
   // Save relevant parts of the d vector to PHV (for K+I)
   phvwr	p.nvme_tgt_kivec0_is_read, d.is_read

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(R2N_SQ_CTX_TABLE_BASE, d.src_queue_id, Q_CTX_SIZE, 
                        Q_CTX_SIZE, r2n_sq_entry_push_start)

exit:
   nop.e
   nop
