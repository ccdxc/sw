/****************************************************************************
 *  Stage: Load the sequencer entry's addr for the next stage to process
 ****************************************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct seq_desc_entry_addr_k k;
struct seq_desc_entry_addr_load_d d;
struct phv_ p;

%%

seq_desc_entry_addr_load_start:
   // Save the entire d vector to PHV
   phvwr	p.seq_desc_entry_addr, d.desc_entry_addr

   // Save the table address
   phvwr	p.seq_tbl_entry_addr, k.intrinsic_table_addr_raw;

   // Set the table and program address 
   LOAD_TABLE_CBASE_OFFSET(d.desc_entry_addr, 0, SEQ_DESC_ENTRY_SIZE,
                           seq_desc_entry_check)

