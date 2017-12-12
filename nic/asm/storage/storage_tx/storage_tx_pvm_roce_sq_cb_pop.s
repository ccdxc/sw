/*****************************************************************************
 *  pvm_roce_sq_cb_pop : Check the queue state and see if there's anything to
 *                       be reclaimed by comparing cindex against the
 *                       (roce_msn % num_entries). If so increment the working
 *                       index and load the queue entry.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl_k k;
// Use q_state_push d-vector as the stage 0 d-vector has space for action-pc
struct s5_tbl_pvm_roce_sq_cb_push_d d;
struct phv_ p;

%%

storage_tx_pvm_roce_sq_cb_pop_start:
   // If queue is empty, exit
   add		r1, r0, d.num_entries
   addi		r2, r0, 1
   sllv		r2, r2, r1
   subi		r2, r2, 1
   and		r2, d.roce_msn, r2
   seq		c1, d.c_ndx, r2
   bcf		[c1], clear_doorbell
   nop
    

   // Pop the entry from the queue. Note: The working consumer index is updated
   // in the pop operation to ensure that 2 consumers don't pop the same entry.
   // The update of the consumer index happens via DMA write to c_ndx only after 
   // the popped entry has been fully consumed in subsequent stages. Also, the
   // w_ndx to be used is saved in GPR r6 for use later as the tblmincr alters 
   // the d-vector.
   add		r6, r0, d.w_ndx
   QUEUE_POP(d.w_ndx, d.num_entries)

   // Store fields needed in the K+I vector into the PHV
   phvwr	p.storage_kivec0_w_ndx, d.w_ndx
   phvwr	p.storage_kivec0_dst_lif, d.rrq_lif
   phvwr	p.storage_kivec0_dst_qtype, d.rrq_qtype
   phvwr	p.storage_kivec0_dst_qid, d.rrq_qid
   phvwr	p.storage_kivec0_dst_qaddr, d.rrq_qaddr
   phvwr	p.storage_kivec1_src_qaddr, STAGE0_KIVEC_QADDR
   phvwr	p.storage_kivec1_src_lif, STAGE0_KIVEC_LIF
   phvwr	p.storage_kivec1_src_qtype, STAGE0_KIVEC_QTYPE
   phvwr	p.storage_kivec1_src_qid, STAGE0_KIVEC_QID
   
   // Set the table and program address for the next stage to process
   // the popped entry (based on the working consumer index in GPR r6).
   LOAD_TABLE_FOR_INDEX(d.base_addr, r6, d.entry_size, d.entry_size,
                        d.next_pc)
clear_doorbell:
   // Update the queue doorbell to clear the scheduler bit
   QUEUE_POP_DOORBELL_CLEAR

   // Setup the start and end DMA pointers to the doorbell pop
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_0_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES
