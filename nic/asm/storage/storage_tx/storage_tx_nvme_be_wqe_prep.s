/*****************************************************************************
 *  nvme_be_wqe_prep: From the NVME backend command, determine which SSD and
 *                    priority queue to send it to.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s2_tbl_k k;
struct s2_tbl_nvme_be_wqe_prep_d d;
struct phv_ p;

%%
   .param storage_tx_pri_q_state_push_start

storage_tx_nvme_be_wqe_prep_start:

   // Save the NVME backend command header into R2N WQE in the PHV
   phvwr	p.{r2n_wqe_src_queue_id...r2n_wqe_is_local},	\
		d.{src_queue_id...is_local}

   // Save the SSD handle and priority into the K+I vector
   phvwr	p.storage_kivec0_ssd_handle, d.ssd_handle
   phvwr	p.storage_kivec0_io_priority, d.io_priority

   // Setup the DMA command to push the NVME backend status entry. For now keep 
   // the destination address to be 0 (in GPR r0). Set this correctly in the
   // next stage.
   DMA_PHV2MEM_SETUP(r2n_wqe_handle, r2n_wqe_pri_qaddr, r0, 
                     dma_p2m_1)

   // Calculate the priority queue state address based on SSD handle
   // Output will be in GPR r7
   TABLE_ADDR_FOR_INDEX(STORAGE_KIVEC0_DST_QADDR, d.ssd_handle,
                        STORAGE_KIVEC2_SSD_Q_SIZE)

   // Save the priority queue state address in the K+I vector into the 
   // R2N WQE in the PHV
   phvwr	p.r2n_wqe_pri_qaddr, r7

   // Use the address calculated above to set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(r7, Q_STATE_SIZE,
                             storage_tx_pri_q_state_push_start)
