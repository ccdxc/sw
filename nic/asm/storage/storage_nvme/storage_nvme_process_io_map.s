/*****************************************************************************
 *  process_io_map: Load the I/O mapping table entry and determine the
 *                  following based on that:
 *                    1. PDMA to convert from PRP to flat buffer in HBM
 *                    2. XTS encryption/decryption
 *                    3. Destination queue to send the I/O
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s4_tbl0_k k;
struct s4_tbl0_process_io_map_d d;
struct phv_ p;

%%
   .param storage_nvme_handle_no_prp_list_start

storage_nvme_process_io_map_start:

   // Save the NVME backend cmd information to PHV to write to the I/O buffer
   phvwr	p.{nvme_be_cmd_hdr_src_queue_id...nvme_be_cmd_hdr_io_priority},	\
		d.{src_queue_id...io_priority}                                           

   // Save the IOB address into GPR r6 to generated DMA addresses based on offsets
   add		r6, r0, NVME_KIVEC_T0_S2S_IOB_ADDR

   // Copy the I/O map entry to I/O buffer
   // Step 1: Calculate dst_addr & size (src_addr is in d-vector)
   // Step 2: Call the macros to setup the Mem2Mem DMAs
   // Note: Don't reuse r5,r7 in between
   addi		r5, r0, IO_MAP_ENTRY_SIZE_BYTES
   add		r7, r6, IO_BUF_IO_MAP_DESC_OFFSET
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, d.entry_addr, 
                     r5, r0, r0, dma_m2m_1)
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r7, 
                              r5, r0, r0, dma_m2m_2)

   // If punt to ARM, ignore destination processing and save to IOB
   seq		c1, NVME_KIVEC_T0_S2S_PUNT_TO_ARM, 1
   bcf		[c1], save_to_iob
   
   // Is remote I/O ?
   seq		c2, d.dst_flags, IO_DST_REMOTE				// delay slot
   bcf		![c2], local_processing

   // Is read command ? 
   seq		c3, NVME_KIVEC_T0_S2S_IS_READ, 1			// delay slot


// Remote destination processing
   // Address of ROCE WQE is based on command type
   // Size of ROCE WQE is fixed 
   add.!c3	r7, r6, IO_BUF_WRITE_ROCE_SQ_WQE_OFFSET
   add.c3	r7, r6, IO_BUF_READ_ROCE_SQ_WQE_OFFSET
   phvwrpair	p.seq_r2n_wqe_r2n_wqe_addr, r7,				\
		p.seq_r2n_wqe_r2n_wqe_size, IO_MAP_ROCE_SQ_WQE_SIZE	

   // Set the is_remote flag in R2N WQE
   phvwr	p.seq_r2n_wqe_is_remote, 1

   // Destination for R2N sequencer is ROCE LIF
   phvwr 	p.{seq_r2n_wqe_dst_lif...seq_r2n_wqe_dst_qaddr},	\
		d.{roce_lif...roce_qaddr}

   // Branch to saving IOB and loading the table
   b		save_to_iob

   // Set the is_remote flag in the I/O context
   phvwr	p.io_ctx_is_remote, 1					// delay slot


// Local destination processing
local_processing:
   // Address of R2N WQE is based on command type
   // Size of R2N WQE is fixed 
   add.!c3	r7, r6, IO_BUF_WRITE_R2N_WQE_OFFSET
   add.c3	r7, r6, IO_BUF_READ_R2N_WQE_OFFSET
   phvwrpair	p.seq_r2n_wqe_r2n_wqe_addr, r7,				\
		p.seq_r2n_wqe_r2n_wqe_size, IO_MAP_R2N_WQE_SIZE	

   // Destination for R2N sequencer is local R2N LIF
   phvwr 	p.{seq_r2n_wqe_dst_lif...seq_r2n_wqe_dst_qaddr},	\
		d.{r2n_lif...r2n_qaddr}


// Saving to IOB before proceeding further
save_to_iob:
   // Setup the DMA command to save the NVME command entry and I/O context
   // into the I/O buffer to be sent to target side
   add		r7, r6, IO_BUF_NVME_BE_CMD_OFFSET + NVME_BE_NVME_CMD_OFFSET
   DMA_PHV2MEM_SETUP_ADDR34(nvme_cmd_opc, io_ctx_iob_addr, 
                            r7, dma_p2m_3)
   
   // Setup the DMA command to save the NVME backend command header
   // into the I/O buffer to be sent to target side
   add		r7, r6, IO_BUF_NVME_BE_CMD_OFFSET 
   DMA_PHV2MEM_SETUP_ADDR34(nvme_be_cmd_hdr_src_queue_id, nvme_be_cmd_hdr_is_read, 
                            r7, dma_p2m_4)
   
   // Load the table and program for downloading the data (if it is a write 
   // command and has no PRP list) in the next stage
   add		r7, NVME_KIVEC_T0_S2S_IOB_ADDR, IO_BUF_PRP_LIST_OFFSET
   LOAD_TABLE_FOR_ADDR34_PC_IMM(r7,
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_handle_no_prp_list_start)
