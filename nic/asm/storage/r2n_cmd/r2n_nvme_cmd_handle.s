/*****************************************************************************
 *  Stage: Copy the ROCE NVME command header into PHV which contains the SSD
 *         and priority queue to send it to
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct r2n_nvme_cmd_k k;
struct r2n_nvme_cmd_handle_d d;
struct phv_ p;

%%
   .param	nvme_be_sq_entry_push_start

r2n_nvme_cmd_handle_start:
   // Save relevant parts of the d vector to PHV (for K+I)
   phvwr	p.nvme_tgt_kivec0_io_priority, d.io_priority
   phvwr	p.nvme_tgt_kivec0_is_read, d.is_read

   // Setup the various descriptors in the R2N buffer for passing the status
   // and read data back. 

   // 1. Copy the handle to send back to initiator
   add		r1, k.nvme_tgt_kivec1_rbuf_hdr_addr, R2N_RX_BUF_BE_STA_CMD_HANDLE_OFFSET
   memwr.d	r1, d.cmd_handle

   // 2. Save the queue pair in the command
   add		r1, k.nvme_tgt_kivec1_rbuf_hdr_addr, R2N_RX_BUF_BE_CMD_SRC_Q_ID_OFFSET
   memwr.w	r1, k.nvme_tgt_kivec2_qp_index

   // 3. Update the data pointers
   add		r1, k.nvme_tgt_kivec1_rbuf_hdr_addr, R2N_RX_BUF_BE_CMD_CMD_HANDLE_OFFSET
   add		r2, r0, k.nvme_tgt_kivec1_rbuf_hdr_addr
   addi		r2, r2, R2N_RX_BUF_ENTRY_DATA_OFFSET
   memwr.d	r1, r2

   // 4. For read command, update location to place data 
   seq		c1, d.is_read, 1
   bcf		[!c1], update_sgl_offset
   add		r1, k.nvme_tgt_kivec1_rbuf_hdr_addr, R2N_RX_BUF_WRITE_REQ_LADDR_OFFSET
   memwr.d	r1, r2	// r2 has already been loaded in step 3

update_sgl_offset:
   // 5. Save the ROCE buffer address in the NVME command
   add		r1, k.nvme_tgt_kivec1_rbuf_hdr_addr, R2N_RX_BUF_BE_CMD_SGL_ADDR_OFFSET
   add		r2, r0, k.nvme_tgt_kivec1_rbuf_hdr_addr
   memwr.d	r1, r2

   // Set key to load the NVME backend queue context in the last stage (which is
   // derived based on the SSD handle and I/O priority)
   LOAD_TABLE_FBASE_IDX(NVME_BE_SQ_CTX_TABLE_BASE, d.ssd_handle, 
                        Q_CTX_SIZE, Q_CTX_SIZE, nvme_be_sq_entry_push_start)

exit:
   nop.e
   nop
