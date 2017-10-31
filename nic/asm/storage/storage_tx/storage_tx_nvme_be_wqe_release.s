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
   .param storage_tx_pci_q_state_push_start
   .param storage_tx_pvm_roce_sq_cb_push_start

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
   DMA_PHV2MEM_SETUP(nvme_be_sta_hdr_time_us, nvme_sta_status, r0, 
                     dma_p2m_1)

   seq		c1, STORAGE_KIVEC0_IS_REMOTE, 1
   bcf		![c1], push_r2n_status
   nop

   // Set the table and program address to push status over ROCE
   // SQ using PVM's ROCE SQ CB for obtaining the p_ndx
   LOAD_TABLE_FOR_ADDR_PARAM(STORAGE_KIVEC0_DST_QADDR, Q_STATE_SIZE,
                             storage_tx_pvm_roce_sq_cb_push_start)

push_r2n_status:
   // Set the table and program address to push status to local R2N CQ
   LOAD_TABLE_FOR_ADDR_PARAM(STORAGE_KIVEC0_DST_QADDR, Q_STATE_SIZE,
                             storage_tx_pci_q_state_push_start)
