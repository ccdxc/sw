/*****************************************************************************
 *  save_iob_addr : Save the IO buffer pointer from the IOB free list entry.
 *                 Allocation from the  IOB ring in the previous stage would
 *                 have popped this free list entry from the ring.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl0_k k;
struct s1_tbl0_timeout_iob_addr_d d;
struct phv_ p;

%%
   .param storage_nvme_save_io_ctx_start

storage_nvme_save_iob_addr_start:

   // Save the IOB address into K+I vector
   phvwr	p.nvme_kivec_t0_s2s_iob_addr, d.iob_addr

   // Save the IOB address into the IO context. 
   phvwr	p.io_ctx_iob_addr, d.iob_addr

   // Set table 1 and program address for next stage to save the oper_status in 
   // the I/O context. This CANNOT be the last table config in this path.
   add		r7, d.iob_addr, IO_BUF_IO_CTX_OFFSET
   LOAD_TABLE1_NO_VALID_BIT_e(r7, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_nvme_save_io_ctx_start)
