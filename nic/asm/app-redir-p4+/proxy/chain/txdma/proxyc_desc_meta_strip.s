#include "app_redir_common.h"

struct phv_                             p;
struct proxyc_meta_strip_k              k;
struct proxyc_meta_strip_meta_strip_d   d;

/*
 * Definition of a single AOL for use with tblrdp/tblwrp
 */
struct descr_aol_single_t {
    A : 64;
    O : 32;
    L : 32;
};

/*
 * Registers usage
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers
 * so ensure program stage ends after invoking it.
 */
#define r_total_strip               r1  // total # of meta header bytes to strip
#define r_curr_strip                r2  // current # of meta header bytes to strip
#define aol_p_r                     r3  // AOL pointer
#define r_len                       r4  // work register: current AOL length
#define r_offs                      r5  // work register: current AOL offset
#define r_return                    r6  // subroutine return address
#define r_cpu_header_addr           r7  // page address of cpu_to_p4plus_header_t

/*
 * Registers reuse (post _aol_meta_strip)
 */
#define r_chain_indices_addr        r1
 
%%

    .param      proxyc_s3_desc_enqueue
    .param      proxyc_s3_cpu_flags_post_read
    .param      proxyc_s3_cpu_flags_skip_read
    .align
    
/*
 * Evaluate AOLs contained in the incoming packet descriptor and
 * strip certain meta headers as necessary.
 */
proxyc_s2_desc_meta_strip:

    CAPRI_CLEAR_TABLE0_VALID
    
    /*
     * Make some assumptions to simplify implementation:
     * Meta headers and packet data are described within the first 3 AOLs,
     * i.e., next_addr and next_pkt are assumed NULL.
     *
     * An AOL can be of zero length, but if non-zero, it is expected
     * to contain at least a full cpu_to_p4plus_header_t + p4plus_to_p4_header_t.
     *
     * Note that tblrdp/tblwrp uses positive incremental offset, while AOLs
     * embedded in descriptor are ordered from low to high as L/O/A. Hence,
     * the initial starting offsets below are L0/L1/L2.
     */
    add         r_cpu_header_addr, r0, r0
    addi        r_total_strip, r0, CPU_TO_P4PLUS_HEADER_SIZE + \
                                   P4PLUS_TO_P4_HDR_SZ
    bal         r_return, _aol_meta_strip
    addi        aol_p_r, r0, APP_REDIR_BIT_OFFS_D_VEC(L0)   // delay slot
    
    sne         c1, r_total_strip, r0
    bal.c1      r_return, _aol_meta_strip
    addi.c1     aol_p_r, r0, APP_REDIR_BIT_OFFS_D_VEC(L1)   // delay slot
    
    sne         c1, r_total_strip, r0
    bal.c1      r_return, _aol_meta_strip
    addi.c1     aol_p_r, r0, APP_REDIR_BIT_OFFS_D_VEC(L2)   // delay slot
    
    /*
     * Ensure valid evaluation of r_cpu_header_addr
     */
    beq         r_cpu_header_addr, r0, _aol_error
    
    /*
     * If cleanup had been set, treat like queue full and 
     * handle it in a subsequent stage
     */
    sne         c1, k.common_phv_do_cleanup_discard, r0 // delay slot
    bcf         [c1], _aol_cpu_flags_read
    
    /*
     * Launch read of chain TxQ's current PI/CI, each index is 16 bits wide.
     * For the current flow, the assumption is we are the only producer
     * for the corresponding TxQ ring.
     */
    add         r_chain_indices_addr, r0, \
                k.to_s2_chain_txq_ring_indices_addr     // delay slot
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          proxyc_s3_desc_enqueue,
                          r_chain_indices_addr,
                          TABLE_SIZE_32_BITS)
    /*
     * Fall through!!!
     */

_aol_cpu_flags_read:

    /*
     * Read flags from r_cpu_header_addr to determine how to free each
     * of the pages in desc (just in case). If r_cpu_header_addr is not 
     * available, will assume pages are from RNMPR (as opposed to
     * RNMPR_SMALL).
     */
    beq         r_cpu_header_addr, r0, _aol_cpu_flags_missing
    nop     
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          proxyc_s3_cpu_flags_post_read,
                          r_cpu_header_addr,
                          TABLE_SIZE_32_BITS)
    nop.e
    nop


_aol_cpu_flags_missing:

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(2, proxyc_s3_cpu_flags_skip_read)
    nop.e
    nop


/*
 * On entry, aol_p_r: points to an AOL in d-vec,
 *           r_total_strip: amount of headers to strip,
 *           r_cpu_header_addr: last evaluated page address to
 *                              cpu_to_p4plus_header_t, if any
 *
 * This function will adjust O (offset) and L (length) to strip the amount
 * of info given in r_total_strip, taking into account that 
 * L could be < r_total_strip.
 *
 * On return, r_total_strip: remaining strip amount, if any.
 *            r_cpu_header_addr: points to cpu_to_p4plus_header_t
 *               of the 1st AOL whose length >= sizeof(cpu_to_p4plus_header_t)
 */
_aol_meta_strip:

    tblrdp.wx   r_offs, aol_p_r, APP_REDIR_BIT_OFFS_STRUCT(descr_aol_single_t, O),  \
                                 APP_REDIR_BIT_SIZE_STRUCT(descr_aol_single_t, O)
    tblrdp.wx   r_len,  aol_p_r, APP_REDIR_BIT_OFFS_STRUCT(descr_aol_single_t, L),  \
                                 APP_REDIR_BIT_SIZE_STRUCT(descr_aol_single_t, L)
    /*
     * Calculate r_cpu_header_addr first
     */
    seq         c1, r_cpu_header_addr, r0
    slt         c2, r_len, CPU_TO_P4PLUS_HEADER_SIZE
    setcf       c1, [c1 & !c2] 
    tblrdp.c1.dx r_cpu_header_addr, aol_p_r, APP_REDIR_BIT_OFFS_STRUCT(descr_aol_single_t, A), \
                                             APP_REDIR_BIT_SIZE_STRUCT(descr_aol_single_t, A)
    add.c1      r_cpu_header_addr, r_cpu_header_addr, r_offs

    /*
     * Now adjust O and L to remove strip amount
     */    
    slt         c1, r_len, r_total_strip
    cmov        r_curr_strip, c1, r_len, r_total_strip
    add         r_offs, r_offs, r_curr_strip
    tblwrp.wx   aol_p_r, APP_REDIR_BIT_OFFS_STRUCT(descr_aol_single_t, O),          \
                         APP_REDIR_BIT_SIZE_STRUCT(descr_aol_single_t, O), r_offs
    sub         r_len, r_len, r_curr_strip
    tblwrp.wx   aol_p_r, APP_REDIR_BIT_OFFS_STRUCT(descr_aol_single_t, L),          \
                         APP_REDIR_BIT_SIZE_STRUCT(descr_aol_single_t, L), r_len
    jr          r_return
    sub         r_total_strip, r_total_strip, r_curr_strip  // delay slot


/*
 * Some invalidity detected in one or more AOLs
 */
_aol_error:

    /*
     * TODO: add stats here
     */
    APP_REDIR_TXDMA_INVALID_AOL_TRAP()
    b           _aol_cpu_flags_read
    phvwri      p.common_phv_do_cleanup_discard, TRUE   // delay slot
     
    
