/*****************************************************************************
 *  Stage: Move the next (skip) stage without loading a table. This is done to
 *         line up the stages.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;

%%
   .param	nvme_be_sq_ssd_tbl_addr_load_start
nvme_be_sq_skip_stage_start:
   // Move the next (skip) stage without loading a table. This is done to
   // line up the stages.
   LOAD_NO_TABLE(nvme_be_sq_ssd_tbl_addr_load_start)
