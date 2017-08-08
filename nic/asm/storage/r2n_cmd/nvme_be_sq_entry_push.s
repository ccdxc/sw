/*****************************************************************************
 *  Stage: Push NVME command from ROCE to the selected priority queue in the
 *         NVME backend's SQ
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_be_push_sq_ctx_k k;
struct nvme_be_push_sq_ctx_sq_push_d d;
struct phv_ p;

%%
   .param	pvm_errq_entry_push_start

nvme_be_sq_entry_push_start:

   // Calculate the source address into r7 as it is to be used as input
   // to PRI_QUEUE_PUSH macro
   add		r7, r0, k.nvme_tgt_kivec1_rbuf_hdr_addr
   addi		r7, r7, R2N_RX_BUF_ENTRY_BE_CMD_OFFSET

   // High priority queue push
   PRI_QUEUE_PUSH(k.nvme_tgt_kivec0_io_priority, NVME_BE_PRIORITY_HI, 
                  d.c_ndx_hi, d.p_ndx_hi, d.p_ndx_hi_db, nvme_tgt_kivec0_p_ndx,
                  d.num_entries, r7, d.base_addr, NVME_BE_SQ_ENTRY_SIZE, 
                  check_med, send_pvm_err)
   b		exit

check_med:
   // Medium priority queue push
   PRI_QUEUE_PUSH(k.nvme_tgt_kivec0_io_priority, NVME_BE_PRIORITY_MED, 
                  d.c_ndx_med, d.p_ndx_med, d.p_ndx_med_db, nvme_tgt_kivec0_p_ndx,
                  d.num_entries, r7, d.base_addr, NVME_BE_SQ_ENTRY_SIZE, 
                  check_lo, send_pvm_err)
   b		exit
check_lo:
   // Low priority queue push
   PRI_QUEUE_PUSH(k.nvme_tgt_kivec0_io_priority, NVME_BE_PRIORITY_LO, 
                  d.c_ndx_lo, d.p_ndx_lo, d.p_ndx_lo_db, nvme_tgt_kivec0_p_ndx,
                  d.num_entries, r7, d.base_addr, NVME_BE_SQ_ENTRY_SIZE, 
                  exit, send_pvm_err)
   b		exit

send_pvm_err:
   LOAD_TABLE_FBASE_IDX(PVM_ERRQ_CTX_TABLE_BASE, PVM_ERRQ_DEFAULT_IDX, 
                        Q_CTX_SIZE, Q_CTX_SIZE, pvm_errq_entry_push_start)

exit:
   nop.e
   nop
