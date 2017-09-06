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

   // Save the dst_qaddr in the K+I vector into the R2N WQE in the PHV
   phvwr	p.r2n_wqe_pri_qaddr, STORAGE_KIVEC0_DST_QADDR

   // Setup the DMA command to push the NVME backend status entry. For now keep 
   // the destination address to be 0 (in GPR r0). Set this correctly in the
   // next stage.
   DMA_PHV2MEM_SETUP(r2n_wqe_handle, r2n_wqe_pri_qaddr, r0, 
                     dma_p2m_1)

   // Set the table and program address 
   LOAD_TABLE_FOR_INDEX_PARAM(STORAGE_KIVEC0_DST_QADDR, d.ssd_handle,
                              STORAGE_KIVEC2_SSD_Q_SIZE, Q_STATE_SIZE,
                              storage_tx_pri_q_state_push_start)

exit:
   nop.e
   nop
