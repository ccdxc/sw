/*****************************************************************************
 *  Stage: Pop the sequencer SQ entry. Initiate PDMA to DMA the payload into
 *         HBM. Ring the doorbell for the sequencer which needs to be executed
 *         after this DMA. Ensure sequencer starts only after the completion
 *         of payload DMA with a write fence.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct seq_sq_entry_k k;
struct seq_sq_entry_qpop_d d;
struct phv_ p;

%%

seq_sq_entry_pop_start:
   // Save the entire d vector to PHV
   phvwr	p.{seq_sq_ent_pdma_src_addr...seq_sq_ent_rsvd},	\
		d.{pdma_src_addr...rsvd}                                           
   // Pop the entry
   QUEUE_POP(k.seq_sq_ctx_c_ndx, k.seq_sq_ctx_num_entries,
             k.seq_sq_ctx_idx, SEQ_SQ_CTX_TABLE_BASE,
             Q_CTX_STATE_OFFSET, Q_CTX_C_NDX_OFFSET)

   // Setup the source of the Mem2Mem DMA (the PDMA src address passed in)
   DMA_MEM2MEM_SETUP_RSIZE(d.pdma_src_addr, d.pdma_size,
                           CAPRI_DMA_M2M_TYPE_SRC, dma_cmd0_cmdtype,
                           dma_cmd0_addr, dma_cmd0_data_size,
                           dma_cmd0_mem2mem_type, dma_cmd0_host_addr)

   // Setup the destination of the Mem2Mem DMA (the PDMA dst address passed in)
   DMA_MEM2MEM_SETUP_RSIZE(d.pdma_dst_addr, d.pdma_size,
                           CAPRI_DMA_M2M_TYPE_DST, dma_cmd1_cmdtype,
                           dma_cmd1_addr, dma_cmd1_data_size,
                           dma_cmd1_mem2mem_type, dma_cmd1_host_addr)

   // Write to the doorbell corresponding to the scheduler bit which 
   // will kickstart the sequencer 
   DMA_PHV2MEM_SETUP(seq_sq_ent_tbl_entry_db_val, seq_sq_ent_tbl_entry_db_val,
                     d.tbl_entry_db, dma_cmd2_phv_start, dma_cmd2_phv_end, 
                     dma_cmd2_cmdtype, dma_cmd2_addr, dma_cmd2_host_addr)

   // Setup the write fence to ensure that PHV doorbell DMA happens after
   // the mem2mem PDMA
   DMA_WRITE_FENCE_SETUP(dma_cmd2_wr_fence)

   // Setup the start and end DMA pointers and exit
   DMA_PTR_SETUP(dma_cmd0_rsvd, dma_cmd2_cmdeop, intrinsic_dma_cmd_ptr)

exit:
   nop.e
   nop
