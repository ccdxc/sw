#include "rawr-defines.h"

struct phv_                         p;
struct rawr_mpage_free_k            k;
struct rawr_mpage_free_mpage_free_d d;

%%
    .param      RNMPR_SMALL_TABLE_BASE
    
    .align

rawr_s7_mpage_free:

    CAPRI_CLEAR_TABLE2_VALID

    /*
     * mpage free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    bcf         [c1], mpage_free_exit
    RAWR_RAWR_FREE_SEM_PINDEX_FULL_TRAP(c1)

    RAWR_FREE_SEM_PINDEX_ELEM_FREE(RNMPR_SMALL_TABLE_BASE,
                                   RNMPR_SMALL_TABLE_ENTRY_SIZE_SHFT,
                                   d.pindex,
                                   k.{to_s7_mpage})
mpage_free_exit:
                                   
    nop.e
    nop
    
