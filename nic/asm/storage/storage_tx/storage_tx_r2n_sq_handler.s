/*****************************************************************************
 *  r2n_sq_handler: Read the R2N WQE posted by local PVM to get the pointer to
 *                  the NVME backend command. Call the next stage to read the
 *                  NVME backend command to determine the SSD queue and
 *                  priority ring to post to.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl_k k;
struct s1_tbl_r2n_sq_handler_d d;
struct phv_ p;

%%
   .param storage_tx_nvme_be_wqe_prep_start
   .param storage_tx_roce_rq_push_start

storage_tx_r2n_sq_handler_start:

   // Save the R2N WQE to PHV
   R2N_WQE_BASE_COPY
  
   seq		c1, d.opcode, R2N_OPCODE_PROCESS_WQE
   bcf		[!c1], check_buf_post
   nop

   // Process WQE => Set the table and program address for loading the
   // WQE pointer
   LOAD_TABLE_FOR_ADDR_PC_IMM(d.handle, STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_tx_nvme_be_wqe_prep_start)

check_buf_post:
   seq		c1, d.opcode, R2N_OPCODE_BUF_POST
   bcf		[!c1], exit
   nop

   // Copy the destination information present in the R2N WQE to K+I
   phvwr	p.{storage_kivec0_dst_lif...storage_kivec0_dst_qaddr}, \
                d.{dst_lif...dst_qaddr}

   // Setup the R2N buffer to post using mem2mem DMA in DMA commands 1 & 2
   R2N_BUF_POST_SETUP(d.handle)
   
   // Set the program address and table address based on the destination passed 
   // in the WQE to post the R2N buffer to ROCE RQ
   LOAD_TABLE_FOR_ADDR34_PC_IMM(d.dst_qaddr, Q_STATE_SIZE,
                                storage_tx_roce_rq_push_start)

exit:
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_0_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)
   LOAD_NO_TABLES
