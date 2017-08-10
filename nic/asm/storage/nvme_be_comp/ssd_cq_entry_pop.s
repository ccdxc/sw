/*****************************************************************************
 *  Stage: Save the NVME status entry in CQ to PHV. Increment consumer index in
 *         CQ context to pop the entry. Load the address of the command index
 *         that was allocated in the SSD saved command list for the next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct ssd_cq_entry_k k;
struct ssd_cq_entry_qpop_d d;
struct phv_ p;

%%
   .param	ssd_saved_cmd_handle_start

ssd_cq_entry_pop_start:

   // Save the entire d vector to PHV
   phvwr	p.{nvme_be_resp_p_nvme_sta_lo...nvme_be_resp_p_nvme_sta_w7},\
		d.{nvme_sta_lo...nvme_sta_w7}

   // Derive the saved command index from the NVME status
   add		r1, r0, d.nvme_sta_cid
   andi		r1, r1, 0xFF
   phvwr	p.nvme_tgt_kivec0_cmd_index, r1

   // Pop the entry
   QUEUE_POP(k.nvme_tgt_kivec0_c_ndx, k.nvme_tgt_kivec0_num_entries,
             k.nvme_tgt_kivec0_idx, SSD_CQ_CTX_TABLE_BASE, Q_CTX_STATE_OFFSET,
             Q_CTX_C_NDX_OFFSET)

   // Set the table and program address 
   add		r4, r0, k.nvme_tgt_kivec0_idx
   muli		r4, r4, SSD_CMDS_ENTRY_SIZE
   addi		r4, r4, SSD_CMDS_HEADER_SIZE
   addi		r4, r4, SSD_CMDS_TABLE_BASE
   LOAD_TABLE_CBASE_IDX(r4, d.nvme_sta_cid,
                        NVME_BE_SQ_ENTRY_SIZE, R2N_NVME_HDR_SIZE,
                        ssd_saved_cmd_handle_start)

exit:
   nop.e
   nop
