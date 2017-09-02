/*****************************************************************************
 *  r2n_sq_handler: Read the R2N WQE posted by local PVM to get the pointer to
 *                  the NVME backend command. Call the next stage to read the
 *                  NVME backend command to determine the SSD queue and
 *                  priority ring to post to.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

// TODO: FIXME the table numbering
struct s1_tbl_k k;
struct s1_tbl_r2n_sq_handler_d d;
struct phv_ p;

%%
   .param storage_tx_nvme_be_wqe_prep_start

storage_tx_r2n_sq_handler_start:

   // Update the priority queue doorbell to clear the scheduler bit
   PRI_QUEUE_POP_DOORBELL_UPDATE

   // Save the R2N WQE to PHV
   R2N_WQE_BASE_COPY

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(d.handle, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                             storage_tx_nvme_be_wqe_prep_start)

exit:
   nop.e
   nop
