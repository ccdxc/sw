/*****************************************************************************
 *  allocate_iob : Allocate IO buffer from free list if the check SQ stage
 *                 has marked there is something to dequeue. This stage 
 *                 consumes an item off the iob_ring and stores the pointer
 *                 in the K+I vector part of the PHV. If the ring is empty,
 *                 error flag is set in the  K+I vector part of the PHV.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl0_k k;
struct s1_tbl0_allocate_iob_d d;
struct phv_ p;

%%
   .param storage_nvme_pop_sq_start
   .param storage_nvme_save_iob_addr_start

storage_nvme_allocate_iob_start:
   // Is ring empty
   seq		c1, d.p_ndx, d.c_ndx
   bcf		[c1], tbl_load
   // delay slot
   phvwr.c1	p.nvme_kivec_global_oper_status, IO_CTX_OPER_STATUS_NON_STARTER

   // Get the address of IOB pointed to by the c_ndx in the ring
   // Macro stores return value in GPR: r7
   TABLE_ADDR_FOR_INDEX(d.base_addr, d.c_ndx, d.entry_size)

   // Consume the IOB free list entry at the c_ndx in the ring
   QUEUE_POP(d.c_ndx, d.num_entries)

   // Load the table for the IOB free list entry to derive the IOB 
   LOAD_TABLE1_FOR_ADDR34_PC_IMM(r7, d.entry_size[2:0],
                                 storage_nvme_save_iob_addr_start)

tbl_load:
   // Set the table and program address to pop the sq entry in the next stage
   LOAD_TABLE_FOR_ADDR34_PC_IMM(NVME_KIVEC_GLOBAL_SRC_QADDR,
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_nvme_pop_sq_start)
