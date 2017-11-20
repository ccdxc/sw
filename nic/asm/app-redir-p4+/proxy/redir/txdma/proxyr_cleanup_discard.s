#include "app_redir_common.h"

struct phv_                     p;
struct proxyr_cleanup_discard_k k;

%%
    .param      proxyr_s7_desc_free
    .param      proxyr_s7_ppage_free
    .param      proxyr_s7_mpage_free
    
    .align

/*
 * Common code to lanuch cleanup code to free desc/ppage/mpage
 * due to semaphore pindex full on one or more such resources.
 */
proxyr_s6_cleanup_discard:

    CAPRI_CLEAR_TABLE0_VALID
        
    /*
     * Launch desc semaphore pindex free
     */
    seq         c1, k.common_phv_desc_valid, r0
    bcf         [c1], ppage_cleanup_launch
    phvwr       p.to_s7_desc, k.to_s6_desc  // delay slot
    APP_REDIR_FREE_SEM_PINDEX_FETCH_UPDATE(0, r4,
                                           APP_REDIR_RNMDR_FREE_IDX,
                                           proxyr_s7_desc_free)
ppage_cleanup_launch:

    /*
     * Launch ppage semaphore pindex free
     */
    seq         c1, k.common_phv_ppage_valid, r0
    bcf         [c1], mpage_cleanup_launch
    phvwr       p.to_s7_ppage, k.to_s6_ppage // delay slot
    APP_REDIR_FREE_SEM_PINDEX_FETCH_UPDATE(1, r4,
                                           APP_REDIR_RNMPR_FREE_IDX,
                                           proxyr_s7_ppage_free)
mpage_cleanup_launch:

    /*
     * Launch mpage semaphore pindex free
     */
    sne         c1, k.common_phv_mpage_sem_pindex_full, r0
    seq         c2, k.common_phv_mpage_valid, r0
    bcf         [c1 | c2], packet_discard
    phvwr       p.to_s7_mpage, k.to_s6_mpage // delay slot
    APP_REDIR_FREE_SEM_PINDEX_FETCH_UPDATE(2, r4,
                                           APP_REDIR_RNMPR_SMALL_FREE_IDX,
                                           proxyr_s7_mpage_free)
packet_discard:

    /*
     * TODO: add stats here
     */
    nop.e
    nop

