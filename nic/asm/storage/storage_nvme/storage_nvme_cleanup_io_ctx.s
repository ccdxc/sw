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
   .param storage_nvme_free_iob_start

storage_nvme_cleanup_io_ctx_start:

   // I/O not completed (status path) or IOB not already free (Unused IOB ? i
   // Drop PHV and exit as ARM will handle it
   seq		c1, d.oper_status, IO_CTX_OPER_STATUS_COMPLETED
   seq		c2, d.oper_status, IO_CTX_OPER_STATUS_FREE
   bcf		![c1 | c2], drop_n_exit
   nop

   // Set error status
   tblwr	d.oper_status, IO_CTX_OPER_STATUS_FREE

   // Set the table and program address to put the IO buffer back into
   // the free list in the next stage
   LOAD_TABLE_FOR_ADDR34_PC_IMM(NVME_KIVEC_IOB_RING3_BASE_ADDR,
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_free_iob_start)

drop_n_exit:
   // Since there is no DMA operation, drop the PHV.
   phvwr	p.p4_intr_global_drop, 1

   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES

