/*****************************************************************************
 *  roce_sq_xlate: Read the ROCE SQ xlate entry and figure out the correct
 *                 PVM SQ for this ROCE CQ/SQ queue pair.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s2_tbl_k k;
struct s2_tbl_roce_sq_xlate_d d;
struct phv_ p;

%%
   .param storage_tx_roce_r2n_wqe_prep_start
   .param storage_tx_pvm_roce_sq_cb_update_start

storage_tx_roce_sq_xlate_start:

   // Check if this is a new command or xfer done event
   sne		c1, STORAGE_KIVEC1_ROCE_CQ_NEW_CMD, 1
   bcf		[c1], xfer_done
   nop
   
   // 1. New command processing
   // Save the destination queue parameters in the R2N WQE
   // so that the response can be sent over ROCE
   phvwr	p.r2n_wqe_dst_lif, d.dst_lif
   phvwr	p.r2n_wqe_dst_qtype, d.dst_qtype
   phvwr	p.r2n_wqe_dst_qid, d.dst_qid
   phvwr	p.r2n_wqe_dst_qaddr, d.dst_qaddr

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(STORAGE_KIVEC0_DST_QADDR,
                             STORAGE_DEFAULT_TBL_LOAD_SIZE,
                             storage_tx_roce_r2n_wqe_prep_start)

xfer_done:
   // 1. Transfer done event processing
   // Save the queue parameters to load PVM's ROCE SQ CB to reclaim
   // the SQ entries upto the message sequence number
   phvwr	p.storage_kivec0_dst_lif, d.dst_lif
   phvwr	p.storage_kivec0_dst_qtype, d.dst_qtype
   phvwr	p.storage_kivec0_dst_qid, d.dst_qid
   phvwr	p.storage_kivec0_dst_qaddr, d.dst_qaddr

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(d.dst_qaddr, Q_STATE_SIZE,
                             storage_tx_pvm_roce_sq_cb_update_start)
