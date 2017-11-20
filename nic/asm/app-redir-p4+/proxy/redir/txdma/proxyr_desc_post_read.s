#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_desc_k                    k;
struct proxyr_desc_desc_post_read_d     d;

%%

    .param      proxyr_s3_mpage_sem_pindex_post_update
    .align
    
/*
 * Evaluate AOLs contained in the incoming packet descriptor and launch
 * memory page pindex fetch (for storing meta headers).
 */
proxyr_s2_desc_post_read:

    CAPRI_CLEAR_TABLE0_VALID
    
    /*
     * Make some assumptions to simplify implementation:
     *  - desc scratch: TCP flag
     *  - AOL0: TCP payload
     *  - AOL1: NULL
     *  - AOL2: NULL
     *  - next_addr and next_pkt are also assumed NULL.
     */
#if APP_REDIR_TXDMA_INVALID_AOL_DEBUG
    sne         c1, d.L0, r0
    sne         c2, d.L1, r0
    sne         c3, d.L2, r0
    sne         c4, d.next_addr, r0
    sne         c5, d.next_pkt, r0
    bcf         [!c1 | c2 | c3 | c4 | c5], aol_error
#endif

    /*
     * Shift A0 down to A1 so that A0 can be replaced with
     * meta page later
     */
    phvwr       p.to_s6_ppage, d.{A0}.dx    // delay slot
    phvwri      p.common_phv_ppage_valid, TRUE
    phvwr       p.aol_A1, d.A0
    phvwr       p.aol_O1, d.O0
    phvwr       p.aol_L1, d.L0
    
    /*
     * Fetch/update memory page pindex for storing meta headers
     */
    APP_REDIR_ALLOC_SEM_PINDEX_FETCH_UPDATE(0, r4,
                                            APP_REDIR_RNMPR_SMALL_ALLOC_IDX,
                                            proxyr_s3_mpage_sem_pindex_post_update)
    nop.e
    nop


aol_error:

    /*
     * Unexpected values in desc
     */
    APP_REDIR_TXDMA_INVALID_AOL_TRAP()
    nop.e    
    nop    

