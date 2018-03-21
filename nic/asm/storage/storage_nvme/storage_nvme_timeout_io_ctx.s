/*****************************************************************************
 *  timeout_io_ctx: Marked the oper_status to have timed out via a locked
 *                  table write
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s3_tbl0_k k;
struct s3_tbl0_timeout_io_ctx_d d;
struct phv_ p;

%%

storage_nvme_timeout_io_ctx_start:

   // ARM already detected a timeout ? Drop PHV and exit 
   seq		c1, d.oper_status, IO_CTX_OPER_STATUS_IN_PROGRESS

   // Set error status
   tblwr.c1	d.oper_status, IO_CTX_OPER_STATUS_TIMED_OUT


   // Clearing the doorbell is done in a subsequent P4+ pass.
   // Since there is no DMA operation, drop the PHV.
   phvwr	p.p4_intr_global_drop, 1

   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES

