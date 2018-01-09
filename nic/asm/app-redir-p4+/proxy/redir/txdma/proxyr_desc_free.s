#include "app_redir_common.h"

struct phv_                         p;
struct proxyr_desc_free_k           k;
struct proxyr_desc_free_desc_free_d d;

/*
 * Registers usage
 */
#define r_table_base                r1  // RNMDR_TABLE_BASE
#define r_table_idx                 r2  // PI index

%%
    .param      RNMDR_TABLE_BASE
    
    .align

/*
 * Entered after having acquired a descriptor free index semaphore,
 * this function invokes a common function to free page 0.
 */    
proxyr_s7_desc_free:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * descriptor free semaphore should never be full
     */
    sne         c1, d.pindex_full, r0
    bcf         [c1], _free_sem_pindex_full
    add         r_table_idx, r0, d.{pindex}.wx    // delay slot

    APP_REDIR_IMM64_LOAD(r_table_base, RNMDR_TABLE_BASE)
    mincr       r_table_idx, CAPRI_RNMDR_RING_SHIFT, r0
    add         r_table_base, r_table_base, r_table_idx, \
                RNMDR_TABLE_ENTRY_SIZE_SHFT
    memwr.d     r_table_base, k.{to_s7_desc_sbit0_ebit31...\
                                 to_s7_desc_sbit32_ebit33}
    
    /*
     * Update CI
     */
    addi        r_table_base, r0, CAPRI_SEM_RNMDR_ALLOC_CI_RAW_ADDR
    mincr       r_table_idx, CAPRI_RNMDR_RING_SHIFT, 1
    memwr.w     r_table_base, r_table_idx.wx
    wrfence.e
    nop
    

/*
 * A free semaphore index was unexpectedly full
 */                                   
_free_sem_pindex_full:
                                   
    /*
     * Would have added stats here.
     * However, this is stage 7 so there are no more stages left to increment stats.
     */
    APP_REDIR_FREE_SEM_PINDEX_FULL_TRAP()
    nop.e
    nop
    
