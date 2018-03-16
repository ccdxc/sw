/*****************************************************************************
 *  process_be_status: Process the backend status. If there is any error,
 *                     set I/O status to error and it will be handled to ARM.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s2_tbl0_k k;
struct s2_tbl0_process_be_status_d d;
struct phv_ p;

%%
   .param storage_nvme_process_io_ctx_start

storage_nvme_process_be_status_start:

   // Save NVME Backend status header, NVME status to PHV
   phvwr	p.{nvme_be_sta_hdr_time_us...nvme_be_sta_hdr_r2n_buf_handle},	\
		d.{time_us...r2n_buf_handle}
   phvwr	p.{nvme_sta_cspec...nvme_sta_status}, d.{cspec...status}

   // If I/O returned with error status, mark it in oper_status of I/O context
   seq		c1, d.status, NVME_STATUS_SUCESS
   phvwr.!c1	p.nvme_kivec_global_oper_status, IO_CTX_OPER_STATUS_BE_ERROR

   // Store the I/O context to K+I vector
   phvwr	p.nvme_kivec_t0_s2s_iob_addr, d.iob_addr

   // Load the I/O context for processing in the next stage
   add		r7, d.iob_addr, IO_BUF_IO_CTX_OFFSET
   LOAD_TABLE_FOR_ADDR_PC_IMM(r7, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_nvme_process_io_ctx_start)
