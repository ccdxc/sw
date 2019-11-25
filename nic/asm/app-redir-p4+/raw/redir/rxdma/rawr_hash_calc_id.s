#include "app_redir_common.h"

struct phv_     p;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers!
 */
#define r_ring_index_select         r3

%%

    .param      rawr_chain_pindex_pre_alloc
    .param      CAPRI_CPU_HASH_MASK
    .param      CAPRI_CPU_MAX_ARQID
    
    .align
    
rawr_toeplitz_hash_none:

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP_e(0, rawr_chain_pindex_pre_alloc)
    
rawr_toeplitz_hash_calc_id:

    /*
     * Entered from asm/common-p4+/rx_table_cpu_hash.s after a HW hash calculation,
     * use same algorithm as cpu-p4plus program to calculate ARQ QID/CPU ID from the hash. 
     */
    TOEPLITZ_HASH_CALC_ID(r_ring_index_select,
                          CAPRI_CPU_HASH_MASK,
                          CAPRI_CPU_MAX_ARQID,
                          c1)
    b           rawr_toeplitz_hash_none
    phvwr       p.rawr_kivec0_chain_ring_index_select, r_ring_index_select // delay slot
    
