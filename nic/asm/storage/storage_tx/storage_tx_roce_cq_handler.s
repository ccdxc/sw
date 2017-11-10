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

   // Update the queue doorbell to clear the scheduler bit
   QUEUE_POP_DOORBELL_UPDATE

   // Store the qp in the K+I vector
   phvwr	p.storage_kivec0_dst_qid, d.qp

   // If operation type is send, then a new buffer has been posted
   sne		c1, d.op_type, ROCE_OP_TYPE_SEND_RCVD
   bcf		[c1], check_xfer
   nop
   
   // Write the relevant fields into the PHV for forming the R2N WQE
   phvwr	p.r2n_wqe_handle, d.wrid_msn
   phvwri	p.r2n_wqe_is_remote, 1
   phvwri	p.r2n_wqe_opcode, R2N_OPCODE_PROCESS_WQE
   phvwri	p.storage_kivec1_roce_cq_new_cmd, 1
   
   b		tbl_load

check_xfer:
   // If (operation type is write or write_imm) and status is success,
   // then transfer is done
   sne		c1, d.op_type, ROCE_OP_TYPE_WRITE
   sne		c2, d.op_type, ROCE_OP_TYPE_WRITE_IMM
   bcf		[c1 & c2], exit
   sne		c1, d.status, ROCE_CQ_STATUS_SUCCESS
   bcf		[c1], exit
   
   // Store the ROCE message sequence number in the PHV so that it be
   // used to reclaim the SQ entries
   phvwr	p.storage_kivec3_roce_msn, d.wrid_msn[31:0]

tbl_load:
   add		r5, r0, d.qp
   addi		r5, r5, 1
   addi		r6, r0, STORAGE_DEFAULT_TBL_LOAD_SIZE
   LOAD_TABLE_FOR_INDEX_PARAM(STORAGE_KIVEC0_DST_QADDR, r5, r6, r6,
                              storage_tx_roce_sq_xlate_start)

exit:
   LOAD_NO_TABLES

