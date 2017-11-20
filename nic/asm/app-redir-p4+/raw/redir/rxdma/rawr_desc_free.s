#include "app_redir_common.h"

struct phv_                         p;
struct rawr_desc_free_k             k;
struct rawr_desc_free_desc_free_d   d;

%%
    .param      RNMDR_TABLE_BASE
    
    .align

rawr_s7_desc_free:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * descriptor free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    bcf         [c1], desc_free_exit
    nop

    APP_REDIR_FREE_SEM_PINDEX_ELEM_FREE(RNMDR_TABLE_BASE,
                                        RNMDR_TABLE_ENTRY_SIZE_SHFT,
                                        d.pindex,
                                        k.{to_s7_desc})
desc_free_exit:
                                   
    APP_REDIR_FREE_SEM_PINDEX_FULL_TRAP()
    nop.e
    nop
    
