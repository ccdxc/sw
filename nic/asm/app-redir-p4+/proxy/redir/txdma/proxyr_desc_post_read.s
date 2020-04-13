#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_desc_k                    k;
struct proxyr_desc_post_read_desc_d     d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers!
 */
#define r_alloc_inf_addr            r3
#define r_len                       r4
#define r_qstate_addr               r5

%%

    .param      proxyr_s3_mpage_sem_pindex_post_update
    .param      proxyr_s3_mpage_sem_pindex_skip
    .param      proxyr_err_stats_inc

    .align
    
/*
 * Evaluate AOLs contained in the incoming packet descriptor and launch
 * memory page pindex fetch (for storing meta headers).
 */
proxyr_s2_desc_post_read:

    //CAPRI_CLEAR_TABLE0_VALID

    /*
     * Make some assumptions to simplify implementation:
     *  - desc scratch: TCP flag
     *  - AOL0: TCP payload
     *  - AOL1: NULL
     *  - AOL2: NULL
     */
    phvwr       p.to_s5_ppage, d.{A0}.dx
    add         r_len, d.L1, d.L2
    bne         r_len, r0, _aol_error

    /*
     * Skip allocating mpage if cleanup had been set
     */
    sne         c1, k.common_phv_do_cleanup_discard, r0 // delay slot
    bcf         [c1], _mpage_sem_pindex_skip
    
    /*
     * Fetch/update memory page pindex for storing meta headers
     */
    addi        r_alloc_inf_addr, r0, ASIC_SEM_RNMPR_SMALL_ALLOC_INF_ADDR // delay slot
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyr_s3_mpage_sem_pindex_post_update,
                          r_alloc_inf_addr,
                          TABLE_SIZE_64_BITS)
    /*
     * Shift A0 down so that A0 can be replaced with
     * meta page later
     */
    phvwr.e     p.{aol_A1, aol_O1, aol_L1}, d.{A0, O0, L0}
    nop

/*
 * Unexpected values in desc
 */
_aol_error:

    APP_REDIR_TXDMA_INVALID_AOL_TRAP()
    phvwri      p.t3_s2s_inc_stat_aol_err, 1
    phvwri      p.common_phv_do_cleanup_discard, TRUE

    /*
     * Fall through!!!
     */

/*
 * Skip meta page allocation due to errors or CB not active
 */
_mpage_sem_pindex_skip:
    
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, proxyr_s3_mpage_sem_pindex_skip)
    nop.e    
    nop    

