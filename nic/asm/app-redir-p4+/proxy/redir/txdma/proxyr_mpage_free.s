#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_mpage_free_k              k;
struct proxyr_mpage_free_mpage_free_d   d;

/*
 * Registers usage
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
    
    .align

proxyr_s7_mpage_free:

    CAPRI_CLEAR_TABLE2_VALID

    /*
     * mpage free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    bcf         [c1], proxyr_page_free_sem_pindex_full
    addi        r_page_is_small, r0, TRUE   // delay slot
    add         r_page_addr, r0, k.{to_s7_mpage_sbit0_ebit3...\
                                    to_s7_mpage_sbit28_ebit33}
    bal         r_return, proxyr_page_free
    add         r_table_idx, r0, d.pindex                   // delay slot
    nop.e
    nop
    

/*
 * A free semaphore index was unexpectedly full
 */                                   
proxyr_page_free_sem_pindex_full:

    /*
     * Would have added stats here.
     * However, this is stage 7 so there are no more stages left to increment stats.
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
proxyr_page_free:

    bne         r_page_is_small, r0, _small_page_free
    add         r_scratch, r0, r_table_idx.wx   // delay slot
    APP_REDIR_IMM64_LOAD(r_table_base, RNMPR_TABLE_BASE)
    mincr       r_scratch, CAPRI_RNMPR_RING_SHIFT, r0
    add         r_table_base, r_table_base, r_scratch, \
                RNMPR_TABLE_ENTRY_SIZE_SHFT
    memwr.d     r_table_base, r_page_addr
    
    /*
     * Update CI
     */
    addi        r_table_base, r0, CAPRI_SEM_RNMPR_ALLOC_CI_RAW_ADDR
    mincr       r_scratch, CAPRI_RNMPR_RING_SHIFT, 1
    memwr.w     r_table_base, r_scratch.wx
    jr          r_return
    wrfence     // delay slot
    
_small_page_free:

    APP_REDIR_IMM64_LOAD(r_table_base, RNMPR_SMALL_TABLE_BASE)
    mincr       r_scratch, CAPRI_RNMPR_SMALL_RING_SHIFT, r0
    add         r_table_base, r_table_base, r_scratch, \
                RNMPR_SMALL_TABLE_ENTRY_SIZE_SHFT
    memwr.d     r_table_base, r_page_addr
    
    /*
     * Update CI
     */
    addi        r_table_base, r0, CAPRI_SEM_RNMPR_SMALL_ALLOC_CI_RAW_ADDR
    mincr       r_scratch, CAPRI_RNMPR_SMALL_RING_SHIFT, 1
    memwr.w     r_table_base, r_scratch.wx
    jr          r_return
    wrfence     // delay slot
    
