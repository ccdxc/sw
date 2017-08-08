/*****************************************************************************
 *  Stage: Save ROCE HQ entry to PHV Increment consumer index in ROCE HQ context
 *         to pop the entry. Parse the HQ entry completion handle to determine
 *         actions for next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct r2n_hq_entry_k k;
struct r2n_hq_entry_qpop_d d;
struct phv_ p;

%%
   .param	r2n_rq_tbl_addr_load_start

r2n_hq_entry_pop_start:
   // Save the entire d vector to PHV
   phvwr	p.{rbuf_post_cmd_handle...rbuf_post_data_size},	\
		d.{cmd_handle...data_size}

   // Save relevant parts of the d vector to PHV (for K+I)
   phvwr	p.nvme_tgt_kivec1_rbuf_hdr_addr, d.cmd_handle

   // Pop the entry
   QUEUE_POP(k.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec0_num_entries,
             k.nvme_tgt_kivec0_idx, R2N_HQ_CTX_TABLE_BASE,
             Q_CTX_STATE_OFFSET, Q_CTX_C_NDX_OFFSET)

   // Move the next (skip) stage without loading a table. This is done to
   // line up the stages.
   LOAD_NO_TABLE(r2n_rq_tbl_addr_load_start)

exit:
   nop.e
   nop
