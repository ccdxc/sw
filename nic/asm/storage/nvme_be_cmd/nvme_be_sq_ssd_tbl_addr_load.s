/*****************************************************************************
 *  Stage: Load the SSD info table based on the NVME backend SQ idx for
 *         the next stage to perform WWR
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_be_sq_ssd_tbl_addr_k k;
struct phv_ p;

%%
   .param	nvme_be_sq_pri_wrr_start
nvme_be_sq_ssd_tbl_addr_load_start:
   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(SSD_INFO_TABLE_BASE, k.nvme_tgt_kivec0_idx,
                        SSD_INFO_ENTRY_SIZE, SSD_INFO_ENTRY_SIZE,
                        nvme_be_sq_pri_wrr_start)

