/*****************************************************************************
 *  send_read_data: Send read data for cases which don't involve large I/O
 *                  xfers
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s5_tbl0_k k;
struct s5_tbl0_handle_no_prp_list_d d;
struct phv_ p;

%%
   .param storage_nvme_lookup_sq_start

storage_nvme_send_read_data_start:

   // Save the IOB address into GPR r6 to generated DMA addresses based on offsets
   add		r6, r0, NVME_KIVEC_T0_S2S_IOB_ADDR

   // Is write command (or) command not completed ?
   // => don't download data
   seq		c1, NVME_KIVEC_T0_S2S_IS_READ, 0
   sne		c2, NVME_KIVEC_GLOBAL_OPER_STATUS, IO_CTX_OPER_STATUS_COMPLETED
   bcf		[c1 | c2], tbl_load

   // Compute base address into r7, initialize xfer_len into r5
   add		r7, r6, IO_BUF_DATA_OFFSET				// delay slot
   add		r5, r0, r0

   // Setup DMA of PRP0 entry (macro takes care of validity)
   NVME_DATA_XFER_TO_HOST(NVME_KIVEC_PRP_BASE_PRP0, r7, 
                          NVME_KIVEC_GLOBAL_NVME_DATA_LEN,
                          r5, dma_m2m_1, dma_m2m_2, tbl_load)

   // Setup DMA of PRP1 entry (macro takes care of validity)
   NVME_DATA_XFER_TO_HOST(NVME_KIVEC_PRP_BASE_PRP1, r7, 
                          NVME_KIVEC_GLOBAL_NVME_DATA_LEN,
                          r5, dma_m2m_3, dma_m2m_4, tbl_load)


tbl_load:
   // Load the NVME SQ (stored in dst_qaddr) to figure out the CQ to send
   // the NVME status to in the next stage
   LOAD_TABLE_FOR_ADDR34_PC_IMM(NVME_KIVEC_T0_S2S_DST_QADDR,
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_lookup_sq_start)
