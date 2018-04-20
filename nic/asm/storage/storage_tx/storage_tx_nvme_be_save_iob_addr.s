/*****************************************************************************
 *  nvme_be_save_iob_addr : Save the IO buffer pointer passed in the command
 *                          to the R2N WQE
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s2_tbl1_k k;
struct s2_tbl1_nvme_be_save_iob_addr_d d;
struct phv_ p;

%%

storage_tx_nvme_be_save_iob_addr_start:

   // Save the IOB address into the R2N WQE. 
   phvwr	p.r2n_wqe_iob_addr, d.iob_addr

   // Table 1 is not used in the next stage
   CLEAR_TABLE1_e


