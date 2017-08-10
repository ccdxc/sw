/*****************************************************************************
 *  Stage: Update the SSD info table with the running counters based on
 *         I/O command priority. Load the address of the bitmap of the
 *         saved commands to the SSD for next stage to release.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct ssd_info_k k;
struct ssd_info_update_weights_d d;
struct phv_ p;

%%
   .param	ssd_saved_cmd_skip_stage_start

ssd_info_update_start:

   // High priority running counter decrement
   SSD_INFO_UPDATE_AND_LOAD(k.nvme_tgt_kivec0_io_priority, 
                            NVME_BE_PRIORITY_HI, d.hi_running, 
                            d.num_running, 1, check_med,
                            ssd_saved_cmd_skip_stage_start)

check_med:
   // Medium priority running counter decrement
   SSD_INFO_UPDATE_AND_LOAD(k.nvme_tgt_kivec0_io_priority,
                            NVME_BE_PRIORITY_MED, d.med_running, 
                            d.num_running, 1, check_lo,
                            ssd_saved_cmd_skip_stage_start)

check_lo:
   // Low priority running counter decrement
   SSD_INFO_UPDATE_AND_LOAD(k.nvme_tgt_kivec0_io_priority,
                            NVME_BE_PRIORITY_LO, d.lo_running, 
                            d.num_running, 1, exit,
                            ssd_saved_cmd_skip_stage_start)

exit:
   nop.e
   nop
