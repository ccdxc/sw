/*****************************************************************************
 *  handle_no_prp_list: Handle the case where there is no PRP list. If this is
 *                      a write command, download the data specified by
 *                      prp0 and prp1 into the I/O buffer.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s5_tbl0_k k;
struct s5_tbl0_handle_no_prp_list_d d;
struct phv_ p;

%%
   .param storage_nvme_process_dst_seq_start

storage_nvme_handle_no_prp_list_start:

   // Save the IOB address into GPR r6 to generated DMA addresses based on offsets
   add		r6, r0, NVME_KIVEC_T0_S2S_IOB_ADDR

   // Table write the PRP entries and check PRP assist
   tblwr	d.entry1, NVME_KIVEC_PRP_BASE_PRP0
   seq		c1, NVME_KIVEC_T0_S2S_PRP_ASSIST, 0
   bcf		[c1], no_prp_list
   tblwr.c1	d.entry1, NVME_KIVEC_PRP_BASE_PRP1			// delay slot

   // Setup the PRP download via PDMA if assist is set
   // TODO: FIXME: Size should be based on data len without blindly using
   //              PRP_ASSIST_MAX_LIST_SIZE
   // Step 1: Calculate dst_addr & size (src_addr is in d-vector)
   // Step 2: Call the macros to setup the Mem2Mem DMAs
   // Note: Don't reuse r5,r7 in between
   addi		r5, r0, PRP_ASSIST_MAX_LIST_SIZE
   add		r7, r6, IO_BUF_PRP_LIST_ENTRY_OFFSET(1)
   DMA_MEM2MEM_SETUP(CAPRI_DMA_M2M_TYPE_SRC, NVME_KIVEC_PRP_BASE_PRP1, 
                     r5, r0, r0, dma_m2m_5)
   DMA_MEM2MEM_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r7, 
                              r5, r0, r0, dma_m2m_6)

   b		tbl_load

no_prp_list:
   // Is read command (or) command to be punted to ARM ?
   // => don't download data
   seq		c2, NVME_KIVEC_T0_S2S_IS_READ, 1			// delay slot
   seq		c3, NVME_KIVEC_T0_S2S_PUNT_TO_ARM, 1
   bcf		[c2 | c3], tbl_load

   // Compute base address into r7, initialize xfer_len into r5
   add		r7, r6, IO_BUF_DATA_OFFSET				// delay slot
   add		r5, r0, r0

   // Setup DMA of PRP0 entry (macro takes care of validity)
   NVME_DATA_XFER_FROM_HOST(NVME_KIVEC_PRP_BASE_PRP0, r7, 
                            NVME_KIVEC_GLOBAL_NVME_DATA_LEN,
                            r5, dma_m2m_7, dma_m2m_8, tbl_load)

   // Setup DMA of PRP1 entry (macro takes care of validity)
   NVME_DATA_XFER_FROM_HOST(NVME_KIVEC_PRP_BASE_PRP1, r7, 
                            NVME_KIVEC_GLOBAL_NVME_DATA_LEN,
                            r5, dma_m2m_9, dma_m2m_10, tbl_load)


tbl_load:
   // Load the table and program for downloading the data (if it is a write 
   // command and has no PRP list) in the next stage
   add		r7, NVME_KIVEC_T0_S2S_IO_MAP_BASE_ADDR,			\
		IO_BUF_SEQ_DB_OFFSET + IO_BUF_SEQ_R2N_DB_OFFSET
   LOAD_TABLE_FOR_ADDR34_PC_IMM(r7,
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_process_dst_seq_start)
