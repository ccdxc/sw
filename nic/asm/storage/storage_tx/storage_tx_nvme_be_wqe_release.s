/*****************************************************************************
 *  nvme_be_wqe_release: Save NVME command to be sent to the SSD into memory.
 *                    Load the address of the SSD's SQ context for the next
 *                    stage to push the command.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s4_tbl_k k;
struct s4_tbl_nvme_be_wqe_release_d d;
struct phv_ p;

%%
   .param storage_tx_q_state_push_start

storage_tx_nvme_be_wqe_release_start:

   // Clear the bit corresponding to the cmd_index in the bitmap
   add          r1, r0, STORAGE_KIVEC0_CMD_INDEX
   sllv         r2, 1, r1
   xor          r2, r2, -1

   // Write the bitmap back to the table 
   tbland       d.bitmap, r2

   // Setup the DMA command to push the NVME backend status entry. For now keep 
   // the destination address to be 0 (in GPR r0). Set this correctly in the
   // next stage.
   DMA_PHV2MEM_SETUP(nvme_be_sta_hdr_time_us, nvme_be_sta_nvme_sta_w7, r0, 
                     dma_p2m_1)

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(STORAGE_KIVEC0_DST_QADDR, Q_STATE_SIZE,
                             storage_tx_q_state_push_start)

exit:
   nop.e
   nop
