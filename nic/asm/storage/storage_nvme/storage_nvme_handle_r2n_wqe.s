/*****************************************************************************
 *  handle_r2n_wqe: Read the R2N WQE posted by ROCE to get the status buffer
 *                  address. Load the status buffer for the next stage.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl0_k k;
struct s1_tbl0_handle_r2n_wqe_d d;
struct phv_ p;

%%
   .param storage_nvme_process_be_status_start

storage_nvme_handle_r2n_wqe_start:

   // Check if WQE needs to be processed. TODO: Status buffer free when 
   // no processing needs to be done ?
   seq		c1, d.opcode, R2N_OPCODE_PROCESS_WQE
   bcf		[!c1], exit

   // Store the IO status buffer post descriptor address in K+I vector 
   sub		r1, d.handle, IO_STATUS_BUF_BE_STATUS_OFFSET // delay slot
   phvwr	p.nvme_kivec_arm_dst7_rrq_desc_addr, r1

   // Process WQE => Set the table and program address for loading the
   // WQE pointer
   LOAD_TABLE_FOR_ADDR_PC_IMM(d.handle, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_nvme_process_be_status_start)

exit:
   LOAD_NO_TABLES
