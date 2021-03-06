/*****************************************************************************
 *  q_state_pop : Check the queue state and see if there's anything to be
 *                popped. If so increment the working index and load the
 *                queue entry.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl_k k;
// Use q_state_push d-vector as the stage 0 d-vector has space for action-pc
struct s2_tbl_q_state_push_d d;
struct phv_ p;

%%

storage_tx_q_state_pop_start:
   // If queue is empty, exit
   QUEUE_EMPTY(d.p_ndx, d.w_ndx, clear_doorbell)

   // Pop the entry from the queue. Note: The working consumer index is updated
   // in the pop operation to ensure that 2 consumers don't pop the same entry.
   // The update of the consumer index happens via DMA write to c_ndx only after 
   // the popped entry has been fully consumed in subsequent stages. Also, the
   // w_ndx to be used is saved in GPR r6 for use later as the tblmincr alters 
   // the d-vector.
   add		r6, r0, d.w_ndx
   QUEUE_POP(d.w_ndx, d.num_entries)

   // Store fields needed in the K+I vector into the PHV
   phvwr        p.storage_kivec0_w_ndx, d.w_ndx
   phvwr        p.{storage_kivec0_dst_lif...storage_kivec0_dst_qaddr}, \
                d.{dst_lif...dst_qaddr}
                
   phvwrpair	p.storage_kivec1_src_lif, STAGE0_KIVEC_LIF, \
        	p.storage_kivec1_src_qtype, STAGE0_KIVEC_QTYPE
   phvwrpair	p.storage_kivec1_src_qid, STAGE0_KIVEC_QID, \
   	        p.storage_kivec1_src_qaddr, STAGE0_KIVEC_QADDR
   phvwr	p.storage_kivec6_ssd_ci_addr, d.ssd_ci_addr
   
   phvwrpair	p.storage_kivec2_ssd_q_num, d.ssd_q_num, \
   	        p.storage_kivec2_ssd_q_size, d.ssd_q_size
   // TODO: derive is_q0 from QID
   phvwrpair	p.storage_kivec0_is_q0, 0, \
   	        p.storage_kivec0_ssd_bm_addr, d.ssd_bm_addr
   
   // Initialize the vf_id and sq_id fields in the PHV
   phvwrpair	p.pvm_cmd_trailer_vf_id, d.vf_id, \
   	        p.pvm_cmd_trailer_sq_id, d.sq_id
   
   // Set the table and program address for the next stage to process
   // the popped entry (based on the working consumer index in GPR r6).
   TABLE_ADDR_FOR_INDEX(d.base_addr, r6, d.entry_size)
   LOAD_TABLE0_FOR_ADDR64(r7, STORAGE_DEFAULT_TBL_LOAD_SIZE, d.next_pc)

load_table_default:   
   LOAD_TABLE0_FOR_ADDR64(r7, d.entry_size[2:0], d.next_pc)
   
clear_doorbell:
   QUEUE_EMPTY(d.c_ndx, d.w_ndx, drop_n_exit)

   // Update the queue doorbell to clear the scheduler bit
   QUEUE_POP_DOORBELL_CLEAR

   // Setup the start and end DMA pointers to the doorbell pop
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_0_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   b 		exit
   nop

drop_n_exit:
   phvwr	p.p4_intr_global_drop, 1

exit:
   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES
