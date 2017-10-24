#include "raw_redir_common.h"

struct phv_                     p;
struct rawr_cleanup_discard_k   k;

%%
    .param      rawr_s7_desc_free
    .param      rawr_s7_ppage_free
    .param      rawr_s7_mpage_free
    
    .align

/*
 * Common code to lanuch cleanup code to free desc/ppage/mpage
 * due to semaphore pindex full on one or more such resources.
 */
rawr_s6_cleanup_discard:

    CAPRI_CLEAR_TABLE0_VALID
        
    /*
     * Launch desc semaphore pindex free
     */
    sne         c1, k.common_phv_desc_sem_pindex_full, r0
    bcf         [c1], ppage_cleanup_launch
    phvwr       p.to_s7_desc, k.to_s6_desc  // delay slot
    RAWR_FREE_SEM_PINDEX_FETCH_UPDATE(0, r4,
                                      RAWRDR_FREE_IDX,
                                      rawr_s7_desc_free)
ppage_cleanup_launch:

    /*
     * Launch ppage semaphore pindex free
     */
    sne         c1, k.common_phv_ppage_sem_pindex_full, r0
    seq         c2, k.common_phv_ppage_valid, r0
    bcf         [c1 | c2], mpage_cleanup_launch
    phvwr       p.to_s7_ppage, k.to_s6_ppage // delay slot
    RAWR_FREE_SEM_PINDEX_FETCH_UPDATE(1, r4,
                                      RAWRPR_FREE_IDX,
                                      rawr_s7_ppage_free)
mpage_cleanup_launch:

    /*
     * Launch mpage semaphore pindex free
     */
    sne         c1, k.common_phv_mpage_sem_pindex_full, r0
    seq         c2, k.common_phv_mpage_valid, r0
    bcf         [c1 | c2], packet_discard
    phvwr       p.to_s7_mpage, k.to_s6_mpage // delay slot
    RAWR_FREE_SEM_PINDEX_FETCH_UPDATE(2, r4,
                                      RAWRPR_SMALL_FREE_IDX,
                                      rawr_s7_mpage_free)
packet_discard:
    phvwr.e     p.p4_intr_global_drop, 1
    nop

