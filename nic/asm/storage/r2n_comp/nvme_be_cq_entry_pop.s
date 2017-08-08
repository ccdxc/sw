/*****************************************************************************
 *  Stage: Save NVME backend CQ entry to PHV. Increment consumer index in NVME
 *         backend CQ context to pop the entry. Load the ROCE NVME command to
 *         figure out actions for next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_be_cq_entry_k k;
struct nvme_be_cq_entry_qpop_d d;
struct phv_ p;

%%
   .param	nvme_be_r2n_sq_derive_start

nvme_be_cq_entry_pop_start:
   // Save relevant parts of the d vector to PHV (for K+I)
   phvwr	p.nvme_tgt_kivec1_rbuf_hdr_addr, d.cmd_handle

   // Initialize NVME backend status in R2N buffer to Good
   add		r1, d.cmd_handle, R2N_RX_BUF_BE_STA_STATUS_OFFSET
   memwr.d	r1, NVME_BE_STATUS_GOOD

   // Pop the entry
   QUEUE_POP(k.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec0_num_entries,
             k.nvme_tgt_kivec0_idx, NVME_BE_CQ_CTX_TABLE_BASE,
             Q_CTX_STATE_OFFSET, Q_CTX_C_NDX_OFFSET)

   // Set the table and program address 
   LOAD_TABLE_CBASE_OFFSET(d.cmd_handle, R2N_RX_BUF_ENTRY_BE_CMD_OFFSET,
                           R2N_RX_BUF_ENTRY_BE_CMD_HDR_SIZE,
                           nvme_be_r2n_sq_derive_start)

exit:
   nop.e
   nop
