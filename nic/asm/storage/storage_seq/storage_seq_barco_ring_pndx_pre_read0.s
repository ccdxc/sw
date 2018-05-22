/*****************************************************************************
 *  seq_barco_ring_pndx_pre_read0:
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct phv_ p;

%%
   .param storage_seq_barco_ring_pndx_read

storage_seq_barco_ring_pndx_pre_read0:

   // Advance to a common stage for executing table lock read to get the
   // Barco ring pindex.
   LOAD_TABLE_NO_LKUP_PC_IMM_e(0, storage_seq_barco_ring_pndx_read)

