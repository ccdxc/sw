/*****************************************************************************
 *  timeout_iob_skip: Stage with no work to align the write of the oper_status
 *                    via a locked table write in stage 3
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s2_tbl0_k k;
struct s2_tbl0_timeout_iob_skip_d d;
struct phv_ p;

%%
   .param storage_nvme_timeout_io_ctx_start

storage_nvme_timeout_iob_skip_start:

   // Load the IO context for the next stage to modify oper status with table locking
   add		r7, NVME_KIVEC_T0_S2S_IOB_ADDR, IO_BUF_IO_CTX_OFFSET
   LOAD_TABLE_FOR_ADDR34_PC_IMM(r7, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_timeout_io_ctx_start)
