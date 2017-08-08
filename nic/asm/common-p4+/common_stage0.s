/*****************************************************************************
 *  common_stage0.s: This assembly program provides the common entry point to 
 *                   jump to various P4+ programs. 
 *                   1. Include the start labels used in your stage0 programs 
 *                      in both the parameter and in the jump instrunction. 
 *                   2. Use <this_program_name, stage0_label> when invoking 
 *                      Capri MPU loader API capri_program_label_to_offset() 
 *                      to derive the relative offset to program the queue 
 *                      context structure.
 *                   3. Use <this_program_name> when invoking Capri MPU
 *                      loader API capri_program_to_base_addr() to derive
 *                      the base address for the stage 0 P4+ programs in the
 *                      table config engine.
 *****************************************************************************/


%%
   .param       nvme_be_sq_ctx_check_start

nvme_be_cmd_stage0_start:
   j		nvme_be_sq_ctx_check_start
   nop

   nop.e
   nop

