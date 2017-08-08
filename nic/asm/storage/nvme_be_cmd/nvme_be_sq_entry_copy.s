/*****************************************************************************
 *  Stage: Copy the first part of the NVME command into PHV. Load the
 *         address of the second part of the NVME command in the SQ entry for
 *         the next stage.
 *****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct nvme_be_sq_entry1_k k;
struct nvme_be_sq_entry1_copy_d d;
struct phv_ p;

%%
   .param	nvme_be_sq_entry_pop_start
nvme_be_sq_entry_copy_start:
   nop
foo:
   // Save the entire d vector to PHV
   phvwr	p.{nvme_be_cmd_p_src_queue_id...nvme_be_cmd_p_cmd_handle},\
		d.{src_queue_id...cmd_handle}

   // Set the table and program address
   // TODO: FIXME on the bitwise split of table_pc
   LOAD_TABLE_CBASE_OFFSET(k.common_te0_phv_table_pc_sbit4_ebit27, 
                           R2N_NVME_CMD_OFFSET,
                           R2N_NVME_CMD_SIZE, nvme_be_sq_entry_pop_start)
