#include "raw_redir_common.h"

struct phv_     p;

%%

    .param      rawr_s5_chain_pindex_pre_alloc
    .align

/*
 * Table 3 advance to next stage which is stage 5 to eventually arrive
 * at a pre-agreed upon stage for handling chain pindex atomic update.
 */    
rawr_s4_chain_qidxr_stage_advance:

    CAPRI_CLEAR_TABLE0_VALID

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, rawr_s5_chain_pindex_pre_alloc)
    nop.e
    nop
