 /*****************************************************************************
+ *  push_roce_rq: Push a ROCE RQ WQE by issuing the DMA commands to write
+ *                the ROCE RQ WQE and incrementing the p_ndx via ringing the 
+ *                doorbell. 
+ *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s7_tbl1_k k;
struct s7_tbl1_push_roce_rq_d d;
struct phv_ p;

%%

storage_nvme_push_roce_rq_start:
   // Check queue full condition and exit
   // TODO: Push error handling
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the entry to be pushed has to be 
   // written to in the destination queue. RRQ Base address is stored in 
   // table 1's dst_qaddr for lack of space. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(NVME_KIVEC_T1_S2S_DST_QADDR, d.p_ndx, d.entry_size)

   // Setup the DMA of the ROCE RQ descriptor
   // Src: Is set up by an earlier stage in K+I vector
   // Dst: Calculated by the Queue Push Address and stored in r7
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, 
                     NVME_KIVEC_ARM_DST7_RRQ_DESC_ADDR, 
                     ROCE_RQ_WQE_SIZE, r0, r0, dma_m2m_9)
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r7, 
                              ROCE_RQ_WQE_SIZE, r0, r0, dma_m2m_10)
   
   // Push the entry to the queue (this increments p_ndx and writes to table)
   QUEUE_PUSH(d.p_ndx, d.num_entries)

   // Ring the doorbell for the recipient of the push.
   NVME_ROCE_RQ_PUSH_DOORBELL_RING(dma_p2m_11)


   // Stage 7 table 0 program takes care of clearing all tables and setting up 
   // DMA pointers => do nothing here
exit:
   nop.e
   nop
