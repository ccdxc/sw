/*****************************************************************************
 *  Stage: Save ROCE CQ entry to PHV Increment consumer index in ROCE CQ context
 *         to pop the entry. Parse the CQ entry completion handle to determine
 *         actions for next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct r2n_cq_entry_k k;
struct r2n_cq_entry_qpop_d d;
struct phv_ p;

%%
   .param	r2n_nvme_cmd_handle_start
   .param	r2n_cq_rx_buf_get_start

r2n_cq_entry_pop_start:
   // Save the entire d vector to PHV
   phvwr	p.{rcq_ent_comp_type...rcq_ent_qp_index},	\
		d.{comp_type...qp_index}

   // Save relevant parts of the d vector to PHV (for K+I)
   phvwr	p.nvme_tgt_kivec1_rbuf_hdr_addr, d.cmd_handle
   phvwr	p.nvme_tgt_kivec2_qp_index, d.qp_index

   // Pop the entry
   QUEUE_POP(k.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec0_num_entries,
             k.nvme_tgt_kivec0_idx, R2N_CQ_CTX_TABLE_BASE,
             Q_CTX_STATE_OFFSET, Q_CTX_C_NDX_OFFSET)

   // Check if this is a new NVME command
   sne		c1, d.comp_type, R2N_COMP_TYPE_NEW_BUFFER
   bcf		[c1], check_status_xfer
   nop

   // Set key to load the ROCE NVME command header in next stage.
   // Command is at an offset into the ROCE Rx buffer.
   LOAD_TABLE_CBASE_OFFSET(d.cmd_handle,
                           R2N_RX_BUF_ENTRY_BE_CMD_OFFSET,
                           R2N_RX_BUF_ENTRY_BE_CMD_HDR_SIZE,
                           r2n_nvme_cmd_handle_start)

check_status_xfer:   
   // Check if this is a status xfer completion:
   // comp_type == R2N_COMP_TYPE_SEND_REQ && cmd_handle != 0
   sne		c1, d.comp_type, R2N_COMP_TYPE_SEND_REQ
   seq		c2, d.cmd_handle, 0
   bcf		[c1 | c2], exit
   nop

   // Set key to load the ROCE RQ buffer post address to read in next stage
   LOAD_TABLE_CBASE_OFFSET(d.cmd_handle,
                           R2N_RX_BUF_ENTRY_BUF_POST_OFFSET,
                           R2N_RX_BUF_ENTRY_BUF_POST_SIZE,
                           r2n_cq_rx_buf_get_start)

exit:
   nop.e
   nop
