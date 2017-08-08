/*****************************************************************************
 *  Stage: Move the next (skip) stage without loading a table. This is done to
 *         line up the stages.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;

%%

ssd_saved_cmd_skip_stage_start:
   // Move the next (skip) stage without loading a table. This is done to
   // line up the stages.
   LOAD_NO_TABLE(ssd_saved_cmd_tbl_addr_load)
