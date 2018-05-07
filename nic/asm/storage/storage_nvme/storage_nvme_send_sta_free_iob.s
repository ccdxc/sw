/*****************************************************************************
 *  send_sta_free_iob: Post a message to the P4+ program to free the IOB
 *                     from the NVME status handling path.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s7_tbl0_k k;
struct s7_tbl0_send_sta_free_iob_d d;
struct phv_ p;

%%

storage_nvme_send_sta_free_iob_start:
   // Check queue full condition and exit
   // TODO: Push error handling
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, tbl_load)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, d.entry_size)

   // DMA the I/O context from PHV to the destinations sequencer queue
   DMA_PHV2MEM_SETUP_ADDR34(io_ctx_iob_addr, io_ctx_iob_addr,
                            r7, dma_p2m_7)

   // Push the entry to the queue (this increments p_ndx and writes to table)
   QUEUE_PUSH(d.p_ndx, d.num_entries)

   // Ring the doorbell for the recipient of the push
   NVME_SEND_STA_FREE_IOB_DOORBELL_RING(dma_p2m_8)

   // DMA commands 9, 10, 11 are used by storage_nvme_push_roce_rq
   // which executes in stage 7, table 1 context

   // Setup the start and end DMA pointers 
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_11_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   LOAD_NO_TABLES
