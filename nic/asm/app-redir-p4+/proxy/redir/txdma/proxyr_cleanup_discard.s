#include "app_redir_common.h"

struct phv_                     p;
struct proxyr_cleanup_discard_k k;

/*
 * Registers usage
 */
#define r_elem_addr                 r1  // element address to free
#define r_free_inf_addr             r2

%%
    .param      proxyr_s7_desc_free
    .param      proxyr_s7_ppage_free
    .param      proxyr_s7_mpage_free
    
    .align

/*
 * Common code to lanuch cleanup code to free desc/ppage/mpage
 * due to semaphore pindex full on one or more such resources.
 */
proxyr_s6_cleanup_discard:

    CAPRI_CLEAR_TABLE0_VALID
        
    /*
     * Launch desc semaphore pindex free
     */
    add         r_elem_addr, r0, k.{to_s6_desc_sbit0_ebit31...\
                                    to_s6_desc_sbit32_ebit33}
    beq         r_elem_addr, r0, _ppage_cleanup_launch
    phvwr       p.to_s7_desc, r_elem_addr   // delay slot
    addi        r_free_inf_addr, r0, CAPRI_SEM_RNMDR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyr_s7_desc_free,
                          r_free_inf_addr,
                          TABLE_SIZE_64_BITS)
_ppage_cleanup_launch:

    /*
     * Launch ppage semaphore pindex free
     */
    add         r_elem_addr, r0, k.{to_s6_ppage_sbit0_ebit5...\
                                    to_s6_ppage_sbit30_ebit33}
    beq         r_elem_addr, r0, _mpage_cleanup_launch
    phvwr       p.to_s7_ppage, r_elem_addr  // delay slot
    addi        r_free_inf_addr, r0, CAPRI_SEM_RNMPR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          proxyr_s7_ppage_free,
                          r_free_inf_addr,
                          TABLE_SIZE_64_BITS)
_mpage_cleanup_launch:

    /*
     * Launch mpage semaphore pindex free
     */
    add         r_elem_addr, r0, k.{to_s6_mpage_sbit0_ebit3...\
                                    to_s6_mpage_sbit28_ebit33}
    seq         c1, r_elem_addr, r0
    sne         c2, k.common_phv_mpage_sem_pindex_full, r0
    bcf         [c1 | c2], _packet_discard
    phvwr       p.to_s7_mpage, r_elem_addr  // delay slot
    addi        r_free_inf_addr, r0, CAPRI_SEM_RNMPR_SMALL_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS,
                          proxyr_s7_mpage_free,
                          r_free_inf_addr,
                          TABLE_SIZE_64_BITS)

_packet_discard:

    /*
     * TODO: add stats here
     */
    nop.e
    nop

