/*****************************************************************************
 *  Stage: Read and process the NVME backend command that was saved before
 *         sending to SSD. Form NVME backend status based on this. Load the
 *         address of the SSD info table to update the running counters in the
 *         next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct ssd_saved_cmd_k k;
struct ssd_saved_cmd_handle_d d;
struct phv_ p;

%%
   .param	ssd_info_update_start

ssd_saved_cmd_handle_start:

   // Form the NVME backend response
   phvwri	p.nvme_be_resp_p_time_us, 0
   phvwri	p.nvme_be_resp_p_be_status, 0
   phvwri	p.nvme_be_resp_p_rsvd, 0
   phvwr	p.nvme_be_resp_p_cmd_handle, d.cmd_handle

   // p.nvme_be_resp_p_is_q0 = (d.src_queue_id == 0 ? 1 : 0)
   and		r4, d.src_queue_id, r0
   xori		r4, r4, 1
   phvwr	p.nvme_be_resp_p_is_q0, r4

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(SSD_INFO_TABLE_BASE, k.nvme_tgt_kivec0_idx,
                        SSD_INFO_ENTRY_SIZE, SSD_INFO_ENTRY_SIZE,
                        ssd_info_update_start)

exit:
   nop.e
   nop
