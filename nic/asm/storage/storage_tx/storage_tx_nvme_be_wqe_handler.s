/*****************************************************************************
 *  nvme_be_wqe_handler: Read the saved R2N WQE to determine which priority
 *                       queue's running counters to update
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s2_tbl_k k;
struct s2_tbl_nvme_be_wqe_handler_d d;
struct phv_ p;

%%
   .param storage_tx_pri_q_state_decr_start

storage_tx_nvme_be_wqe_handler_start:

   // Save the R2N WQE to PHV
   R2N_WQE_FULL_COPY

   // Restore the fields in the NVME backend status to saved values
   phvwr	p.nvme_be_sta_hdr_r2n_buf_handle, d.r2n_buf_handle
   phvwr	p.nvme_sta_cid, d.nvme_cmd_cid

   // Save the SSD handle and priority into the K+I vector
   phvwr	p.storage_kivec0_ssd_handle, d.ssd_handle
   phvwr	p.storage_kivec0_io_priority, d.io_priority

   // Ring the sequencer doorbell as needed
   addi		r1, r0, 1
   seq		c1, d.db_enable, r1
   bcf		![c1], load_tbl
   SEQUENCER_DOORBELL_UPDATE(dma_p2m_3)

load_tbl:
   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(d.pri_qaddr, Q_STATE_SIZE,
                             storage_tx_pri_q_state_decr_start)
