/*****************************************************************************
 *  nvme_be_wqe_save: Save the R2N WQE pointing to the NVME command to be sent
 *                    to the SSD into memory. Load the address of the SSD's
 *                    SQ context for the next stage to push the command.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s4_tbl_k k;
struct s4_tbl_nvme_be_wqe_save_d d;
struct phv_ p;

%%
   .param storage_tx_pci_q_state_push_start

storage_tx_nvme_be_wqe_save_start:

   // Find first clear bit in the bitmap and set it. Store the free bit (cmd_index)
   // in GPR r6 to be used as input to SSD_CMD_ENTRY_ADDR_CALC.
   ffcv		r6, d.bitmap, r0
   addi		r7, r0, 1
   sllv		r7, r7, r6

   // Write the bitmap back to the table and set the command index in PHV
   tblor	d.bitmap, r7

   // Store the command index (in GPR r6) in PHV
   phvwr	p.storage_kivec0_cmd_index, r6

   // Store the command index (in GPR r6) in the NVME command as well
   phvwr	p.nvme_cmd_cid, r6.hx

   // Calculate the address to save R2N WQE based on the command index offset into
   // the SSD's list of outstanding commands. Output is stored in GPR r7.
   SSD_CMD_ENTRY_ADDR_CALC

   // Setup the DMA command to save the R2N WQE entry. Use the address 
   // calculated by SSD_CMD_ENTRY_ADDR_CALC and stored in GRP r7.
   // Note: pri_q_push uses dma_p2m_3 for push because dma_p2m_2 is needed here
   //       to save the SSD bitmap
   DMA_PHV2MEM_SETUP(r2n_wqe_handle, r2n_wqe_pad, r7, dma_p2m_2)

   // Setup the DMA command to push the NVME command entry. For now keep the 
   // destination address to be 0 (in GPR r0). Set this correctly in the
   // next stage.
   DMA_PHV2MEM_SETUP(nvme_cmd_opc, nvme_cmd_dw15, r0, dma_p2m_1)

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(STORAGE_KIVEC0_DST_QADDR, Q_STATE_SIZE,
                             storage_tx_pci_q_state_push_start)
