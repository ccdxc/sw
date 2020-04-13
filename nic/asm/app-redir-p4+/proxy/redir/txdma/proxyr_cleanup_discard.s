#include "app_redir_common.h"

struct phv_                     p;
struct proxyr_cleanup_discard_k k;

/*
 * Registers usage
 */
#define r_elem_addr                 r1  // element address to free
#define r_free_inf_addr             r2
#define r_qstate_addr               r3

%%
    .param      proxyr_s6_desc_free
    .param      proxyr_s6_ppage_free
    .param      proxyr_s6_mpage_free
    .param      proxyr_err_stats_inc
    
    .align

/*
 * Common code to lanuch cleanup code to free desc/ppage/mpage
 * due to semaphore pindex full on one or more such resources.
 */
proxyr_s5_cleanup_discard:

    CAPRI_CLEAR_TABLE1_VALID
        
    /*
     * Launch desc semaphore pindex free
     */
    add         r_elem_addr, r0, k.{to_s5_desc_sbit0_ebit31...\
                                    to_s5_desc_sbit32_ebit33}
    /*
     * c1 will be cumulative flag indicating at lease one 
     * element is non-NULL (flag value would be FALSE)
     */
    seq         c1,  r_elem_addr, r0
    beq         r_elem_addr, r0, _ppage_cleanup_launch
    phvwr       p.to_s6_desc, r_elem_addr   // delay slot
    addi        r_free_inf_addr, r0, ASIC_SEM_RNMDR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyr_s6_desc_free,
                          r_free_inf_addr,
                          TABLE_SIZE_64_BITS)
_ppage_cleanup_launch:

    /*
     * Launch ppage semaphore pindex free
     */
    add         r_elem_addr, r0, k.{to_s5_ppage_sbit0_ebit5...\
                                    to_s5_ppage_sbit30_ebit33}
    seq.c1      c1,  r_elem_addr, r0    // accumulate flag
    beq         r_elem_addr, r0, _mpage_cleanup_launch
    phvwr       p.to_s6_ppage, r_elem_addr  // delay slot
    addi        r_free_inf_addr, r0, ASIC_SEM_RNMPR_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          proxyr_s6_ppage_free,
                          r_free_inf_addr,
                          TABLE_SIZE_64_BITS)
_mpage_cleanup_launch:

    /*
     * Launch mpage semaphore pindex free
     */
    add         r_elem_addr, r0, k.{to_s5_mpage_sbit0_ebit3...\
                                    to_s5_mpage_sbit28_ebit33}
    seq.c1      c1,  r_elem_addr, r0    // accumulate flag
    beq         r_elem_addr, r0, _packet_discard
    phvwr       p.to_s6_mpage, r_elem_addr  // delay slot
    addi        r_free_inf_addr, r0, ASIC_SEM_RNMPR_SMALL_FREE_INF_ADDR
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS,
                          proxyr_s6_mpage_free,
                          r_free_inf_addr,
                          TABLE_SIZE_64_BITS)

_packet_discard:

    /*
     * If nothing needed to be freed above, go collect current stats
     */
    nop.!c1.e
    
    PROXYRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                                 k.{common_phv_qstate_addr_sbit0_ebit5... \
                                    common_phv_qstate_addr_sbit30_ebit33},
                                 p.t3_s2s_inc_stat_check_all)
    nop.e
    nop

