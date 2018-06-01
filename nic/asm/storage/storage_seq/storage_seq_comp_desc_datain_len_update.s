/*****************************************************************************
 *  seq_comp_desc_datain_len_update
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s4_tbl1_k k;
struct s4_tbl1_seq_comp_desc_datain_len_update_d d;
struct phv_ p;

/*
 * Registers usage:
 */

%%

storage_seq_comp_desc_datain_len_update:

    // Note: unlike memwr, tblwr is guaranteed to be written to memory
    // by the time the stage terminates.

    tblwr.e     d.datain_len, SEQ_KIVEC8XTS_DATAIN_LEN
    CLEAR_TABLE1 // delay slot
