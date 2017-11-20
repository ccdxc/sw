#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_mpage_free_k              k;
struct proxyr_mpage_free_mpage_free_d   d;

%%
    .param      RNMPR_SMALL_TABLE_BASE
    
    .align

proxyr_s7_mpage_free:

    CAPRI_CLEAR_TABLE2_VALID

    /*
     * mpage free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    bcf         [c1], mpage_free_exit
    nop

    APP_REDIR_FREE_SEM_PINDEX_ELEM_FREE(RNMPR_SMALL_TABLE_BASE,
                                        RNMPR_SMALL_TABLE_ENTRY_SIZE_SHFT,
                                        d.pindex,
                                        k.{to_s7_mpage})
mpage_free_exit:
                                   
    APP_REDIR_FREE_SEM_PINDEX_FULL_TRAP()
    nop.e
    nop
    
