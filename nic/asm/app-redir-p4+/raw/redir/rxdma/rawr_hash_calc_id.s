#include "app_redir_common.h"

struct phv_     p;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers!
 */
#define r_toeplitz_hash_result      r1

%%

    .param      rawr_s4_chain_pindex_pre_alloc
    .param      CAPRI_CPU_HASH_MASK
    .param      CAPRI_CPU_MAX_ARQID
    
    .align
    
rawr_s3_toeplitz_hash_none:

    //CAPRI_CLEAR_TABLE1_VALID
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, rawr_s4_chain_pindex_pre_alloc)
    nop.e
    nop

rawr_s3_toeplitz_hash_calc_id:

    /*
     * Entered from asm/common-p4+/rx_table_cpu_hash.s after a HW hash calculation,
     * use same algorithm as cpu-p4plus program to calculate ARQ QID/CPU ID from the hash. 
     */
    TOEPLITZ_HASH_CALC_ID(r_toeplitz_hash_result,
                          CAPRI_CPU_HASH_MASK,
                          CAPRI_CPU_MAX_ARQID,
                          c1)
    b           rawr_s3_toeplitz_hash_none
    phvwr       p.common_phv_chain_ring_index_select, r_toeplitz_hash_result // delay slot

