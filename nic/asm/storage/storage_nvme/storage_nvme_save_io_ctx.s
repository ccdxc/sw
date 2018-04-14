/*****************************************************************************
 *  save_io_ctx: Save the oper_status in I/O context via a locked table write
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s3_tbl1_k k;
struct s3_tbl1_save_io_ctx_d d;
struct phv_ p;

%%

storage_nvme_save_io_ctx_start:


   // Set the oper status to in use in the K+I vector and write it to the table.
   // Locked table write protects the modification of this oper status.
   phvwr	p.nvme_kivec_global_oper_status, IO_CTX_OPER_STATUS_IN_PROGRESS
   tblwr	d.oper_status, IO_CTX_OPER_STATUS_IN_PROGRESS

   // Table 1 is not used in the next stage
   CLEAR_TABLE1_e
