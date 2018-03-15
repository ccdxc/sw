/*****************************************************************************
 *  check_sq_state : Check the NVME queue state and see if there's
 *                   anything to be popped. If yes, allocate the iob
 *                   resource in the next stage. If not, set a flag to exit 
 *                   the pipeline in the sq pop stage after clearing doorbell
 *                   (as needed).
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl0_k k;
// Use pop_sq d-vector as the stage 0 d-vector has space for action-pc
struct s2_tbl0_pop_sq_d d;
struct phv_ p;

%%
   .param storage_nvme_allocate_iob_start

storage_nvme_check_sq_state_start:
   // Is queue empty
   seq		c1, d.p_ndx, d.w_ndx
   phvwr.c1	p.nvme_kivec_global_oper_status, IO_CTX_OPER_STATUS_NON_STARTER

   // Store the source queue information in K+I vector
   phvwrpair	p.nvme_kivec_global_src_lif, STAGE0_KIVEC_LIF, \
        	p.nvme_kivec_global_src_qtype, STAGE0_KIVEC_QTYPE
   phvwrpair	p.nvme_kivec_global_src_qid, STAGE0_KIVEC_QID, \
   	        p.nvme_kivec_global_src_qaddr, STAGE0_KIVEC_QADDR

   // Store the NVME SQ address in the I/O context
   phvwr	p.io_ctx_nvme_sq_qaddr, STAGE0_KIVEC_QADDR

   // Store the I/O mapping table's base address in K+I vector
   phvwr	p.nvme_kivec_t0_s2s_io_map_base_addr, d.io_map_base_addr
   
   // Set the table and program address to allocate the IO buffer in the
   // next stage
   LOAD_TABLE_FOR_ADDR34_PC_IMM(d.iob_ring_base_addr,
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_allocate_iob_start)
