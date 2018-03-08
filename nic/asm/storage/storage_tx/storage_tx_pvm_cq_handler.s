/*****************************************************************************
 *  pvm_cq_handler: Save the NVME command in SQ entry to PHV. DMA the
 *                  working consumer index to the consumer index in the
 *                  queue state. Check to see if we can do PRP assist and
 *                  load the address for the next stage based on that.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl_k k;
struct s1_tbl_pvm_cq_handler_d d;
struct phv_ p;

%%
   .param storage_tx_pci_q_state_push_start

storage_tx_pvm_cq_handler_start:

   // Initialize the remaining fields of the PVM command in the PHV
   phvwr	p.{nvme_sta_cspec...nvme_sta_status}, d.{cspec...status}

   // Overwrite the destination queue parameters in K+I vector based on the
   // PVM status passed in
   phvwr	p.{storage_kivec0_dst_lif...storage_kivec0_dst_qaddr},	\
		d.{dst_lif...dst_qaddr}

   // Setup the DMA command to push the NVME status entry. For now keep the 
   // destination address to be 0 (in GPR r0). Set this correctly in the
   // next stage.
   DMA_PHV2MEM_SETUP_ADDR64(nvme_sta_cspec, nvme_sta_status, r0, 
                            dma_p2m_1)

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR34_PC_IMM(d.dst_qaddr, Q_STATE_SIZE,
                                storage_tx_pci_q_state_push_start)
