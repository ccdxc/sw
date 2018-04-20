/*****************************************************************************
 *  storage_tx_seq_barco_ring_pndx_read_start: common stage for launching
 *                           table lock read to get the Barco ring pindex.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct phv_ p;

%%
   .param storage_tx_seq_barco_ring_push_start

storage_tx_seq_barco_ring_pndx_read_start:

   LOAD_TABLE_FOR_ADDR34_PC_IMM(STORAGE_KIVEC4_BARCO_PNDX_SHADOW_ADDR,
                                STORAGE_KIVEC4_BARCO_PNDX_SIZE,
                                storage_tx_seq_barco_ring_push_start)

