/*****************************************************************************
 *  pri_q_state_push: Push to a queue by issuing the DMA commands and
 *                    incrementing the priority p_ndx via ringing the doorbell.
 *                    Assumes that data to be pushed is in DMA command 1.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl_k k;
struct s3_tbl_pri_q_state_push_d d;
struct phv_ p;

%%

storage_tx_pri_q_state_push_start:

   // Push to high priority queue (if priority matches and queue not full)
   PRI_QUEUE_PUSH(STORAGE_KIVEC0_IO_PRIORITY, NVME_BE_PRIORITY_HI,
                  d.p_ndx_hi, d.c_ndx_hi, d.base_addr, d.num_entries,
                  d.entry_size, check_med, exit)
   b	complete_dma

   // Push to medium priority queue (if priority matches and queue not full)
check_med:
   PRI_QUEUE_PUSH(STORAGE_KIVEC0_IO_PRIORITY, NVME_BE_PRIORITY_MED,
                  d.p_ndx_med, d.c_ndx_med, d.base_addr, d.num_entries,
                  d.entry_size, check_lo, exit)
   b	complete_dma

   // Push to low priority queue (if priority matches and queue not full)
check_lo:
   PRI_QUEUE_PUSH(STORAGE_KIVEC0_IO_PRIORITY, NVME_BE_PRIORITY_LO,
                  d.p_ndx_lo, d.c_ndx_lo, d.base_addr, d.num_entries,
                  d.entry_size, exit, exit)

   // Setup the start and end DMA pointers
complete_dma:
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_2_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   LOAD_NO_TABLES
exit:
   nop.e
   nop
