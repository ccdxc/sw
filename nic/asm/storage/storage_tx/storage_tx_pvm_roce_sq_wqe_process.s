/*****************************************************************************
 *  pvm_roce_sq_wqe_process: Process the ROCE SQ WQE as pointed to by the
 *                           PVM's ROCE SQ and load the R2N buffer from the
 *                           WRID in the WQE if the operation type was
 *                           RDMA_SEND. Then post the buffer back to ROCE RQ.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s1_tbl_k k;
struct s1_tbl_pvm_roce_sq_wqe_process_d d;
struct phv_ p;

%%
   .param storage_tx_roce_rq_push_start

storage_tx_pvm_roce_sq_wqe_process_start:

   // If operation type is send on the target side, then the buffer has to 
   // posted back
   add          r1, d.op_type, r0
   indexn       r1, r1, [ROCE_OP_TYPE_SEND, ROCE_OP_TYPE_SEND_INV, ROCE_OP_TYPE_SEND_IMM, ROCE_OP_TYPE_SEND_INV_IMM], 0
   blt.s        r1, r0, exit
   nop   
   
   // In DOL environment, all buffer posting is done by infrastructure to serialize
   // operations. Enable the buffer posting in production code when PVM is ready.
   b		exit
   nop
   
   // Setup the R2N buffer to post using mem2mem DMA in DMA commands 1 & 2
   R2N_BUF_POST_SETUP(d.wrid)

   
   // Set the table and the program address for the next stage
   LOAD_TABLE_FOR_ADDR34_PC_IMM(STORAGE_KIVEC0_DST_QADDR, Q_STATE_SIZE,
                                storage_tx_roce_rq_push_start)

exit:
   // No need to setup the start and end DMA pointers, just drop PHV
   phvwr	p.p4_intr_global_drop, 1
   LOAD_NO_TABLES

