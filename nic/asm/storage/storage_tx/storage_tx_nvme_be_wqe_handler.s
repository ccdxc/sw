/*****************************************************************************
 *  nvme_be_wqe_handler: Read the saved R2N WQE to determine which priority
 *                       queue's running counters to update
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s2_tbl_k k;
struct s2_tbl_nvme_be_wqe_handler_d d;
struct phv_ p;

%%
   .param storage_tx_pri_q_state_decr_start

storage_tx_nvme_be_wqe_handler_start:

   // Save the R2N WQE to PHV
   R2N_WQE_FULL_COPY

   // Restore the fields in the NVME backend status to saved values
   phvwrpair	p.nvme_be_sta_hdr_r2n_buf_handle, d.r2n_buf_handle, \
                p.nvme_sta_cid, d.nvme_cmd_cid
   phvwr	p.iob_addr_ctx_iob_addr, d.iob_addr


   // Save the SSD handle and priority into the K+I vector
   phvwrpair    p.storage_kivec0_io_priority, d.io_priority, \
        	p.storage_kivec0_ssd_handle, d.ssd_handle

   // Ring the sequencer doorbell if enabled
   seq		c1, d.db_enable, 1
   bcf		![c1], check_remote
   phvwr	p.storage_kivec0_is_read, d.is_read[0]  // delay slot
   
   SEQUENCER_DOORBELL_FORM_UPDATE_RING(dma_p2m_3)

   // If R2N WQE is remote, push write data (if any) and status over ROCE
   // to the initiator on the remote side
check_remote:
   seq		c1, d.is_remote, 1
   bcf		![c1], push_local_status
   nop

   // Save the is_remote flag to PHV
   phvwr	p.storage_kivec0_is_remote, 1
   
   // Save the remote destination from R2N WQE into PHV
   phvwr	p.{storage_kivec0_dst_lif...storage_kivec0_dst_qaddr}, \
                d.{dst_lif...dst_qaddr}

   // Calculate the base address of R2N buffer and store it in GPR r6
   sub		r6, d.handle, R2N_BUF_NVME_BE_CMD_OFFSET

   // Setup the DMA command to copy the NVME backend status entry from PHV 
   // to the R2N buffer
   addi		r7, r6, R2N_BUF_STATUS_BUF_OFFSET
   DMA_PHV2MEM_SETUP_ADDR64(nvme_be_sta_hdr_time_us, iob_addr_ctx_iob_addr, r7, 
                            dma_p2m_1)

   // If read command, send read data back via RDMA write
   seq		c1, d.is_read[0], 1
   bcf		![c1], push_remote_status
   nop

   // Push RDMA write descriptor from R2N buffer to the ROCE SQ
   // Source (DMA command 3) is the write descriptor from R2N buffer. 
   // Destination address (in DMA command 4) to be set by the push operation.
   addi		r7, r6, R2N_BUF_WRITE_REQ_OFFSET
   addi		r5, r0, ROCE_SQ_WQE_SIZE
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_SRC, r7, r5, r0, r0, dma_m2m_3)
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r0, r5, r0, r0, dma_m2m_4)

   
   // Push status descriptor (RDMA send) from R2N buffer to the ROCE SQ
push_remote_status:
   // Source (DMA command 5) is the status  descriptor from R2N buffer. 
   // Destination address (in DMA command 6) to be set by the push operation.
   addi		r7, r6, R2N_BUF_STATUS_REQ_OFFSET
   addi		r5, r0, ROCE_SQ_WQE_SIZE
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_SRC, r7, r5, r0, r0, dma_m2m_5)
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r0, r5, r0, r0, dma_m2m_6)

   // Jump to loading the tables
   b		load_tbl
   nop
  
   // If R2N WQE is local, push status to local R2N CQ
push_local_status:
   // Setup the DMA command to push the NVME backend status entry. For now keep 
   // the destination address to be 0 (in GPR r0). Set this correctly in the
   // next stage.
   DMA_PHV2MEM_SETUP_ADDR64(nvme_be_sta_hdr_time_us, iob_addr_ctx_iob_addr, r0, 
                            dma_p2m_1)

load_tbl:
   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR34_PC_IMM(d.pri_qaddr, Q_STATE_SIZE,
                                storage_tx_pri_q_state_decr_start)
