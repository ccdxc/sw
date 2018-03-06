/*****************************************************************************
 *  roce_r2n_wqe_prep: Read the ROCE SQ xlate entry 0 and figure out the
 *                     destination R2N lif/type/queue to send the R2N WQE to.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct s3_tbl_k k;
struct s3_tbl_roce_r2n_wqe_prep_d d;
struct phv_ p;

%%
   .param storage_tx_q_state_push_start

storage_tx_roce_r2n_wqe_prep_start:

   // Save the queue parameters to load PVM's R2N SQ (for sending R2N WQE)
   // into the PHV
   phvwr	p.storage_kivec0_dst_lif, d.dst_lif
   phvwr	p.storage_kivec0_dst_qtype, d.dst_qtype
   phvwr	p.storage_kivec0_dst_qid, d.dst_qid
   phvwr	p.storage_kivec0_dst_qaddr, d.dst_qaddr

   // Setup the DMA command to push the R2N WQE entry. For now keep the 
   // destination address to be 0 (in GPR r0). Set this correctly in the
   // next stage.
   DMA_PHV2MEM_SETUP_ADDR64(r2n_wqe_handle, r2n_wqe_pad, r0, dma_p2m_1)

   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR34_PC_IMM(d.dst_qaddr, Q_STATE_SIZE,
                                storage_tx_q_state_push_start)
