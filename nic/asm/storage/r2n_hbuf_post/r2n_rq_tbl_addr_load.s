/*****************************************************************************
 *  Stage: Load the ROCE RQ context table for the next stage to push the Rx
 *         buffer
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct r2n_rq_tbl_addr_k k;
struct phv_ p;

%%
   .param	r2n_rq_entry_push_start

r2n_rq_tbl_addr_load_start:
   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(R2N_RQ_CTX_TABLE_BASE, k.nvme_tgt_kivec0_paired_q_idx, 
                        Q_CTX_SIZE, Q_CTX_SIZE, r2n_rq_entry_push_start)

