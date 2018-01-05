/*****************************************************************************
 *  nvme_sq_handler: Save the NVME command in SQ entry to PHV. DMA the
 *                   working consumer index to the consumer index in the
 *                   queue state. Check to see if we can do PRP assist and
 *                   load the address for the next stage based on that.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl_k k;
struct s1_tbl_nvme_sq_handler_d d;
struct phv_ p;

%%
   .param storage_tx_pci_q_state_push_start

storage_tx_nvme_sq_handler_start:

   // Save the NVME command to PHV
   phvwr	p.{nvme_cmd_opc...nvme_cmd_dw15}, d.{opc...dw15}                                           

   // Initialize PRP assist parameters in PHV
   phvwri	p.pvm_cmd_trailer_num_prps, 0
   phvwri	p.pvm_cmd_trailer_tickreg, 0

   // Initialize set PRP assist flag to false (unless check logic overrides this)
   phvwri	p.storage_kivec0_prp_assist, 0

   // PRP assist check. Output of PRP_SIZE(d.dptr2) stored in r7.
   PRP_ASSIST_CHECK(STORAGE_KIVEC0_IS_Q0, d.opc, d.psdt, d.nlb, 
                    d.dptr1, d.dptr2, dma_nvme_cmd)

   // PRP assist is to be done
   phvwri	p.storage_kivec0_prp_assist, 1

   // Calculate assist size in terms of number of PRPS. GPR r7 has the PRP 
   // size from PRP_ASSIST_CHECK, shift by 3 to get num PRPS
   srl		r7, r7, 3

   // Use the minimum value of <num PRPs, NVME_MAX_XTRA_PRPS>
   addi		r2, r0, NVME_MAX_XTRA_PRPS
   sle		c1, r2, r7
   bcf		[!c1], max_prps
   phvwr	p.pvm_cmd_trailer_num_prps, r7
   b 		dma_nvme_cmd

max_prps:
   // MAX PRPs as bound by upper limit
   phvwri	p.pvm_cmd_trailer_num_prps, NVME_MAX_XTRA_PRPS
     
dma_nvme_cmd:
   // Setup the DMA command to push the NVME command entry. For now keep the 
   // destination address to be 0 (in GPR r0). Set this correctly in the
   // next stage.
   DMA_PHV2MEM_SETUP(nvme_cmd_opc, pvm_cmd_trailer_num_prps, r0, 
                     dma_p2m_1)

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(STORAGE_KIVEC0_DST_QADDR, Q_STATE_SIZE,
                             storage_tx_pci_q_state_push_start)
