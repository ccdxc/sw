/****************************************************************************
 *  Stage: Process the sequencer entry and determine the next submission
 *         queue to push to.
 ****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct seq_desc_entry_k k;
struct seq_desc_entry_check_d d;
struct phv_ p;

%%

seq_desc_entry_check_start:
   // Save the entire d vector to PHV
   phvwr	p.{seq_is_valid...seq_next_sq_ctx_addr},	\
		d.{is_valid...next_sq_ctx_addr}

   // If sequencer entry not valid, exit
   seq		c1, d.is_valid, 1
   bcf		![c1], exit

   // If not last sequencer entry, set the address to process the next
   // descriptor entry
   seq		c2, d.is_last, 1
   bcf		[c2], tbl_load

   // Write the descriptor entry address for the next iteration
   add		r1, r0, k.seq_tbl_entry_addr
   add		r2, r0, k.seq_desc_entry_addr
   addi		r2, r2, SEQ_DESC_ENTRY_SIZE
   memwr.d	r1, r2

tbl_load:
   // Set the table and program address 
   LOAD_TABLE_CBASE_OFFSET(d.next_sq_ctx_addr, 0, Q_CTX_SIZE,
                           seq_next_sq_entry_push)

exit:
   nop.e
   nop

