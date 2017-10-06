/*****************************************************************************
 *  nvme_be_cmd_handler: Save the NVME command to PHV. Load the address of the
 *                       the priority queue state again to increment the 
 *                       running counters in the next stage.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s2_tbl_k k;
struct s2_tbl_nvme_be_cmd_handler_d d;
struct phv_ p;

%%
   .param storage_tx_pri_q_state_incr_start

storage_tx_nvme_be_cmd_handler_start:

   // Save the NVME command to PHV
   phvwr	p.{nvme_cmd_opc...nvme_cmd_dw15}, d.{opc...dw15}                                           

   // Store the original value of NVME command id in R2N WQE in PHV. This will 
   // be saved to HBM so that this command id can be restored when processing
   // the NVME status.
   phvwr	p.r2n_wqe_nvme_cmd_cid, d.cid

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(STORAGE_KIVEC1_SRC_QADDR, Q_STATE_SIZE,
                             storage_tx_pri_q_state_incr_start)

exit:
   nop.e
   nop
