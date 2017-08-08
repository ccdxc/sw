/*****************************************************************************
 *  Stage: Enqueue descriptor to next submission queue. Prepare DMA commands
 *         that will (a) add the descriptory and (b) increment the p_ndx and
 *         ring the doorbell for the next submission queue.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct seq_next_sq_ctx_k k;
struct seq_next_sq_ctx_qpush_d d;
struct phv_ p;

%%

seq_next_sq_entry_push_start:
   // Save the entire d vector to PHV
   phvwr	p.{next_sq_ctx_idx...next_sq_ctx_rsvd}, d.{idx...rsvd}

   // Check queue full condition and exit
   QUEUE_FULL(d.p_ndx, d.c_ndx, d.num_entries, exit)

   // Calculate the address to which the command has to be written to in the
   // next submission queue. Output will be stored in GPR r7.
   QUEUE_PUSH_ADDR_RSIZE(d.base_addr, d.p_ndx, k.seq_desc_size)

   // Setup the source of the Mem2Mem DMA (the descriptor)
   DMA_MEM2MEM_SETUP_RSIZE(k.seq_desc_base_addr, k.seq_desc_size,
                           CAPRI_DMA_M2M_TYPE_SRC, dma_cmd0_cmdtype,
                           dma_cmd0_addr, dma_cmd0_data_size,
                           dma_cmd0_mem2mem_type, dma_cmd0_host_addr)

   // Setup the destination of the Mem2Mem DMA (the descriptor)
   DMA_MEM2MEM_SETUP_RSIZE(r7, k.seq_desc_size,
                           CAPRI_DMA_M2M_TYPE_DST, dma_cmd1_cmdtype,
                           dma_cmd1_addr, dma_cmd1_data_size,
                           dma_cmd1_mem2mem_type, dma_cmd1_host_addr)

   // Push the entry
   QUEUE_PUSH_SAVE_PNDX(d.p_ndx, d.num_entries, p.next_sq_ctx_p_ndx)
 
   // DMA NVME backend CQ p_ndx to p_ndx_db (doorbell) register
   // TODO: Check of this is by scheduler bit or doorbell write
   DMA_PHV2MEM_SETUP(next_sq_ctx_p_ndx, next_sq_ctx_p_ndx, d.p_ndx_db, 
                     dma_cmd2_phv_start, dma_cmd2_phv_end, dma_cmd2_cmdtype, 
                     dma_cmd2_addr, dma_cmd2_host_addr)

   // If not last descriptor in sequence set the scheduler bit once again.
   // We need a branch delay slot here to avoid writing DMA command into PHV.
   seq		c1, k.seq_is_last, 1
   bcf		[c1], last_seq
   nop
   

   // Ring the doorbell for scheduling the sequencer once again
   // Write the table entry address instead of p_ndx
   DMA_PHV2MEM_SETUP(seq_tbl_entry_addr, seq_tbl_entry_addr, k.seq_tbl_entry_db,
                     dma_cmd3_phv_start, dma_cmd3_phv_end, dma_cmd3_cmdtype, 
                     dma_cmd3_addr, dma_cmd3_host_addr)
   
   // Setup the start and end DMA pointers and exit
   DMA_PTR_SETUP(dma_cmd0_rsvd, dma_cmd3_cmdeop, intrinsic_dma_cmd_ptr)
   b		exit

last_seq:
   // Setup the start and end DMA pointers and exit
   DMA_PTR_SETUP(dma_cmd0_rsvd, dma_cmd2_cmdeop, intrinsic_dma_cmd_ptr)

exit:
   nop.e
   nop
