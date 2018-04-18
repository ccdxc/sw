/*****************************************************************************
 *  process_dst_seq: Process the destination sequencer doorbell in the I/O
 *                   buffer to setup the sequencer push and doorbell
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s6_tbl0_k k;
struct s6_tbl0_process_dst_seq_d d;
struct phv_ p;

%%
   .param storage_nvme_push_arm_q_start
   .param storage_nvme_push_dst_seq_q_start

storage_nvme_process_dst_seq_start:
   // Store the NVME SQ address in the I/O context
   phvwr	p.{nvme_kivec_t0_s2s_dst_lif...nvme_kivec_t0_s2s_dst_qaddr}, 	\
		d.{lif...qaddr}

   // Setup DMA command to store the I/O context into the I/O buffer
   add		r7, NVME_KIVEC_T0_S2S_IOB_ADDR, IO_BUF_IO_CTX_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(io_ctx_iob_addr, io_ctx_nvme_sq_qaddr, 
                            r7, dma_p2m_11)
   
   
   // Check if the IOB is to be sent to dst sequencer & copied to ARM (or)
   // punted only to ARM
   seq		c1, NVME_KIVEC_T0_S2S_PUNT_TO_ARM, 1
   bcf		[c1], push_to_arm
   nop

   // Set table 0 and program address to push the WQE to the destination
   // sequencer in the next stage. 
   LOAD_TABLE_FOR_ADDR34_PC_IMM(d.qaddr,
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_push_dst_seq_q_start)

push_to_arm:
   // Set table 0 and program address to push the IO buffer to ARM in the
   // next stage. 
   LOAD_TABLE_FOR_ADDR34_PC_IMM(NVME_KIVEC_ARM_DST6_ARM_QADDR,
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_push_arm_q_start)
