/*****************************************************************************
 *  Stage: Update the SSD info table with the running counters based on
 *         I/O command priority. Load the address of the bitmap of the
 *         saved commands to the SSD for next stage to release.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_be_ssd_info_k k;
struct nvme_be_ssd_info_update_weights_d d;
struct phv_ p;

%%

ssd_info_update_start:
   // Save the entire d vector to PHV
   phvwr	p.{ssd_info_lo_weight...ssd_info_max_cmds},	\
		d.{lo_weight...max_cmds}

   // Store the value to decrement by in a register
   addi		r5, r0, 1

   // Store the priority field in k-vector in a register
   add		r4, r0, k.nvme_be_cmd_io_priority

   // High priority running counter decrement
   SSD_INFO_UPDATE_AND_LOAD(r4, NVME_BE_PRIORITY_HI, d.hi_running, 
                            d.num_running, r5, check_med,
                            ssd_saved_cmd_skip_stage)

check_med:
   // Medium priority running counter decrement
   SSD_INFO_UPDATE_AND_LOAD(r4, NVME_BE_PRIORITY_MED, d.med_running, 
                            d.num_running, r5, check_lo,
                            ssd_saved_cmd_skip_stage)

check_lo:
   // Low priority running counter decrement
   SSD_INFO_UPDATE_AND_LOAD(r4, NVME_BE_PRIORITY_LO, d.lo_running, 
                            d.num_running, r5, exit,
                            ssd_saved_cmd_skip_stage)

exit:
   nop.e
   nop
