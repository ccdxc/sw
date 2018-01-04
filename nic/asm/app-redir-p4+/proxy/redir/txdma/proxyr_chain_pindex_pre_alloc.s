#include "app_redir_common.h"

struct phv_                     p;
struct proxyr_chain_pindex_k    k;

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers!
 */
#define r_ring_indices_addr         r3
#define r_qstate_addr               r4

%%

    .param      proxyr_s6_chain_xfer      
    .param      proxyr_s6_cleanup_discard      
    .param      proxyr_err_stats_inc

    .align

/*
 * Launch read of chain TxQ's current PI/CI, each index is 16 bits wide.
 * For the current flow, the assumption is we are the only producer
 * for the corresponding TxQ ring.
 */
proxyr_s5_chain_pindex_pre_alloc:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * Proceed only if cleanup had not been signaled and mpage 
     * successfully allocated
     */
    sne         c1, k.common_phv_do_cleanup_discard, r0
    bcf         [c1], _cleanup_discard_launch
    sne         c1, k.common_phv_mpage_sem_pindex_full, r0
    bcf         [c1], _mpage_sem_pindex_full
#ifdef DO_NOT_USE_CPU_SEM
    add         r_ring_indices_addr, r0, k.{to_s5_chain_ring_indices_addr_sbit0_ebit31...\
                                            to_s5_chain_ring_indices_addr_sbit32_ebit33} // delay slot
    beq         r_ring_indices_addr, r0, _null_ring_indices_addr
    nop

    /*
     * Chain to ARQ: access HBM queue index table directly using
     * table lock.
     *
     * Note: table lock is only effective for a given stage so all P4+
     * programs must coordinate so that they lock a given table in
     * the same stage. For the ARM ARQ, that is stage 6.
     */     
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN,
                          proxyr_s6_chain_xfer,
                          r_ring_indices_addr,
                          TABLE_SIZE_512_BITS)
#else
    CPU_ARQ_SEM_IDX_INC_ADDR(TX, 
                             k.common_phv_chain_ring_index_select,
                             r_ring_indices_addr)
    CAPRI_NEXT_TABLE_READ(1, 
                          TABLE_LOCK_DIS,
                          proxyr_s6_chain_xfer,
                          r_ring_indices_addr,
                          TABLE_SIZE_64_BITS)

#endif
    nop.e
    nop


/*
 * Chain ring indices address unexpected null
 */
_null_ring_indices_addr:    

    PROXYRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                                 k.{common_phv_qstate_addr_sbit0_ebit5... \
                                    common_phv_qstate_addr_sbit30_ebit33},
                                 p.t3_s2s_inc_stat_null_ring_indices_addr)
    b           _cleanup_discard_launch
    nop
    
/*
 * Meta page semaphore alloc pindex full
 */
_mpage_sem_pindex_full:    

    PROXYRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                                 k.{common_phv_qstate_addr_sbit0_ebit5... \
                                    common_phv_qstate_addr_sbit30_ebit33},
                                 p.t3_s2s_inc_stat_sem_alloc_full)
    b           _cleanup_discard_launch
    nop

    
/*
 * Initiate cleanup discard
 */
_cleanup_discard_launch:    

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, proxyr_s6_cleanup_discard)
    nop.e
    nop
