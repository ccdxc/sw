/*****************************************************************************
 *  seq_barco_ring_push: Push to Barco XTS ring by issuing the mem2mem DMA
 *                       commands and incrementing the p_ndx via ringing the
 *                       doorbell. Assumes that data to be pushed has its
 *                       source in DMA cmd 1 and destination in DMA cmd 2.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_seq_barco_ring_push_d d;
struct phv_ p;

%%

storage_tx_seq_barco_ring_push_start:

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(STORAGE_KIVEC1_SSD_CI_ADDR,  d.p_ndx, STORAGE_KIVEC1_XTS_DESC_SIZE)
   add      r6, r0, d.p_ndx
   addi     r6, r6, 1
   phvwr	p.qpush_doorbell_data_data, r6

   // DMA command address update
   DMA_ADDR_UPDATE(r7, dma_m2m_2)
   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_3_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   LOAD_NO_TABLES
