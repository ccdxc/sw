#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_ppage_free_k              k;
struct proxyr_ppage_free_ppage_free_d   d;

%%
    .param      RNMPR_TABLE_BASE
    
    .align

proxyr_s7_ppage_free:

    CAPRI_CLEAR_TABLE1_VALID

    /*
     * ppage free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    bcf         [c1], ppage_free_exit
    nop

    APP_REDIR_FREE_SEM_PINDEX_ELEM_FREE(RNMPR_TABLE_BASE,
                                        RNMPR_TABLE_ENTRY_SIZE_SHFT,
                                        d.pindex,
                                        k.{to_s7_ppage})
ppage_free_exit:
                                   
    APP_REDIR_FREE_SEM_PINDEX_FULL_TRAP()
    nop.e
    nop
    
