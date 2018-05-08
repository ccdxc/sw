/*****************************************************************************
 *  storage_seq_barco_ring_pndx_read: common stage for launching
 *                                    table lock read to get the Barco ring pindex.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct phv_ p;

%%
   .param storage_seq_barco_ring_push

storage_seq_barco_ring_pndx_read:

   LOAD_TABLE_FOR_ADDR34_PC_IMM(SEQ_KIVEC4_BARCO_PNDX_SHADOW_ADDR,
                                SEQ_KIVEC4_BARCO_PNDX_SIZE,
                                storage_seq_barco_ring_push)

