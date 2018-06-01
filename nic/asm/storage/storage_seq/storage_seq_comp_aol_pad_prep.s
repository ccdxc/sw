/*****************************************************************************
 *  seq_comp_aol_pad_prep
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl3_k k;
struct phv_ p;

/*
 * Registers usage:
 * CAUTION: r1 is also implicitly used by LOAD_TABLE1_FOR_ADDR_PC_IMM()
 */
#define r_last_blk_no               r3  // last block number
#define r_last_aol_p                r4  // pointer to last AOL descriptor

%%
   .param storage_seq_comp_aol_src_pad
   .param storage_seq_comp_aol_dst_pad

storage_seq_comp_aol_pad_prep:

    // TxDMA descriptor resources are very scarce so launch table reads
    // to do in-stage AOL update.
    //
    // Note: unlike memwr, tblwr is guaranteed to be written to memory
    // by the time the writing stage terminates.
    
    sub         r_last_blk_no, SEQ_KIVEC3_NUM_BLKS, 1
    add         r_last_aol_p, SEQ_KIVEC6_AOL_SRC_VEC_ADDR, \
                r_last_blk_no, BARCO_AOL_DESC_SIZE_SHIFT
    LOAD_TABLE1_FOR_ADDR_PC_IMM(r_last_aol_p, 
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_seq_comp_aol_src_pad)
                
    add         r_last_aol_p, SEQ_KIVEC6_AOL_DST_VEC_ADDR, \
                r_last_blk_no, BARCO_AOL_DESC_SIZE_SHIFT
    LOAD_TABLE2_FOR_ADDR_PC_IMM(r_last_aol_p, 
                                STORAGE_DEFAULT_TBL_LOAD_SIZE,
                                storage_seq_comp_aol_dst_pad)
                                
    CLEAR_TABLE3_e


