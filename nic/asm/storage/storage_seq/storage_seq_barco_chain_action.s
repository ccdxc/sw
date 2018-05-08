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

storage_tx_seq_barco_chain_action:

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   // Note that d.p_ndx is a shadow copy of the HW ring producer index.
   add          r6, d.p_ndx, r0
   QUEUE_PUSH_ADDR(STORAGE_KIVEC4_BARCO_RING_ADDR, r6, STORAGE_KIVEC4_BARCO_DESC_SIZE)
   mincr        r6, STORAGE_KIVEC4_BARCO_RING_SIZE, STORAGE_KIVEC4_BARCO_NUM_DESCS

   // Need to word swap before writing back as the p_ndx is little endian
   phvwr	p.barco_doorbell_data_p_ndx, r6.wx

   // DMA command address update
   DMA_ADDR_UPDATE(r7, SEQ_DMA_COMP_CHAIN_BARCO_M2M_DST)
   
   tblwr.f.e    d.p_ndx, r6
   CLEAR_TABLE0 // delay slot
