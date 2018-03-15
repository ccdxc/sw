/*****************************************************************************
 *  handle_cmd: Save the NVME command in SQ entry to PHV. Process the
 *              command to see if it is a Read/Write I/O which can
 *              be processed inline in the datapath. Setup the PRP
 *              assist descriptors for inline processing. For exception
 *              cases, punt to ARM.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl0_k k;
struct s3_tbl0_handle_cmd_d d;
struct phv_ p;

%%
   .param storage_nvme_process_io_map_start

storage_nvme_handle_cmd_start:

   // Save the NVME command to PHV
   phvwr	p.{nvme_cmd_opc...nvme_cmd_dw15}, d.{opc...dw15}                                           

   // Store relevant fields in PHV for handling Read command. For Write 
   // command, these fields are all 0s as PHV is initialized to 0s.
   seq		c1, d.opc, NVME_READ_CMD_OPCODE
   phvwr.c1	p.nvme_kivec_t0_s2s_is_read, 1
   phvwr.c1	p.io_ctx_is_read, 1
   phvwr.c1	p.nvme_be_cmd_hdr_is_read, 1

   // r1: nvme_data_len which is LB_SIZE(nlb + 1)
   sll		r1, d.nlb, LB_SIZE_SHIFT
   addi  	r1, r1, 1;						                            \

   // Store the NVME data length into PHV and into K+I vector
   phvwr	p.nvme_kivec_global_nvme_data_len, r1
   phvwr	p.io_ctx_nvme_data_len, r1

   // If opcode is not one of Read (or) Write command, punt
   seq		c1, d.opc, NVME_READ_CMD_OPCODE
   seq		c2, d.opc, NVME_WRITE_CMD_OPCODE
   bcf		[!c1 & !c2], punt_to_arm

   // If admin_q (or) psdt set (or) len > MAX_ASSIST_SIZE, punt
   seq		c3, NVME_KIVEC_SQ_INFO_IS_ADMIN_Q, 0	// delay slot
   seq		c4, d.psdt, 0
   sle		c5, r1, MAX_ASSIST_SIZE
   bcf		[!c3 | !c4 | !c5], punt_to_arm
  
   // Write PRP0, PRP1 to K+I vector
   phvwrpair	p.nvme_kivec_prp_base_prp0, d.dptr1,	\
		p.nvme_kivec_prp_base_prp1, d.dptr2
   

   // Check if LB_SIZE(nlb + 1) > (PRP_SIZE(dptr1) + PRP_SIZE(dptr2)) 
   // => there is a PRP list and prp_assist needs to be set
   addi		r2, r0, PRP_SIZE_SUB
   and		r3, d.dptr1, PRP_SIZE_MASK
   sub		r3, r2, r3
   and		r4, d.dptr2, PRP_SIZE_MASK
   sub		r4, r2, r4
   add		r5, r3, r3
   sle		c6, r1, r5
   b		tbl_load
   phvwr.c1	p.nvme_kivec_t0_s2s_prp_assist, 1 // delay slot

   // Punt to arm
punt_to_arm:
   phvwr	p.nvme_kivec_t0_s2s_punt_to_arm, 1
   
tbl_load:
   // Set the table and program address to process the IO map entryin the next stage
   LOAD_TABLE_FOR_INDEX_PARAM(NVME_KIVEC_T0_S2S_IO_MAP_BASE_ADDR,
                              d.nsid,
                              IO_MAP_ENTRY_SIZE_LOG2,
                              STORAGE_DEFAULT_TBL_LOAD_SIZE,
                              storage_nvme_process_io_map_start)
