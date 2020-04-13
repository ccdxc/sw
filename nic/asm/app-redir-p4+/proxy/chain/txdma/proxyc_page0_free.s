#include "app_redir_common.h"

struct phv_                             p;
struct proxyc_page0_free_k              k;
struct proxyc_page0_free_page0_free_d   d;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers
 * so ensure program stage ends after invoking it.
 */
#define r_page_addr                 r1  // page address to free
#define r_page_is_small             r2  // small page indicator
#define r_table_base                r3  // RNMPR_TABLE_BASE or RNMPR_SMALL_TABLE_BASE
#define r_table_idx                 r4  // PI index
#define r_return                    r5  // return address
#define r_scratch                   r6
 
%%
    .param      RNMPR_TABLE_BASE
    .param      RNMPR_SMALL_TABLE_BASE
    .param      proxyc_s6_page1_no_free
    .param      proxyc_s6_page1_free
    
    .align

/*
 * Entered as a stage transition to launch a page 1 free index
 * semaphore acquisition when there was no page 0 to free.
 */    
proxyc_s5_page0_no_free:

    CAPRI_CLEAR_TABLE1_VALID
    
_page1_free_applic_check:

    /*
     * Launch page 1 semaphore free pindex free acquisition
     * if applicable
     */
    seq         c1, r0, k.{t1_s2s_aol_A1_sbit0_ebit3...\
                           t1_s2s_aol_A1_sbit4_ebit51}
    bcf         [!c1], _page1_sem_free_idx_launch
    nop
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, proxyc_s6_page1_no_free)
    nop.e
    nop

_page1_sem_free_idx_launch:
        
    /*
     * cmov can't handle large immediate so use 2 adds
     */
    sne         c1, k.t1_s2s_aol_A1_small, r0
    addi.c1     r_scratch, r0, ASIC_SEM_RNMPR_SMALL_FREE_INF_ADDR
    addi.!c1    r_scratch, r0, ASIC_SEM_RNMPR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ_e(1, TABLE_LOCK_DIS,
                            proxyc_s6_page1_free,
                            r_scratch,
                            TABLE_SIZE_64_BITS)
    nop

    .align
    

/*
 * Entered after having acquired a page free index semaphore,
 * this function invokes a common function to free page 0.
 */    
proxyc_s5_page0_free:

    CAPRI_CLEAR_TABLE1_VALID

    /*
     * page free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    bcf         [c1], proxyc_page_free_sem_pindex_full
    add         r_page_is_small, r0, k.t1_s2s_aol_A0_small  // delay slot
    add         r_page_addr, r0, k.{t1_s2s_aol_A0_sbit0_ebit47...\
                                    t1_s2s_aol_A0_sbit48_ebit51}
    bal         r_return, proxyc_page_free
    add         r_table_idx, r0, d.pindex                   // delay slot
    b           _page1_free_applic_check
    nop

    .align
    
/*
 * A free semaphore index was unexpectedly full
 */                                   
proxyc_page_free_sem_pindex_full:

    /*
     * Would have added stats here.
     * However, this label may have been arrived at from tage 7
     * so there are no more stages left to increment stats.
     */
    APP_REDIR_FREE_SEM_PINDEX_FULL_TRAP()
    nop.e
    nop


/*
 * On input, 
 *     r_page_addr: page address to free
 *     r_page_is_small: small page indicator
 *     r_table_idx: index location in which to write r_page_addr
 *     r_return   : return address
 *
 * Both page addr and index come in as they were read which
 * are in in host order (little endian)
 */
proxyc_page_free:

    bne         r_page_is_small, r0, _small_page_free
    add         r_scratch, r0, r_table_idx.wx   // delay slot
    APP_REDIR_IMM64_LOAD(r_table_base, RNMPR_TABLE_BASE)
    mincr       r_scratch, ASIC_RNMPR_RING_SHIFT, r0
    add         r_table_base, r_table_base, r_scratch, \
                RNMPR_TABLE_ENTRY_SIZE_SHFT
    memwr.d     r_table_base, r_page_addr
    
    /*
     * Update CI
     */
    addi        r_table_base, r0, ASIC_SEM_RNMPR_ALLOC_CI_RAW_ADDR
    mincr       r_scratch, ASIC_RNMPR_RING_SHIFT, 1
    memwr.w     r_table_base, r_scratch.wx
    jr          r_return
    wrfence     // delay slot
    
_small_page_free:

    APP_REDIR_IMM64_LOAD(r_table_base, RNMPR_SMALL_TABLE_BASE)
    mincr       r_scratch, ASIC_RNMPR_SMALL_RING_SHIFT, r0
    add         r_table_base, r_table_base, r_scratch, \
                RNMPR_SMALL_TABLE_ENTRY_SIZE_SHFT
    memwr.d     r_table_base, r_page_addr
    
    /*
     * Update CI
     */
    addi        r_table_base, r0, ASIC_SEM_RNMPR_SMALL_ALLOC_CI_RAW_ADDR
    mincr       r_scratch, ASIC_RNMPR_SMALL_RING_SHIFT, 1
    memwr.w     r_table_base, r_scratch.wx
    jr          r_return
    wrfence     // delay slot
    
