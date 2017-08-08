/*****************************************************************************
 *  Stage: Read and process the NVME backend command that was saved before
 *         sending to SSD. Form NVME backend status based on this. Load the
 *         address of the SSD info table to update the running counters in the
 *         next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct ssd_saved_cmds1_k k;
struct ssd_saved_cmds1_handle_d d;
struct phv_ p;

%%

ssd_saved_cmd_handle_start:
   // Save the entire d vector to PHV
   phvwr	p.{nvme_be_cmd_src_queue_id...nvme_be_cmd_cmd_handle},	\
		d.{src_queue_id...cmd_handle}

   // Derive the saved command index from the NVME status
   phvwri	p.nvme_be_resp_be_status, 0
   phvwr	p.nvme_be_resp_cmd_handle, d.cmd_handle

   // p.nvme_be_cmd_is_read = (d.src_queue_id == 0 ? 1 : 0)
   and		r4, d.src_queue_id, r0
   xori		r4, r4, 1
   phvwr	p.nvme_be_resp_is_q0, r4

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(SSD_INFO_TABLE_BASE, k.scq_ctx_idx, SSD_INFO_ENTRY_SIZE,
                        SSD_INFO_ENTRY_SIZE, ssd_info_update)

exit:
   nop.e
   nop
