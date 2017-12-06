#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_desc_k                    k;
struct proxyr_desc_desc_post_read_d     d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers!
 */
#define r_alloc_inf_addr            r3

%%

    .param      proxyr_s3_mpage_sem_pindex_post_update
    .param      proxyr_s3_mpage_sem_pindex_skip
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
    phvwr       p.to_s6_ppage, d.{A0}.dx
    sne         c1, d.L0, r0
    sne         c2, d.L1, r0
    sne         c3, d.L2, r0
    sne         c4, d.next_addr, r0
    sne         c5, d.next_pkt, r0
    bcf         [!c1 | c2 | c3 | c4 | c5], _aol_error

    /*
     * Skip allocating mpage if cleanup had been set
     */
    sne         c1, k.common_phv_do_cleanup_discard, r0 // delay slot
    bcf         [c1], _mpage_sem_pindex_skip
    
    /*
     * Shift A0 down to A1 so that A0 can be replaced with
     * meta page later
     */
    phvwr       p.aol_A1, d.A0  // delay slot
    phvwr       p.aol_O1, d.O0
    phvwr       p.aol_L1, d.L0
    
    /*
     * Fetch/update memory page pindex for storing meta headers
     */
    addi        r_alloc_inf_addr, r0, CAPRI_SEM_RNMPR_SMALL_ALLOC_INF_ADDR
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyr_s3_mpage_sem_pindex_post_update,
                          r_alloc_inf_addr,
                          TABLE_SIZE_64_BITS)
    nop.e
    nop


/*
 * Unexpected values in desc
 */
_aol_error:

    /*
     * TODO: add stats here
     */
    APP_REDIR_TXDMA_INVALID_AOL_TRAP()
    phvwri      p.common_phv_do_cleanup_discard, TRUE

    /*
     * Fall through!!!
     */

/*
 * Skip meta page allocation due to errors or CB not active
 */
_mpage_sem_pindex_skip:
    
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, proxyr_s3_mpage_sem_pindex_skip)
    nop.e    
    nop    

