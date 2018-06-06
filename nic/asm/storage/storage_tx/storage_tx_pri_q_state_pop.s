/*****************************************************************************
 *  pri_q_state_pop : Check the queue state and see if there's anything to be
 *                    popped from any priority rings. If so increment the
 *                    working index for that ring and load the queue entry.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl_k k;
// Use q_state_push d-vector as the stage 0 d-vector has space for action-pc
struct s3_tbl_pri_q_state_push_d d;
struct phv_ p;

%%
   .param storage_tx_nvme_be_sq_handler_start

storage_tx_pri_q_state_pop_start:
   // Store fields needed in the K+I vector into the PHV
   phvwr	p.{storage_kivec0_dst_lif...storage_kivec0_dst_qaddr}, \
                d.{dst_lif...dst_qaddr}
   
   phvwrpair	p.storage_kivec1_src_lif, STAGE0_KIVEC_LIF, \
        	p.storage_kivec1_src_qtype, STAGE0_KIVEC_QTYPE
   phvwrpair	p.storage_kivec1_src_qid, STAGE0_KIVEC_QID, \
   	        p.storage_kivec1_src_qaddr, STAGE0_KIVEC_QADDR
   
   // TODO: derive is_q0 from QID
   phvwrpair	p.storage_kivec0_is_q0, 0, \
   	        p.storage_kivec0_ssd_bm_addr, d.ssd_bm_addr

   // Pop the entry from the priority queue. The working consumer index is 
   // updated in the pop operation to ensure that 2 consumers don't pop the 
   // same entry.  The update of the consumer index happens via DMA write to 
   // c_ndx only after the popped entry has been fully consumed in subsequent 
   // stages. 

   // If high priority queue can be serviced, go with it, else check medium
   // priority queue
   PRI_QUEUE_CAN_POP(d.p_ndx_hi, d.w_ndx_hi, d.hi_running, d.hi_weight, 
                     check_med)
   SERVICE_PRI_QUEUE(d.w_ndx_hi, NVME_BE_PRIORITY_HI, storage_tx_nvme_be_sq_handler_start)

check_med:
   // If medium priority queue can be serviced, go with it, else check low
   // priority queue
   PRI_QUEUE_CAN_POP(d.p_ndx_med, d.w_ndx_med, d.med_running, d.med_weight,
                     check_lo)
   SERVICE_PRI_QUEUE(d.w_ndx_med, NVME_BE_PRIORITY_MED, storage_tx_nvme_be_sq_handler_start)

check_lo:
   // If medium priority queue can be serviced, go with it, else exit
   PRI_QUEUE_CAN_POP(d.p_ndx_lo, d.w_ndx_lo, d.lo_running, d.lo_weight,
                     clear_doorbell)
   SERVICE_PRI_QUEUE(d.w_ndx_lo, NVME_BE_PRIORITY_LO, storage_tx_nvme_be_sq_handler_start)


clear_doorbell:
   // Nothing can be popped => Clear the doorbell
   PRI_QUEUE_POP_DOORBELL_CLEAR_RESET

   // Setup the start and end DMA pointers to the doorbell pop
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_0_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

   // Nothing more to process in subsequent stages
   LOAD_NO_TABLES

