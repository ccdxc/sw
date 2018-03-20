/*****************************************************************************
 *  process_io_ctx: Process I/O context and finish the I/O if there are
 *                  no errors from backend processing:
 *                  1. For read commands which involve large I/O xfers or
 *                     services kickstart sequencers for those services
 *                     (don't execute steps 2,3). Status should be sent only
 *                     after those operations are done.
 *                  2. For read commands which dont involve large I/O xfers
 *                     processing, form PDMA command to DMA data to host memory
 *                  3. For write commands read commands which dont involve
 *                     large I/O xfers, start processing to send NVME status
 *                     back to host.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s3_tbl0_k k;
struct s3_tbl0_process_io_ctx_d d;
struct phv_ p;

%%
   .param storage_nvme_send_read_data_start

storage_nvme_process_io_ctx_start:

   // ARM already detected a timeout ? Drop PHV and exit 
   sne		c1, d.oper_status, IO_CTX_OPER_STATUS_TIMED_OUT
   bcf		[c1], drop_n_exit

   // Form the oper status in the K+I vector and write it to the table.
   // Locked table write protects the modification of this oper status.

   // Backend error ? Set oper status to error, drop PHV and exit
   sne		c2, NVME_KIVEC_GLOBAL_OPER_STATUS, IO_CTX_OPER_STATUS_BE_ERROR	// delay slot
   bcf		[c2], set_err_status
   nop

   // Set oper status to completed and proceed to processing status
   phvwr	p.nvme_kivec_global_oper_status, IO_CTX_OPER_STATUS_COMPLETED
   tblwr	d.oper_status, IO_CTX_OPER_STATUS_COMPLETED


   // Store the NVME SQ address as the destination (for now) in the K+I vector
   // This will be used to derive the actual destination CQ with a lookup later
   phvwr	p.nvme_kivec_t0_s2s_dst_qaddr, d.nvme_sq_qaddr	

   // Set the fields in the K+I vector to xfer data for read command which
   // dont involve large I/O xfers
   seq		c3, d.is_read, 1
   sle		c4, d.nvme_data_len, NVME_READ_MAX_INLINE_DATA_SIZE
   andcf	c1, [c2 & c3]
   phvwr.c1	p.nvme_kivec_t0_s2s_is_read, 1
   phvwr.c1	p.nvme_kivec_global_nvme_data_len, d.nvme_data_len

   // TODO: Branch to kickstart sequencer PDMA for large data xfers 
   //       based on [c2 & c3 & c4]

   // Load the PRP list for the next stage to send Read data for 
   // small xfers
   add		r7, NVME_KIVEC_T0_S2S_IOB_ADDR, IO_BUF_PRP_LIST_OFFSET
   LOAD_TABLE_FOR_ADDR_PC_IMM(r7, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_nvme_send_read_data_start)


set_err_status:
   // Set error status
   phvwr	p.nvme_kivec_global_oper_status, IO_CTX_OPER_STATUS_BE_ERROR
   tblwr	d.oper_status, IO_CTX_OPER_STATUS_BE_ERROR

drop_n_exit:
   // Set PHV to drop
   phvwr	p.p4_intr_global_drop, 1

   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES
