#include "app_redir_common.h"

struct phv_                     p;
struct s2_tbl_k                 k;
struct s2_tbl_pkt_prep_d        d;

/*
 * Registers for phvwrp accesses to DMA_MEM2PKT descriptors
 *
 * Note that CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP uses r1/r2 as scratch registers
 * so ensure program stage ends after invoking it.
 */
#define r_num_dma                   r1  // # of DMA descriptors written
#define r_dma_mem2pkt_ptr           r2  // current DMA mem2pkt pointer
#define r_addr                      r3  // DMA page address
#define r_size                      r4  // DMA page size
#define r_skip                      r5  // adjustment to r_addr/r_size
#define r_return                    r6  // subroutine return address
#define r_cpu_header_addr           r7  // page address of cpu_to_p4plus_header_t

/*
 * Register reuse
 */

%%

    .param      rawc_pkt_txdma_post
    .param      rawc_cpu_flags_not_read

    .align
    
/*
 * Evaluate AOLs contained in the incoming packet descriptor and prep
 * DMA commands to inject the packet. During DMA commands setup,
 * strip certain meta headers as necessary.
 *
 * Note1: If a next service chain TxQ had been configured, none of the
 * DMA commands here will actually execute (see rawc_pkt_txdma_post).
 * However, AOLs evaluation is always done in case freeing of the pages
 * becomes necessary.
 *
 * Note2: the final step of TxDMA, if applicable, requires restoring 
 * the src_lif in the cap_phv_intr_global_t for the DMA. This value will
 * be read from a meta header and passed to the next stage.
 */
rawc_pkt_txdma_prep:

    /*
     * Make some assumptions to simplify implementation:
     * Meta headers and packet data are described within the first 3 AOLs,
     * i.e., next_addr and next_pkt are assumed NULL.
     *
     * An AOL can be of zero length, but if non-zero, it is expected
     * to contain at least a full cpu_to_p4plus_header_t
     */
    add         r_cpu_header_addr, r0, r0
    add         r_num_dma, r0, r0
    add         r_dma_mem2pkt_ptr, r0, APP_REDIR_BIT_OFFS_PHV(dma_pkt0_dma_cmd_type)
    addi        r_skip, r0, CPU_TO_P4PLUS_HEADER_SIZE
    add         r_addr, r0, d.{A0}.dx
    add         r_size, r0, d.{L0}.wx
    sne         c1, r_size, r0
    bal.c1      r_return, _dma_mem2pkt_prep
    add         r_addr, r_addr, d.{O0}.wx       // delay slot

    bal         r_return, _next_dma_base_get
    add         r_addr, r0, d.{A1}.dx           // delay slot
    add         r_size, r0, d.{L1}.wx
    sne         c1, r_size, r0
    bal.c1      r_return, _dma_mem2pkt_prep
    add         r_addr, r_addr, d.{O1}.wx       // delay slot

    bal         r_return, _next_dma_base_get
    add         r_addr, r0, d.{A2}.dx           // delay slot
    add         r_size, r0, d.{L2}.wx
    sne         c1, r_size, r0
    bal.c1      r_return, _dma_mem2pkt_prep
    add         r_addr, r_addr, d.{O2}.wx       // delay slot

    /*
     * Ensure at least one DMA descriptor was written
     */
    seq         c1, r_num_dma, r0
    bal.c1      r_return, _aol_error
    subi        r_num_dma, r_num_dma, 1         // delay slot
    bal         r_return, _next_dma_base_get
    CAPRI_CLEAR_TABLE0_VALID                    // delay slot
            
    phvwrpi     r_dma_mem2pkt_ptr, APP_REDIR_BIT_OFFS_DMA_MEM2PKT(pkteop), \
                APP_REDIR_BIT_SIZE_DMA_MEM2PKT(pkteop), TRUE;
    phvwr       p.rawc_kivec3_last_mem2pkt_ptr, r_dma_mem2pkt_ptr

    /*
     * Launch read of cpu_to_p4plus_header_t to obtain src_lif info plus
     * page freeing instruction. The header address was calculated
     * by prep_dma_mem2pkt.
     */
_if2:     
    beq         r_cpu_header_addr, r0, _endif2
    CAPRI_NEXT_TABLE_READ_e(1, TABLE_LOCK_DIS,
                            rawc_pkt_txdma_post,
                            r_cpu_header_addr,
                            TABLE_SIZE_32_BITS)
    nop
_endif2:

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP_e(1, rawc_cpu_flags_not_read)


_next_dma_base_get:

    /*
     * Layout of DMA descriptors in PHV is not straight forward, in that
     * dma0's bit offset is usually less than dma1's offset, except when
     * they span 2 different flits. So we can't rely on add/sub but
     * must manually calculate the new r_dma_mem2pkt_ptr here.
     */
    add         r_dma_mem2pkt_ptr, r0, APP_REDIR_BIT_OFFS_PHV(dma_pkt0_dma_cmd_type)
    seq         c1, r_num_dma, 1
    add.c1      r_dma_mem2pkt_ptr, r0, APP_REDIR_BIT_OFFS_PHV(dma_pkt1_dma_cmd_type)
    seq         c1, r_num_dma, 2
    jr          r_return
    add.c1      r_dma_mem2pkt_ptr, r0, APP_REDIR_BIT_OFFS_PHV(dma_pkt2_dma_cmd_type) // delay slot
    
_dma_mem2pkt_prep:

    /*
     * On entry, r_size contains a non-zero AOL length (guaranteed
     * by the caller).
     *
     * This function returns:
     *    r_skip: reset to zero if cpu_to_p4plus_header_t was skipped
     *
     * r_skip may equal 0 or sizeof(cpu_to_p4plus_header_t);
     * Note: when r_size is > 0, it must also be at least >= r_skip
     */
    beq         r_size, r0, _aol_error  // enforce caller's guarantee
    slt         c2, r_size, r_skip      // delay slot, C2 used again later below
    bcf         [c2], _aol_error
    sne         c4, r_skip, r0          // delay slot

    /*
     * Calculate and return the page address of cpu_to_p4plus_header_t
     * It should be the page address of the first AOL that has
     * non-zero length (when r_skip > 0).
     */
    add.c4      r_cpu_header_addr, r0, r_addr

    /*
     * Will adjust r_addr/r_size by the amount indicated in r_skip
     * and set up the DMA descriptor. Ensure that r_size > r_skip,
     * Otherwise, no need to set up the current descriptor (i.e.,
     * when r_size == r_skip, the entire descriptor will be skipped)
     */    
    sle         c3, r_size, r_skip
    add.!c3     r_addr, r_addr, r_skip
    sub.!c3     r_size, r_size, r_skip
    phvwrpi.!c3 r_dma_mem2pkt_ptr, APP_REDIR_BIT_OFFS_DMA_MEM2PKT(cmdtype),    \
                APP_REDIR_BIT_SIZE_DMA_MEM2PKT(cmdtype), CAPRI_DMA_COMMAND_MEM_TO_PKT;
                
    phvwrp.!c3  r_dma_mem2pkt_ptr, APP_REDIR_BIT_OFFS_DMA_MEM2PKT(addr),       \
                APP_REDIR_BIT_SIZE_DMA_MEM2PKT(addr), r_addr;
                
    phvwrp.!c3  r_dma_mem2pkt_ptr, APP_REDIR_BIT_OFFS_DMA_MEM2PKT(size),       \
                APP_REDIR_BIT_SIZE_DMA_MEM2PKT(size), r_size;
    /*
     * Update r_num_dma and r_skip on return:
     *    Only need to skip sizeof(cpu_to_p4plus_header_t) once so the
     *    next skip would be zero if we had actually done it.
     */
    addi.!c3    r_num_dma, r_num_dma, 1
    jr          r_return
    add.!c2     r_skip, r0, r0          // delay slot


/*
 * Either none of the AOLs contained any data, or cpu_to_p4plus_header_t
 * spanned across multiple AOLs!
 */
_aol_error:

    APP_REDIR_TXDMA_INVALID_AOL_TRAP()
    RAWC_METRICS_SET(aol_error_discards)
    jr          r_return
    phvwri      p.rawc_kivec0_do_cleanup_discard, TRUE   // delay slot

    
