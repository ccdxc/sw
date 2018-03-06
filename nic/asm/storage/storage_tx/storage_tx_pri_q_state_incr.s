/*****************************************************************************
 *  pri_q_state_incr: Increment the running counters in the priority queue.
 *                    Load the table to save the R2N WQE in the next stage.
 *****************************************************************************/


#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl_k k;
struct s3_tbl_pri_q_state_incr_d d;
struct phv_ p;

%%

   .param storage_tx_nvme_be_wqe_save_start

storage_tx_pri_q_state_incr_start:

   // Increment counters of high priority queue (if priority matches)
   PRI_QUEUE_INCR(STORAGE_KIVEC0_IO_PRIORITY, NVME_BE_PRIORITY_HI,
                  d.hi_running, d.num_running, check_med)
   b	tbl_load

   // Increment counters of medium priority queue (if priority matches)
check_med:
   PRI_QUEUE_INCR(STORAGE_KIVEC0_IO_PRIORITY, NVME_BE_PRIORITY_MED,
                  d.med_running, d.num_running, check_lo)
   b	tbl_load

   // Increment counters of low priority queue (if priority matches)
check_lo:
   PRI_QUEUE_INCR(STORAGE_KIVEC0_IO_PRIORITY, NVME_BE_PRIORITY_LO,
                  d.lo_running, d.num_running, tbl_load)

tbl_load:
   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR34_PC_IMM(STORAGE_KIVEC0_SSD_BM_ADDR, 
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_tx_nvme_be_wqe_save_start)
