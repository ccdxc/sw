/*****************************************************************************
 *  roce_cq_handler: Handle the ROCE CQ entry by looking at the operation type
 *                   to determine one of the cases and process it accordingly:
 *                   1. New command received in the send buffer => handle  it
 *                   2. RDMA write operation succeded => post the buffer back
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl_k k;
struct s1_tbl_roce_cq_handler_d d;
struct phv_ p;

%%
   .param storage_tx_roce_sq_xlate_start

storage_tx_roce_cq_handler_start:

   // Store the qp in the K+I vector
   phvwr	p.storage_kivec0_dst_qid, d.qp

   // If operation type is send, then a new buffer has been posted
   sne		c1, d.op_type, ROCE_OP_TYPE_SEND_RCVD
   bcf		[c1], check_xfer
   nop
   
   // Write the relevant fields into the PHV for forming the R2N WQE
   phvwr	p.r2n_wqe_handle, d.wrid_msn
   phvwrpair    p.r2n_wqe_opcode, R2N_OPCODE_PROCESS_WQE, \
                p.r2n_wqe_is_remote, 1
   phvwri	p.storage_kivec1_roce_cq_new_cmd, 1
   
   // Store the data buffer pointer in r5 and the R2N buffer pointer in r6
   sub		r6, d.wrid_msn, R2N_BUF_NVME_BE_CMD_OFFSET
   addi		r5, r6, R2N_BUF_DATA_OFFSET

   // Overwrite the PRP1 to point to the data buffer
   // TODO: Based on the data size, need to fix up this pointer to PRP list
   phvwr	p.nvme_cmd_dptr1, r5.dx
   addi		r7, r6, R2N_BUF_NVME_CMD_PRP1_OFFSET
   DMA_PHV2MEM_SETUP(nvme_cmd_dptr1, nvme_cmd_dptr1, r7, dma_p2m_2)

   // Overwrite the RDMA write request to point to the data buffer

   // Step 1: Write the address to PHV
   phvwr	p.r2n_data_buff_addr_addr, r5

   // Step 2: Setup the DMA for setting WRID
   addi		r7, r6, R2N_BUF_WRITE_REQ_WRID_OFFSET
   DMA_PHV2MEM_SETUP_ADDR64(r2n_data_buff_addr_addr, r2n_data_buff_addr_addr, r7, dma_p2m_3)

   // Load the table for the next stage
   b		tbl_load

check_xfer:
   // If (operation type is write or write_imm) and status is success,
   // then transfer is done
   sne		c1, d.status, ROCE_CQ_STATUS_SUCCESS
   bcf		[c1], exit
   nop
   
   // Store the ROCE message sequence number in the PHV so that it be
   // used to reclaim the SQ entries
   phvwr	p.storage_kivec3_roce_msn, d.wrid_msn[31:0]

tbl_load:
   add		r5, d.qp, 1
   addi		r6, r0, STORAGE_DEFAULT_TBL_LOAD_SIZE
   LOAD_TABLE_FOR_INDEX_PARAM(STORAGE_KIVEC0_DST_QADDR, r5, r6, r6,
                              storage_tx_roce_sq_xlate_start)

exit:
   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_0_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   LOAD_NO_TABLES

