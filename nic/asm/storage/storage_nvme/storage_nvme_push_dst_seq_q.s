/*****************************************************************************
 *  push_dst_seq_q: Push the WQE to the next P4+ program in the chain (ROCE/R2N).
 *                  This will be called only when the IO buffer is not being
 *                  punted to arm and needs to be handled in P4+.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s7_tbl1_k k;
struct s7_tbl1_push_dst_seq_q_d d;
struct phv_ p;

%%

storage_nvme_push_dst_seq_q_start:
   // Check queue full condition and exit
   // TODO: Push error handling
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, d.entry_size)

   // DMA the I/O context from PHV to the destinations sequencer queue
   DMA_PHV2MEM_SETUP_ADDR34(seq_r2n_wqe_r2n_wqe_addr, seq_r2n_wqe_dst_qaddr,
                            r7, dma_p2m_12)

   // Push the entry to the queue (this increments p_ndx and writes to table)
   QUEUE_PUSH(d.p_ndx, d.num_entries)

   // Ring the doorbell for the recipient of the push
   NVME_SEQ_QUEUE_PUSH_DOORBELL_RING(dma_p2m_13)

exit:
   // Exit the program here in table 1 processing. Table 0 processing will
   // clear all tables and exit the pipeline.
   nop.e
   nop
