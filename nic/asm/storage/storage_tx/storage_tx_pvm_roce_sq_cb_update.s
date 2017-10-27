/*****************************************************************************
 *  pvm_roce_sq_cb_update: Update the PVM's ROCE SQ with the MSN from this
 *                         completion queue. Then ring its doorbell for it to
 *                         reclaim the entries for further use.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl_k k;
struct s3_tbl_pvm_roce_sq_cb_update_d d;
struct phv_ p;

%%

storage_tx_pvm_roce_sq_cb_update_start:

   // Overwrite the MSN field in PVM's ROCE SQ CB with that obtained from the
   // CQ entry and passed along in the K+I vector
   tblwr	d.roce_msn, STORAGE_KIVEC3_ROCE_MSN
   
   // Ring the doorbell for the recipient of the push (without update of p_ndx).
   QUEUE_PUSH_DOORBELL_RING(dma_p2m_4)


   // Setup the start and end DMA pointers
   DMA_PTR_SETUP(dma_p2m_0_dma_cmd_pad, dma_p2m_4_dma_cmd_eop,
                 p4_txdma_intr_dma_cmd_ptr)

tbl_load:
   LOAD_NO_TABLES
