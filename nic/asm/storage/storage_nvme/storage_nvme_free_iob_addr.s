/*****************************************************************************
 *  free_iob_addr: Get the address of the IOB that needs to be freed
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl0_k k;
struct s1_tbl0_free_iob_addr_d d;
struct phv_ p;

%%
   .param storage_nvme_cleanup_iob_start

storage_nvme_free_iob_addr_start:

   // Save the IOB address into K+I vector and into PHV
   phvwr	p.nvme_kivec_t0_s2s_iob_addr, d.iob_addr
   phvwr	p.io_ctx_iob_addr, d.iob_addr

   // Load the IOB for the next stage to cleanup
   add		r7, d.iob_addr, IO_BUF_SEQ_QADDR_OFFSET
   LOAD_TABLE_FOR_ADDR34_PC_IMM(r7, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_cleanup_iob_start)
