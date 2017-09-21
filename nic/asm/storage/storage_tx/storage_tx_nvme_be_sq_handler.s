/*****************************************************************************
 *  nvme_be_sq_handler: Read the NVME backend priority submission queue entry.
 *                      Load the actual NVME command for the next stage.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl_k k;
struct s1_tbl_nvme_be_sq_handler_d d;
struct phv_ p;

%%
   .param storage_tx_nvme_be_cmd_handler_start

storage_tx_nvme_be_sq_handler_start:

   // Update the priority queue doorbell to clear the scheduler bit
   PRI_QUEUE_POP_DOORBELL_UPDATE

   // Save the R2N WQE to PHV
   R2N_WQE_FULL_COPY

   // Set the table and program address 
   addi		r7, r0, NVME_BE_NVME_CMD_OFFSET
   add		r7, d.handle, r7
   LOAD_TABLE_FOR_ADDR_PARAM(r7, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                             storage_tx_nvme_be_cmd_handler_start)

exit:
   nop.e
   nop
