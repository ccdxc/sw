/*****************************************************************************
 *  seq_r2n_entry_handler: Handle the R2N WQE entry in sequencer. Form the DMA
 *                         command to copy the R2N WQE as part of the push
 *                         operation in the next stage.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_seq_r2n_entry_handler_d d;
struct phv_ p;

%%
   .param storage_tx_seq_q_state_push_start

storage_tx_seq_r2n_entry_handler_start:

   // Update the queue doorbell to clear the scheduler bit
   QUEUE_POP_DOORBELL_UPDATE

   // Overwrite the destination queue parameters in K+I vector based on the
   // PVM status passed in
   phvwr	p.{storage_kivec0_dst_lif...storage_kivec0_dst_qaddr},	\
		d.{dst_lif...dst_qaddr}

   // Setup the source of the mem2mem DMA into DMA cmd 1.
   // For now, not using any override LIF parameters.
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.r2n_wqe_addr, d.r2n_wqe_size,
                     r0, r0, dma_m2m_1)

   // Setup the destination of the mem2mem DMA into DMA cmd 2 (just fill
   // the size). For now, not using any override LIF parameters.
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_DST, r0, d.r2n_wqe_size,
                     r0, r0, dma_m2m_2)

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(d.dst_qaddr, Q_STATE_SIZE,
                             storage_tx_seq_q_state_push_start)
