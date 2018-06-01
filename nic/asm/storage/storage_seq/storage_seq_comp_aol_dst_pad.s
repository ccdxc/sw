/*****************************************************************************
 *  seq_comp_aol_dst_pad
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s4_tbl2_k k;
struct s4_tbl2_seq_comp_aol_dst_pad_d d;
struct phv_ p;

/*
 * Registers usage:
 */

%%

storage_seq_comp_aol_dst_pad:

    // Set up tblwr for the following:
    // last_dst_aol_p->next_addr = 0
    //
    // Note: unlike memwr, tblwr is guaranteed to be written to memory
    // by the time the stage terminates.
    
    tblwr.e     d.next_addr, r0
    CLEAR_TABLE2 // delay slot
