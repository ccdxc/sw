/*****************************************************************************
 *  timeout_iob_addr: Get the address of the IOB that needs to be marked as
 *                    timed out.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl0_k k;
struct s1_tbl0_timeout_iob_addr_d d;
struct phv_ p;

%%
   .param storage_nvme_timeout_iob_skip_start

storage_nvme_timeout_iob_addr_start:

   // Save the IOB address into K+I vector
   phvwr	p.nvme_kivec_t0_s2s_iob_addr, d.iob_addr

   // Load the IOB for the next stage to cleanup
   LOAD_TABLE_FOR_ADDR34_PC_IMM(d.iob_addr, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_timeout_iob_skip_start)
