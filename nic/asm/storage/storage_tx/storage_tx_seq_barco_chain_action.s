/*****************************************************************************
 *  storage_tx_seq_barco_chain_action: carries out the same operations as
 *                       storage_tx_seq_barco_ring_push_start() except that the
 *                       function is invoked as part of a Barco accelerator chain
 *                       and, hence, k-vec and p-vec setups are different.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl_k k;
struct s3_tbl_seq_barco_chain_action_d d;
struct phv_ p;

%%

storage_tx_seq_barco_chain_action_start:

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   // Note that d.p_ndx is a shadow copy of the HW ring producer index.
   add          r6, d.p_ndx, r0
   QUEUE_PUSH_ADDR(STORAGE_KIVEC4_BARCO_RING_ADDR, r6, STORAGE_KIVEC4_BARCO_DESC_SIZE)
   mincr        r6, STORAGE_KIVEC4_BARCO_RING_SIZE, 1

   // Need to word swap before writing back as the p_ndx is little endian
   phvwr	p.barco_doorbell_data_p_ndx, r6.wx
   tblwr.f      d.p_ndx, r6

   // DMA command address update
   DMA_ADDR_UPDATE(r7, dma_m2m_10)
   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_11_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   LOAD_NO_TABLES
