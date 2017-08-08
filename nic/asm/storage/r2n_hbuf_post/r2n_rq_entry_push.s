/*****************************************************************************
 *  Stage: Push Rx buffer address to ROCE RQ
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct r2n_rq_ctx_k k;
struct r2n_rq_ctx_rq_push_d d;
struct phv_ p;

%%

r2n_rq_entry_push_start:
   // Check queue full condition and exit
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the command has to be written to in the
   // ROCE receive queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR(d.base_addr, d.p_ndx, R2N_RX_BUF_POST_ENTRY_SIZE)

   // DMA write of rbuf_post.cmd_handle ... rbuf_post.data_size to ROCE RQ entry
   DMA_PHV2MEM_SETUP(rbuf_post_cmd_handle, rbuf_post_data_size, r7, 
                     dma_cmd0_phv_start, dma_cmd0_phv_end, dma_cmd0_cmdtype, 
                     dma_cmd0_addr, dma_cmd0_host_addr)
   
   // Push the entry
   QUEUE_PUSH_SAVE_PNDX(d.p_ndx, d.num_entries, p.nvme_tgt_kivec0_p_ndx)
 
   // DMA ROCE RQ p_ndx to p_ndx_db (doorbell) register which will trigger the
   // scheduler bit for ROCE P4+ processing
   DMA_PHV2MEM_SETUP(nvme_tgt_kivec0_p_ndx, nvme_tgt_kivec0_p_ndx, d.p_ndx_db, 
                     dma_cmd1_phv_start, dma_cmd1_phv_end, dma_cmd1_cmdtype, 
                     dma_cmd1_addr, dma_cmd1_host_addr)
   
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_cmd0_rsvd, dma_cmd1_cmdeop, p4_txdma_intr_dma_cmd_ptr)
exit:
   nop.e
   nop
