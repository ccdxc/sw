/*****************************************************************************
 *  seq_comp_aol_src_pad
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s4_tbl1_k k;
struct s4_tbl1_seq_comp_aol_src_pad_d d;
struct phv_ p;

/*
 * Registers usage:
 */

%%

storage_seq_comp_aol_src_pad:

    // Set up tblwr for the following:
    // last_src_aol_p->l0 = last_blk_len
    // last_src_aol_p->a1 = pad_buf_addr
    // last_src_aol_p->l1 = pad_len
    // last_src_aol_p->next_addr = 0
    //
    // Note: unlike memwr, tblwr is guaranteed to be written to memory
    // by the time the stage terminates.
    seq         c1, SEQ_KIVEC8_PAD_LEN, r0
if0:    
    bcf         [c1], endif0
    tblwr       d.L0, SEQ_KIVEC8_LAST_BLK_LEN   // delay slot
        
    tblwr       d.A1, SEQ_KIVEC5_PAD_BUF_ADDR
    tblwr       d.L1, SEQ_KIVEC8_PAD_LEN
    
endif0:

    tblwr.e     d.next_addr, r0
    CLEAR_TABLE1 // delay slot
