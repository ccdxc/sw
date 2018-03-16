/*****************************************************************************
 *  lookup_sq : Lookup the SQ and derive the CQ information. Also update the
 *              SQ head in the NVME status based on the c_ndx.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s5_tbl0_k k;
struct s5_tbl0_lookup_sq_d d;
struct phv_ p;

%%
   .param storage_nvme_push_cq_start

storage_nvme_lookup_sq_start:

   // Set destination LIF parameters to point to the CQ
   phvwr	p.{nvme_kivec_t0_s2s_dst_lif...nvme_kivec_t0_s2s_dst_qaddr},	\
   		d.{cq_lif...cq_qaddr}

   // Store the c_ndx in the NVME status as SQ head
   phvwr	p.nvme_sta_sq_head, d.c_ndx


   // Load the NVME CQ to push the NVME status in the next stage
   LOAD_TABLE_FOR_ADDR34_PC_IMM(d.cq_qaddr,
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_push_cq_start)
